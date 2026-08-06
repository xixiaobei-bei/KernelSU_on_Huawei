cmake_minimum_required(VERSION 3.10)
message ("entering..." ${CMAKE_CURRENT_SOURCE_DIR})
get_filename_component(TOP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../ ABSOLUTE)

#/vendor
set(VENDOR_TOP_ROOT ${TOP_ROOT}/vendor)
message(${VENDOR_TOP_ROOT})

#set compiler
set(LLVM_PATH ${TOP_ROOT}/prebuilts/clang/host/linux-x86/clang-r353983c)
set(CMAKE_CXX_COMPILER ${LLVM_PATH}/bin/clang++)
set(CMAKE_C_COMPILER ${LLVM_PATH}/bin/clang)

Project(res_mgr_driver_test CXX)

# import googletest to project
add_subdirectory( "${VENDOR_TOP_ROOT}/../external/googletest" GTest)
message("googletest folder ...  ${VENDOR_TOP_ROOT}/../external/googletest")
set(gtest_include "${VENDOR_TOP_ROOT}/../external/googletest/googletest/include")
set(gmock_include "${VENDOR_TOP_ROOT}/../external/googletest/googlemock/include")
include_directories(${gtest_include})
include_directories(${gmock_include})


#import mockcpp to project
set(mockcpp_src ${VENDOR_TOP_ROOT}/huawei/chipset_common/modules/mockcpp)
add_library(mockcpp STATIC IMPORTED)
set_property(TARGET mockcpp PROPERTY IMPORTED_LOCATION ${mockcpp_src}/libmockcpp.a)
include_directories(${mockcpp_src}/include)

Project(res_mgr_driver C)

#import llt/stub
include_directories(${VENDOR_TOP_ROOT}/hisi/llt/display/stub/linux/include)
add_subdirectory(${VENDOR_TOP_ROOT}/hisi/llt/display/stub/linux/src libdisplay_kernel_stub)

#---------changed to your own include path ------------------------------------
#DPU
set(DPU_ROOT ${VENDOR_TOP_ROOT}/hisi/ap/kernel/drivers/video/dkmd/dpu)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${DPU_ROOT}/common/utils)
include_directories(${DPU_ROOT}/common/config)
#------------------------------------------------------------------------------

include_directories(${TOP_ROOT}/external/googletest/googletest/include)
message(${TOP_ROOT}/external/googletest/googletest/include)

set(SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/opr_mgr.c
	${CMAKE_CURRENT_SOURCE_DIR}/dvfs.c)

#changed to your own execution file name --------------------------------------
SET(EXECUTABLE_NAME res_mgr_driver_run)
#------------------------------------------------------------------------------


#build executable file and run google test
add_executable(${EXECUTABLE_NAME} ${SOURCES})

#changed to your dependent lib ------------------------------------------------
set(DEPENDENT_LIBS display_kernel_stub)
#------------------------------------------------------------------------------

add_test(NAME res_mgr_driver_tst COMMAND ${EXECUTABLE_NAME})
target_link_libraries(${EXECUTABLE_NAME} PUBLIC ${DEPENDENT_LIBS} gtest gtest_main mockcpp)



