cmake_minimum_required(VERSION 3.0)

project("wmv-combine-anim" C CXX)

message("FBX_ROOT:" $ENV{FBX_ROOT})

set(exe_name "wmv-combine-anim")


add_executable(${exe_name} ${CMAKE_CURRENT_SOURCE_DIR}/anim_combine.cpp)

include_directories($ENV{FBX_ROOT}/include)

target_link_libraries( ${exe_name} debug $ENV{FBX_ROOT}/lib/vs2015/x86/debug/libfbxsdk-md.lib)
target_link_libraries( ${exe_name} optimized $ENV{FBX_ROOT}/lib/vs2015/x86/release/libfbxsdk-md.lib)