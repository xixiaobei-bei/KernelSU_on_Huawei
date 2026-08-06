LOCAL_PATH:= $(call my-dir)

libc_common_src_files:= \
        src/vsprintf_s.c \
        src/wmemmove_s.c\
        src/strncat_s.c\
        src/vsnprintf_s.c\
        src/fwscanf_s.c\
        src/scanf_s.c\
        src/strcat_s.c\
        src/sscanf_s.c\
        src/secureprintoutput_w.c\
        src/wmemcpy_s.c\
        src/wcsncat_s.c\
        src/secureprintoutput_a.c\
        src/secureinput_w.c\
        src/memcpy_s.c\
        src/fscanf_s.c\
        src/vswscanf_s.c\
        src/secureinput_a.c\
        src/sprintf_s.c\
        src/memmove_s.c\
        src/swscanf_s.c\
        src/snprintf_s.c\
        src/vscanf_s.c\
        src/vswprintf_s.c\
        src/wcscpy_s.c\
        src/vfwscanf_s.c\
        src/memset_s.c\
        src/wscanf_s.c\
        src/vwscanf_s.c\
        src/strtok_s.c\
        src/wcsncpy_s.c\
        src/vfscanf_s.c\
        src/vsscanf_s.c\
        src/wcstok_s.c\
        src/securecutil.c\
        src/gets_s.c\
        src/swprintf_s.c\
        src/strcpy_s.c\
        src/wcscat_s.c\
        src/strncpy_s.c


include $(CLEAR_VARS)

LOCAL_ODM_MODULE := true
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := \

LOCAL_C_INCLUDES := vendor/hisi/bsp/libc_sec/include \
                    vendor/hisi/bsp/libc_sec/src

LOCAL_SRC_FILES:=$(libc_common_src_files)

LOCAL_CFLAGS:= -Wall -s -DNDEBUG -O1 -DSECUREC_SUPPORT_STRTOLD=1 
LOCAL_MULTILIB := both

LOCAL_MODULE:= libc_sec_hisi

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := \

LOCAL_C_INCLUDES := vendor/hisi/bsp/libc_sec/include \
                    vendor/hisi/bsp/libc_sec/src


LOCAL_SRC_FILES:=$(libc_common_src_files)

LOCAL_CFLAGS:= -Wall -s -DNDEBUG -O1 -DSECUREC_SUPPORT_STRTOLD=1 
LOCAL_MULTILIB := both

LOCAL_MODULE:= libc_sec_hisi

include $(BUILD_STATIC_LIBRARY)

#include $(CLEAR_VARS)

#LOCAL_MODULE_TAGS := eng

#LOCAL_SHARED_LIBRARIES := libc libcutils libc_sec_hisi

#LOCAL_C_INCLUDES := vendor/hisi/bsp/libc_sec/include \
                    bionic/libc/include \
                    vendor/hisi/bsp/libc_sec/test/basecases \
                    vendor/hisi/bsp/libc_sec/test \
                    vendor/hisi/bsp/libc_sec/src

#LOCAL_SRC_FILES := test/testmain.c \
                   test/testutil.c \
                   test/pub_funcs.c \
                   test/basecases/memmove_test.c \
                   test/basecases/memset_s_test.c\
                   test/basecases/strcpytest.c\
                   test/basecases/dopra_comptest.c\
                   test/basecases/wscanftest.c\
                   test/basecases/scanftest.c\
                   test/basecases/mem_perf.c\
                   test/basecases/swprintftest.c\
                   test/basecases/strcattest.c\
                   test/basecases/memcpytest.c\
                   test/basecases/gets_test.c\
                   test/basecases/sprintftest.c\
                   test/basecases/str_perf.c\
                   test/basecases/strtoktest.c \
                   test/comptest/compare_sscanftest_float.c \
                   test/comptest/compare_printftest_float.c\
                   test/comptest/compare_printftest_str_s.c\
                   test/comptest/compare_sscanftest_int_i.c\
                   test/comptest/compare_printftest_int_d.c\
                   test/comptest/compare_sscanftest_other.c\
                   test/comptest/compare_sscanftest_int_d.c\
                   test/comptest/compare_printftest_other.c\
                   test/comptest/compare_printftest_str_c.c\
                   test/comptest/compare_sscanftest_str_c.c\
                   test/comptest/compare_sscanftest_str_s.c\
                   test/comptest/compare_printftest_int_i.c

#LOCAL_CFLAGS := -Wall -DSECUREC_SUPPORT_STRTOLD=1 
#LOCAL_MODULE:= sectest

#include $(BUILD_EXECUTABLE)
