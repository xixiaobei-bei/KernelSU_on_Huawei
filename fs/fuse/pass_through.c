

#include "fuse_i.h"
#include <linux/file.h>
#include <securec.h>

/*
 * make sure the wrap_file and the wrap_file->file
 * is not null before being invoked
 */
void delete_passthrough_file(struct fuse_conn *fc,
	struct fuse_wrap_file *wrap_file)
{
	spin_lock(&fc->passthrough_files_lock);

	list_del(&wrap_file->entry);

	fput(wrap_file->file);
	wrap_file->file = NULL;
	spin_unlock(&fc->passthrough_files_lock);
}

/** make sure the wrap_file is not null before being invoked */
static void insert_passthrough_file(
	struct fuse_conn *fc, struct fuse_wrap_file *wrap_file, int fd)
{
	spin_lock(&fc->passthrough_files_lock);

	wrap_file->file = fget(fd);
	if (!wrap_file->file) {
		wrap_file->file = NULL;
		spin_unlock(&fc->passthrough_files_lock);
		return;
	}

	list_add_tail(&wrap_file->entry, &fc->passthrough_files);
	spin_unlock(&fc->passthrough_files_lock);
}

void cleanup_passthrough_files(struct fuse_conn *fc)
{
	struct fuse_wrap_file *wrap_file = NULL;
	struct fuse_wrap_file *n = NULL;
	spin_lock(&fc->passthrough_files_lock);
	list_for_each_entry_safe(wrap_file, n, &fc->passthrough_files, entry) {
		if (wrap_file->file)
			fput(wrap_file->file);

		list_del(&wrap_file->entry);
		wrap_file->file = NULL;
	}
	spin_unlock(&fc->passthrough_files_lock);
}

void fuse_pass_through_open_out_args(struct fuse_conn *fc, struct fuse_req *req)
{
	struct fuse_wrap_open_out *wrap_out = NULL;
	struct fuse_open_out *out = NULL;
	uint64_t fd;

	if (req->out.h.error)
		return;

	if (req->in.h.opcode == FUSE_OPEN)
		out = (struct fuse_open_out *)req->out.args[0].value;
	else if (req->in.h.opcode == FUSE_CREATE)
		out = (struct fuse_open_out *)req->out.args[1].value;

	if (out == NULL || IS_ERR(out))
		return;

	wrap_out = container_of(out, struct fuse_wrap_open_out, outarg);

	fd = (out->fh >> FH_SHIFT_TO_FD) & FH_TO_FD_MASK;
	if (fd != 0 && wrap_out->wrap_file)
		insert_passthrough_file(fc, wrap_out->wrap_file, fd);
}

int fuse_flush_update_offset_size(
	struct fuse_file *ff, struct inode *inode)
{
	int ret = 0;

	if (ff == NULL || IS_ERR(ff) || inode == NULL || IS_ERR(inode)) {
		pr_debug("FUSE %s %d: ff or inode is null or err\n",
			__func__, __LINE__);
		return -EINVAL;
	}

	mutex_lock(&ff->meta_update_mutex);
	if ((ff->meta_update.index) > 0) {
		ff->meta_update.fh = ff->fh;
		ret = fuse_setxattr(inode, "UpdateWriteOffSize",
				(const void *)&(ff->meta_update),
				sizeof(struct fuse_opt_offset_size), 0);
		if (ret)
			pr_debug("FUSE %s %d: cannot set xattr\n",
				__func__, __LINE__);

		memset_s((void *)&ff->meta_update,
			sizeof(struct fuse_opt_offset_size),
			0, sizeof(struct fuse_opt_offset_size));
		ff->meta_update.index = 0;
	}
	mutex_unlock(&ff->meta_update_mutex);
	return ret;
}

static int fuse_rt_update_offset_size(
	struct fuse_file *ff, struct inode *inode,
	uint64_t offset, ssize_t size, int flag)
{
	struct fuse_offset_size_data *data = NULL;
	struct fuse_offset_size_data *prev = NULL;
	int ret = 0;

	if (ff == NULL || IS_ERR(ff) || inode == NULL || IS_ERR(inode)) {
		pr_debug("FUSE %s %d: ff or inode is null or err\n",
			__func__, __LINE__);
		return -EINVAL;
	}

	mutex_lock(&ff->meta_update_mutex);
	if ((ff->meta_update.index) >= OPT_META_UPDATE_LIMITED_NUMBER) {
		ff->meta_update.fh = ff->fh;
		ret = fuse_setxattr(
				inode,
				((flag == WRITE) ? "UpdateWriteOffSize" : "UpdateReadOffSize"),
				(const void *)&(ff->meta_update),
				sizeof(struct fuse_opt_offset_size), 0);
		if (ret)
			pr_debug("FUSE %s %d: cannot set xattr\n",
				__func__, __LINE__);

		memset_s((void *)&ff->meta_update,
			sizeof(struct fuse_opt_offset_size),
			0, sizeof(struct fuse_opt_offset_size));
		ff->meta_update.index = 0;
	}

	data = &ff->meta_update.data[ff->meta_update.index];
	prev = data - 1;
	if (ff->meta_update.index > 0 && prev->offset + prev->size == offset) {
		prev->size += size;
	} else {
		data->offset = offset;
		data->size = size;
		ff->meta_update.index++;
	}
	mutex_unlock(&ff->meta_update_mutex);
	return ret;
}

ssize_t fuse_pass_through_rw_iter(
	struct kiocb *iocb, struct iov_iter *direction, int flag)
{
	struct file *file = iocb->ki_filp;
	struct inode *inode = file->f_mapping->host;
	struct fuse_file *ff = file->private_data;
	struct file *op_file = NULL;
	uint64_t offset = iocb->ki_pos;
	ssize_t bytes = 0;

	if (ff == NULL)
		return -EINVAL;

	spin_lock(&ff->fc->passthrough_files_lock);
	if (ff->wrap_file.file == NULL || ff->wrap_file.file->f_op == NULL) {
		spin_unlock(&ff->fc->passthrough_files_lock);
		return -EINVAL;
	}
	op_file = ff->wrap_file.file;
	iocb->ki_filp = get_file(op_file);
	spin_unlock(&ff->fc->passthrough_files_lock);

	if (flag == READ && op_file->f_op->read_iter) {
		bytes = op_file->f_op->read_iter(iocb, direction);
	}
	else if (flag == WRITE && op_file->f_op->write_iter) {
		inode_lock(inode);
		bytes = op_file->f_op->write_iter(iocb, direction);
		if (bytes > 0) {
			fuse_write_update_size(inode, iocb->ki_pos + bytes);
			fuse_invalidate_attr(inode);
			/* send the offset and size to the userspace */
			fuse_rt_update_offset_size(ff, inode, offset, bytes, flag);
		}
		inode_unlock(inode);
	}
	fput(op_file);
	iocb->ki_filp = file;

	return bytes;
}
