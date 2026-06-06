#!/usr/bin/python
#Filename:Opensource_check.py

import os
import re
import sys
import time
import signal

check_flag = 0
compile_flag = 0
clear_flag = 0
help_flag = 0
dist_flag = 0

pwd = os.getcwd()
res = os.popen("env | grep LOGNAME")
logname = res.readline()[len("LOGNAME="):]
res.close();

def log_info(log):
    if '(' in log:
        log = log.replace('(', '\(')
    if ')' in log:
        log = log.replace(')', '\)')
    if 'successed' in log:
        os.system(' '.join(['echo -e \"\\033[40;32m' + log, '\\033[0m\"']))
    elif 'failed' in log or 'error' in log:
        os.system(' '.join(['echo -e \"\\033[40;31m' + log, '\\033[0m\"']))
    else:
        os.system(' '.join(['echo', log]))
    os.system(' '.join(['echo ', log, '>> check_log']))

def compile():
    symbol = ['-', '\\', '|', '/']
    if 'Makefile' not in os.listdir(pwd):
        os.system('echo -e \"\\033[40;31m There is no Makefile in current directory! \\033[0m\"')
	os.system("rm build_log")
	sys.exit()
    else:
        configs = os.listdir('arch/arm64/configs/')
        defconfig = configs[0]
        if -1 == defconfig.find('defconfig'):
            print 'Error, there is no defconfig file under arch/arm64/configs/\n'
            sys.exit()
        os.system(' '.join(['make ARCH=arm64 O=../out', defconfig]))
        print('\nStart to compile kernel. This operation will take 10 to 20 minutes. Please wait patiently. |=_=!|\n')

        pid = os.fork()
        if pid == 0:
            while True:
                for sym in symbol:
                    os.system("echo -e \"\\rKernel compiling... " + sym + " \\c\"")
                    time.sleep(0.250)

        time.sleep(0.200)
        os.system('make ARCH=arm64 O=../out -j8 > build_log 2>&1')

        os.kill(pid, signal.SIGKILL)
	os.wait()
        print "\nCompile complete!\nCompile information is in ../out/build_log\n"
        generate = os.listdir('../out/arch/arm64/boot/')
        if 'Image.gz' not in generate:
            os.system('echo -e \"\\033[40;31mCompile kernel failed! \\033[0m\"')
        else:
            if os.path.isdir('../../check_dir') == True:
                os.system('cp build_log ../../check_dir')
            os.system('mv build_log ../out')
            os.system('echo -e \"\\033[40;32mCompile kernel successed! \\033[0m\"')

def check():
    flag = 0

    log_info("********************Check list No.1 start******************")
    check_nbr1_dir = 'arch/arm64/configs/'
    check_nbr1_file = os.listdir(check_nbr1_dir)
    log_info("File in arch/arm64/configs/ is: %s" %check_nbr1_file[0])
    if len(check_nbr1_file) == 1 and -1 != check_nbr1_file[0].find('defconfig'):
        log_info("Check list No.1 successed!")
    else:
        flag = 1
        log_info("Check list No.1 failed!")
    log_info("********************Check list No.1 end********************")


    log_info("********************Check list No.2 start******************")
    check_nbr2_dir = 'drivers/vcodec/hi_vcodec/'
    check_nbr2_subdir = os.listdir(check_nbr2_dir)
    if 'venc' in check_nbr2_subdir:
        log_info('Get venc subdir')
        res1 = os.popen(' '.join(['find', check_nbr2_dir + 'venc -name *.c']))
        if '' == res1.readline():
            log_info(r"Can\'t get any .c file in venc")
            res2 = os.popen(' '.join(['find', check_nbr2_dir + 'venc -name *.S']))
            if res2.readline() == '':
                log_info(r"Can\'t get any .S file in venc")
            res2.close()
        else:
            log_info("Get one or more .c file(s) in venc")
            flag = 2
        res1.close()

    else:
        log_info(r"Can\'t get vend subdir")

    if 'vdec' in check_nbr2_subdir:
        vdec_subdir = os.listdir(check_nbr2_dir + 'vdec')
        if 'vfmw' in vdec_subdir:
            log_info('Get vdec/vfmw subdir')
            res = os.popen(' '.join(['find', check_nbr2_dir + 'vdec/vfmw -name *.c']))
            resfile = res.read()
            res.close()
            if len(resfile) == 0:
                res1 = os.popen(' '.join(['find', check_nbr2_dir + 'vdec/vfmw -name *.S']))
                resfile1 = res1.read()
                res1.close()
                if len(resfile1) == 0:
                    log_info(r"Can\'t get any .S file in vdec/vfmw")
                log_info(r"Can\'t get any .c file in vdec/vfmw")
            else:
                log_info(r"Get one or more .c file\(s\) in vdec/vfmw")
                flag = 2
        else:
            log_info(r"Can\'t get vdec/vfmw subdir")
    else:
        log_info(r"Can\'t find vdec subdir")

    if flag == 2:
        log_info("Check list No.2 failed!")
    else:
        log_info("Check list No.2 successed!")
    log_info("********************Check list No.2 end********************")


    log_info("********************Check list No.3 start******************")
    check_nbr3_dir = 'drivers/hisi/'
    if True == os.path.isdir('drivers/vendor/hisi/modem'):
        res = os.popen('find drivers/vendor/hisi/modem -name *.c | grep ccore')
        if '' == res.readline():
            log_info(r"Can\'t find files or directory named ccore")
        else:
            log_info("Find files or directory named ccore")
            flag = 3
        res.close()
    else:
        log_info("There is no path: drivers/vendor/hisi/modem")
        hisi_subdir = os.listdir(check_nbr3_dir)
        hisi_subdir_str = ' '.join(hisi_subdir)
        modem_ = hisi_subdir_str.find('modem_')
        if modem_ == -1:
            log_info("There is no directory named modem_xxxxxx under " + check_nbr3_dir)
            modem = hisi_subdir_str.find('modem')
            if modem == -1:
                log_info("Not found modem directory")
            else:
                res = os.popen(' '.join(["find", check_nbr3_dir + 'modem -name *.c | grep ccore']))
                result = res.readline()
                res.close()
            if result == '':
                log_info("Not found any files named ccore under drivers/hisi/modem")
            else:
                log_info("Found some files: " + result.replace('\n', ''))
        else:
            modem_x = hisi_subdir_str[modem : modem + len('modem_hixxxx')]
            res = os.popen(' '.join(["find", check_nbr3_dir + modem_x, '-name *.c | grep ccore']))
            result = res.readline()
            res.close()
            if '' == result:
                log_info("There is no file named ccore in " + check_nbr3_dir + modem_x)
            else:
                log_info("Found some file: " + result.replace('\n', ''))
                flag = 3

    if flag == 3:
        log_info("Check list No.3 failed")
    else:
        log_info("Check list No.3 successed")
    log_info("********************Check list No.3 end********************")


    log_info("********************Check list No.4 start******************")
    check_nbr4_dir = [
            'drivers/hisi/modem/include/taf/ccore',
            'drivers/hisi/modem/include/oam/gu/ccore',
            'drivers/hisi/modem/include/phy',
            'drivers/hisi/lpm3_debug',
            'drivers/hisi/perf_stat',
            'drivers/hisi/tele_mntn',
            'drivers/hisi/watchpoint',
            'drivers/hisi/freqdump']

    for sub_dir in check_nbr4_dir:
        if True == os.path.isdir(sub_dir):
            log_info("Error: " + sub_dir + " exist!")
            flag = 4
        else:
            log_info(sub_dir + " is non-existent")

    sub_dir = os.popen("find drivers/hisi/modem/platform/ -name ccore")
    sub_dir_res = sub_dir.readline()
    sub_dir.close()
    if sub_dir_res == '':
        log_info(r'drivers/hisi/modem/platform/*/ccore is non-existent!')
    else:
        log_info(r'drivers/hisi/modem/platform/*/ccore exist!')
        flag =4

    pro_subdir = os.listdir('drivers/hisi/modem/config/product/')
    pro_subdir_str = pro_subdir[0]
    if True == os.path.isdir('drivers/hisi/modem/config/product/' + pro_subdir_str + '/os/ccore'):
        log_info("Error: " + 'drivers/hisi/modem/config/product/' + pro_subdir_str + '/os/ccore exist!')
        flag = 4
    else:
        log_info('drivers/hisi/modem/config/product/' + pro_subdir_str + '/os/ccore is non-existent!')

    if flag == 4:
        log_info("Check list No.4 failed!")
    else:
        log_info("Check list No.4 successed!")

    log_info("********************Check list No.4 end********************")


    log_info("********************Check list No.5 start******************")
    check_nbr5_dir = 'drivers/vendor'
    if True == os.path.isdir(check_nbr5_dir):
        log_info(check_nbr5_dir + " exist! Please check it manually!")
        flag = 5
    else:
        log_info(check_nbr5_dir + r" is non-existent!")

    if flag == 5:
        log_info("Checklist No.5 failed! Please check it manually!")
    else:
        log_info("Checklist No.5 successed, result is NA!")

    log_info("********************Check list No.5 end********************")


    log_info("********************Check list No.6 start******************")
    check_nbr6_dirlist = [
            'drivers/hisi/ap/platform',
            'drivers/vendor/hisi/ap/platform',
            'drivers/hisi/modem/platform',
            'drivers/vendor/hisi/modem/platform'
    ]

    for sub_dir in check_nbr6_dirlist:
        if True == os.path.isdir(sub_dir):
            res = os.popen(' '.join(["grep -rn \"//\"", sub_dir]))
            res_str = res.readline()
            res.close()
            if res_str == '':
                log_info(r"There is no \"//...\" in " + sub_dir)
            else:
                log_info(r"There is are some \"//...\" in " + sub_dir)
                flag = 6

            res = os.popen(' '.join(["grep -rn \"*/\"", sub_dir]))
            res_str = res.readline()
            res.close()
            if res_str == '':
                log_info(r"There is no \"/*...*/\" in " + sub_dir)
            else:
                log_info(r"There is are some \"/*...*/\" in " + sub_dir)
                flag = 6 
        else:
            log_info(sub_dir + " is non-existent")

    if flag == 6:
        log_info("Checklist No.6 failed!")
    else:
        log_info("Checklist No.6 successed!")

    log_info("********************Check list No.6 end********************")


    log_info("********************Check list No.7 start******************")
    check_nbr7_dir = "drivers/scsi/ufs/"
    keyword = ['samsung', 'toshiba', 'hynix']
    if True == os.path.isdir(check_nbr7_dir):
        resfile = os.listdir(check_nbr7_dir)
        if 'ufs_health.h' in resfile:
            for word in keyword:
                res = os.popen(" ".join(['cat drivers/scsi/ufs/ufs_health.h | grep', word]))
                res_str = res.readline()
                res.close()
                if res_str == '':
                    log_info(r"There is no key word \"" + word + r"\" in the ufs_health.h")
                else:
                    flag = 7
                    log_info(r"Discovery key word \"" + word + r"\" in ufs_health.h")
        else:
            log_info("There is no file named ufs_health.h under " + check_nbr7_dir)
    else:
        log_info(check_nbr7_dir + " is non-existent")

    if flag == 7:
        log_info("Checklist No.7 failed!")
    else:
        log_info("Checklist No.7 successed!")
    log_info("********************Check list No.7 end********************")



    log_info("********************Check list No.8 start******************")
    check_nbr8_dir = "drivers/bootdevice"
    keyword = ['samsung', 'toshiba', 'hynix', 'SAMSUNG', 'TOSHIBA', 'HYNIX']
    if True == os.path.isdir(check_nbr8_dir):
        resfile = os.listdir(check_nbr8_dir)
        if 'proc_boot.c' in resfile:
            for word in keyword:
                res = os.popen(" ".join(['cat drivers/bootdevice/proc_boot.c | grep', word]))
                res_str = res.readline()
                res.close()
                if res_str == '':
                    log_info(r"There is no key word \"" + word + r"\" in the proc_boot.c")
                else:
                    flag = 8
                    log_info(r"Error:discovery key word \"" + word + r"\" in proc_boot.c")
        else:
            log_info("There is no file named proc_boot.c under " + check_nbr8_dir)
    else:
        log_info(check_nbr8_dir + " is non-existent")

    if flag == 8:
        log_info("Checklist No.8 failed!")
    else:
        log_info("Checklist No.8 successed!")
    log_info("********************Check list No.8 end********************")


    log_info("********************Check list No.9 start******************")
    check_nbr9_dir = "drivers/scsi/ufs/"
    keyword = [
            'ufshcd_send_inquiry',
            'ufshcd_read_device_health',
            'ufs_get_dev_health_info',
            'ufs_get_all_health_info']
    if True == os.path.isdir(check_nbr9_dir):
        resfile = os.listdir(check_nbr9_dir)
        if 'ufs_health_info.c' in resfile:
            for word in keyword:
                res = os.popen(" ".join(['cat drivers/scsi/ufs/ufs_health_info.c | grep', word]))
                res_str = res.readline()
                res.close()
                if res_str == '':
                    log_info(r"There is no key word \"" + word + r"\" in the ufs_health_info.c")
                else:
                    flag = 9
                    log_info(r"Discovery key word \"" + word + r"\" in ufs_health_info.c")
        else:
            log_info("There is no file named ufs_health_info.c under " + check_nbr9_dir)
    else:
        log_info(check_nbr9_dir + " is non-existent")

    if flag == 9:
        log_info("Checklist No.9 failed!")
    else:
        log_info("Checklist No.9 successed!")
    log_info("********************Check list No.9 end********************")


    log_info("********************Check list No.10 start******************")
    check_nbr10_dir = 'drivers/hisi'
    check_result = ["check_ifdef",
                    "check_define",
                    "check_undef",
                    "check_c_ifdef",
                    "check_c_define",
                    "check_c_undef"]

    for check_f in check_result:
        os.system(' '.join(["touch", check_f]))

    os.system(' '.join(['grep -rn ifdef', check_nbr10_dir, '> check_ifdef']))
    os.system(' '.join(['grep -rn undef', check_nbr10_dir, '> check_undef']))
    os.system(' '.join(['grep -rn define', check_nbr10_dir, '> check_define']))
    os.system('grep .c: check_ifdef > check_c_ifdef')
    os.system('grep .c: check_undef > check_c_undef')
    os.system('grep .c: check_define > check_c_define')

    c_file = 'xxx.c'

    f_if = open("check_c_ifdef", 'r')
    line_if = f_if.readline()
    while len(line_if) != 0:
        if '.c:' in line_if and 'cplusplus' not in line_if and c_file not in line_if:
            log_info(line_if.replace('\n', ''))

            c_match = re.compile(r'\w+.c:')
            c_find = c_match.search(line_if)
            c_file = c_find.group()
            c_file = c_file[:-1]

            m_match = re.compile(r'\s\(*\w+\)*\s*$')
            m_find = m_match.search(line_if)
            macros = m_find.group()
            macros = macros[1:]

            if '(' in macros:
	    	    macros = macros.replace('(', '')
            if ')' in macros:
	    	    macros = macros.replace(')', '')
            if '\r' in macros:
	    	    macros = macros.replace('\r', '')
            if '\n' in macros:
	    	    macros = macros.replace('\n', '')

            l_match = re.compile(r':\w+:')
            l_find = l_match.search(line_if)
            line_n_if = l_find.group()
            line_n_if = line_n_if[1:-1]

            res = os.popen(' '.join(["grep", macros, "check_c_define | grep", c_file]))
            res_str = res.readline()
            res.close()

            if res_str != '':
                line_def = l_match.search(res_str)
                line_n_def = line_def.group()
                line_n_def = line_n_def[1:-1]
                if int(line_n_def) < int(line_n_if):
                    log_info(res_str.replace('\n', ''))
                else:
                    ret = os.popen(' '.join(['grep', macros, 'check_c_undef | grep', c_file]))
                    line_undef = ret.readline()
                    ret.close()
                    if line_undef != '':
                        line_n_udf = l_match.search(line_undef)
                        line_n_un = line_n_udf.group()
                        line_n_un = line_n_un[1:-1]
                        if int(line_n_if) < int(line_n_un) and int(line_n_un) < int(line_n_def):
                            log_info(line_undef.replace('\n', ''))
                            log_info(res_str.replace('\n', ''))
                        else:
                            log_info("Discovery some error in " + c_file + ":" + line_n_if + ". There is no define of " + macros + ". Please check it manually!")
                            flag = 10

                    else:
                        log_info("Discovery some error in " + c_file + ":" + line_n_if + ". There is no define of " + macros + ". Please check it manually!")
                        flag = 10

            else:
                log_info("Discovery some error in " + c_file + ":" + line_n_if + ". There is no define of " + macros + ". Please check it manually!")
                flag = 10

        line_if = f_if.readline()

    f_if.close()

    if flag == 10:
        log_info("Checklist No.10 failed!")
    else:
        log_info("Checklist No.10 successed!")
    log_info("********************Check list No.10 end********************")

    for check_f in check_result:
        os.system(' '.join(["mv", check_f, "../../check_dir"]))
    os.system("mv check_log ../../check_dir")

def clear():
    print "make distclean..."
    os.system('make ARCH=arm64 distclean')
    if dist_flag == 0:
        print "Delet generate files in ../out"
        os.system('rm -rf ../out')
        os.system("mv Opensource_check.py ../../check_dir")

def help():
    print '''\n\tOptions of Opensource_check.py\n'''
    print '''\t-h or --help: Print the function of every option. If this option is selected, the others options will be invalid.\n'''
    print '''\t-k or --check: This tool will check the files under current directory, $(pwd) must be kernel root directory, according to Opensource checklist.\n'''
    print '''\t-p or --compile: This option need a paramter, which is the directory where \"prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin\" locat in. It 
will be set in environment variable, then kernel will be compiled. When compile finished, the generate files will be located in ../out directory. The compile information
is in the file named build_log. This file will be moved to ../out if the compile result is successful, otherwise, it will be still in kernel root directory.\n'''
    print '''\t-r or --clear: This tool will execute \"make ARCH=arm64 distclean\" and remove the \"../out\" directory'''
    print '''\t-d or --dist: This tool will execute \"make ARCH=arm64 distclean\" only.'''
    print '''\t-a or --all: This option is equal to -k -p -r, and this need a paramter too, which is the same as -p option\n'''


'''**************************************Main function*****************************************'''

if len(sys.argv) == 1:
    check_flag = 1
    compile_flag = 0
    clear_flag = 0
    help_flag = 0
else:
    for option in sys.argv[1:]:
        print ("option is :%s" %option)
        if option.startswith('-'):
	    if option == '-h':
                help_flag = 1
                compile_flag = 0
                check_flag = 0
                clear_flag = 0
                break
	    elif option == '-k':
                check_flag = 1
            elif option == '-p':
                compile_flag = 1
	    elif option == '-r':
                clear_flag = 1
            elif option == '-a':
                if len(sys.argv) != 3:
                    print "Error, behind \"-a\" option is the path of crosscompiler"
                    sys.exit()
                elif False == os.path.isdir(sys.argv[-1]):
                    print "Error, you need specify the path where crosscompiler in!"
                    sys.exit()
                compile_flag = 1
                check_flag = 1
                clear_flag = 1
            elif option == '-d':
                clear_flag = 1
                dist_flag = 1
            else:
                print ("No such option", option)
                sys.exit()

        if option.startswith('--'):
            if option == '--help':
                help_flag = 1
                compile_flag = 0
                check_flag = 0
                clear_flag = 0
                break
            elif option == '--check':
                check_flag = 1
            elif option == '--compile':
                compile_flag = 1
            elif option == '--clear':
                clear_flag = 1
            elif option == '--all':
                compile_flag = 1
                check_flag = 1
                clear_flag = 1
            elif option == '--dist':
                clear_flag = 1
                dist_flag = 1
            else:
                print "No such option", option

if help_flag == 1:
    help()

if check_flag == 1:
    os.system('mkdir -p ../../check_dir')
    os.system('touch check_log')
    check()

if compile_flag == 1:
    if True == os.path.isdir(sys.argv[-1]):
        if sys.argv[-1][-1] == '/':
            sys.argv[-1] = sys.argv[-1][:-1]
        if True == os.path.isdir(sys.argv[-1] + '/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin'):
            env_path = sys.argv[-1] + '/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin'
            os.environ['PATH'] = '/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:' + env_path
        else:
            print ("Error! There is no \"/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin\" under ", sys.argv[-1])
            sys.exit()
    else:
        res = os.popen('env | grep PATH=/')
        path = res.readline()
        res.close()
        if 'aarch64-linux-android-' not in path:
            print ("There is no cross-compiler path in environment variable PATH! Please set it!\n")
            sys.exit()

    res = os.popen('env | grep CROSS_COMPILE')
    cross_compile = res.readline().replace('\n', '')
    res.close()
    if cross_compile != 'CROSS_COMPILE=aarch64-linux-android-':
        os.environ['CROSS_COMPILE'] = 'aarch64-linux-android-'
    os.system('mkdir -p ../out')
    os.system('touch build_log')
    compile()

if clear_flag == 1:
    clear()




