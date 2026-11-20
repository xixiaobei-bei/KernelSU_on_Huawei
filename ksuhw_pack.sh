#!/usr/bin/env bash
set -euo pipefail
export LC_ALL=C          # 保证 ${#str} 按字节计数, cmdline 是 ASCII

# PM_STATE=TRUE -> permissive, 其他(FALSE/空) -> enforcing
if [ "${PM_STATE^^}" = "TRUE" ]; then
    SELINUX_MODE="permissive"
else
    SELINUX_MODE="enforcing"
fi

HEADER="${HEADER:-${PWD}/stock/header.bin}"
TAIL="${TAIL:-${PWD}/stock/tail.bin}"
DTB_BLOB="${DTB_BLOB:-${PWD}/stock/dtb_blob.bin}"
GZ="${GZ:-${PWD}/kernel-source/out/arch/arm64/boot/Image.gz}"
OUT="${OUT:-${PWD}/${ARTIFACT_NAME}.img}"
EXTRA_CMDLINE="${1:-}"

HEADER_SIZE=2048
CMD_OFF=64               # 标准 boot header 的 cmdline 偏移
CMD_LEN=512              # cmdline 长度

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

# ---------- 0. 校验输入 ----------
for f in "$HEADER" "$TAIL" "$DTB_BLOB" "$GZ"; do
    [ -r "$f" ] || { echo "找不到: $f" >&2; exit 1; }
done

[ "$(stat -c%s "$HEADER")" -eq "$HEADER_SIZE" ] \
    || { echo "header.bin 不是 $HEADER_SIZE 字节" >&2; exit 1; }

# Image.gz 必须是 gzip (1f 8b)
[ "$(head -c2 "$GZ" | xxd -p)" = "1f8b" ] \
    || { echo "GZ 不是 gzip 格式: $GZ" >&2; exit 1; }

# DTB blob 必须是 QCDT 或 FDT
case "$(head -c4 "$DTB_BLOB" | xxd -p)" in
    51434454|d00dfeed) ;;   # "QCDT" 或 FDT magic
    *) echo "DTB_BLOB magic 未知: $DTB_BLOB" >&2; exit 1 ;;
esac

# ---------- 1. 复制 header ----------
cp "$HEADER" "$TMP/header"

# ---------- 2. cmdline: 强制 selinux 模式 + 可选追加 ----------
orig=$(dd if="$HEADER" bs=1 skip=$CMD_OFF count=$CMD_LEN 2>/dev/null | tr -d '\0')

if echo "$orig" | grep -q 'androidboot\.selinux='; then
    orig=$(echo "$orig" | sed -E "s#androidboot\.selinux=[^ ]*#androidboot.selinux=${SELINUX_MODE}#")
else
    orig="$orig androidboot.selinux=$SELINUX_MODE"
fi

if [ -n "$EXTRA_CMDLINE" ]; then
    orig="$orig $EXTRA_CMDLINE"
fi

n=${#orig}
if [ "$n" -ge "$CMD_LEN" ]; then
    echo "cmdline 溢出: $n >= $CMD_LEN" >&2
    exit 1
fi

printf '%s' "$orig" > "$TMP/cmd"
head -c $((CMD_LEN - n)) /dev/zero >> "$TMP/cmd"
dd if="$TMP/cmd" of="$TMP/header" bs=1 seek=$CMD_OFF conv=notrunc 2>/dev/null
echo "cmdline: $orig"

# ---------- 3. 计算新 kernel_size 并写回 header offset 8 ----------
gz_size=$(stat -c%s "$GZ")
blob_size=$(stat -c%s "$DTB_BLOB")
kernel_size=$((gz_size + blob_size))
hex=$(printf '%08x' "$kernel_size")
le="${hex:6:2}${hex:4:2}${hex:2:2}${hex:0:2}"         # 转小端字节序
echo -n "$le" | xxd -r -p > "$TMP/ks"
dd if="$TMP/ks" of="$TMP/header" bs=1 seek=8 conv=notrunc 2>/dev/null
echo "Image.gz 大小: $gz_size"
echo "DTB blob 大小: $blob_size"
echo "新 kernel_size: $kernel_size"

# ---------- 4. 组装: header + Image.gz + DTB blob + tail ----------
mkdir -p "$(dirname "$OUT")"
: > "$OUT"
cat "$TMP/header"  >> "$OUT"
cat "$GZ"          >> "$OUT"
cat "$DTB_BLOB"    >> "$OUT"
cat "$TAIL"        >> "$OUT"

echo "输出: $OUT, 大小: $(stat -c%s "$OUT")"
