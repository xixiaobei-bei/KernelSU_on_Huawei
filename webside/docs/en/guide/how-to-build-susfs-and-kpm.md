# Build kernel with SuSFS and KPM

## SUSFS
Supports v4.14.xxx (EMUI 10+) and v4.9.xxx (EMUI 9+).  

Currently compatible with ReSukiSU, integrating Susfs v2.0.0 and the backport patch. If using this patch, you don't need to integrate any of the above hooks. Susfs uses its own inline hooks.  

How to integrate Susfs?  
Go to the Susfs_Patches folder in this repository and select the patch according to the corresponding kernel version.  

Copy the patch to the source code root directory. Taking the v4.14 patch as an example, enter the following in the terminal: `git apply --reject 0001-susfs-Add-Susfs-v2.0.0-and-inline-hooks.patch`  

You can then apply the patch. If the patching fails, you will see the generated error file, which ends with xxxx.rej.  

You can manually apply the patch based on the error location in the .rej file.  

## KPM   
KPM is currently only compatible with v4.9.xxx (EMUI 9+) because subsequent kernel patching operations will cause devices with the v4.14.xxx (EMUI 10+) kernel to crash.  

It is speculated that this is due to EMUI 10 and above systems disabling USER LOCK.  

Go to the Susfs_Patches folder in this repository and select the patch according to the corresponding kernel version number. If you use this patch, you do not need to integrate any of the hooks mentioned above. Susfs uses its own inline hooks, and this patch includes a backport of KPM.  

Copy the patch to the source code root directory. Taking the v4.9 patch as an example, enter the following in the terminal: `git apply --reject 0001-susfs-4.9.xxx-KPM-backport-and-Susfs-v2.0.0-inline-h.patch` This will apply the patch. If the patching fails, you will see the generated error file, which ends with xxxx.rej.  

You can manually patch the error based on the location of the error in the .rej file.  