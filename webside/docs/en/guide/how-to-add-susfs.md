# SUSFS 支持
  
首发支持v4.14.xxx(EMUI 10+)。  
目前兼容ReSukiSU，整合Susfs v2.0.0和backport补丁。如果使用这个补丁，你不需要集成上面任何一个钩子。Susfs使用自己的inline hooks。  
如何集成Susfs?  
进入此仓库的Susfs_Patches文件夹，按照对应内核版本号选择补丁。  
复制补丁到源码根目录，这里按照v4.14的补丁为例子，在终端输入：`git apply --reject 0001-susfs-Add-Susfs-v2.0.0-and-inline-hooks.patch`  
你就能打上补丁，如果打补丁失败，你能看到生成的错误文件，文件按照xxxx.rej结尾。  
你可以跟据.rej文件错误位置，手动打补丁。  
关于v4.9.xxx内核是否会支持？会的，但是bug多。补丁还在制作。制作完成就会上传。