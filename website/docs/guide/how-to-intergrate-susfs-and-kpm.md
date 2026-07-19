# 构建带SuSFS和KPM的内核
  
## SUSFS
支持v4.14.xxx(EMUI 10+)和v4.9.xxx(EMUI9+)。  
目前兼容ReSukiSU，整合Susfs v2.0.0和backport补丁。如果使用这个补丁，你不需要集成上面任何一个钩子。Susfs使用自己的inline hooks。  
如何集成Susfs?  
进入此仓库的Susfs_Patches文件夹，按照对应内核版本号选择补丁。  
复制补丁到源码根目录，这里按照v4.14的补丁为例子，在终端输入：`git apply --reject 0001-susfs-Add-Susfs-v2.0.0-and-inline-hooks.patch`  
你就能打上补丁，如果打补丁失败，你能看到生成的错误文件，文件按照xxxx.rej结尾。  
你可以跟据.rej文件错误位置，手动打补丁。  

## KPM  
KPM目前只能和v4.9.xxx(EMUI9+)兼容，因为后续给内核打补丁的操作会让v4.14.xxx(EMUI 10+)内核的设备死机。  
推测是EMUI10及其以上的系统屏蔽USER LOCK导致的。  
进入此仓库的Susfs_Patches文件夹，按照对应内核版本号选择补丁。如果使用这个补丁，你不需要集成上面任何一个钩子。Susfs使用自己的inline hooks,此补丁包含KPM的向后移植。    
复制补丁到源码根目录，这里按照v4.9的补丁为例子，在终端输入：`git apply --reject 0001-susfs-4.9.xxx-KPM-backport-and-Susfs-v2.0.0-inline-h.patch`  
你就能打上补丁，如果打补丁失败，你能看到生成的错误文件，文件按照xxxx.rej结尾。  
你可以跟据.rej文件错误位置，手动打补丁。  