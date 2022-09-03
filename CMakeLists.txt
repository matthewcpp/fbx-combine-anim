cmake_minimum_required(VERSION 3.0)

if (DEFINED ENV{FBX_SDK_DIR})
	set(fbx_sdk_root $ENV{FBX_SDK_DIR})
elseif(DEFINED FBX_SDK_DIR)
	set(fbx_sdk_root ${FBX_SDK_DIR})
else()
	message(FATAL_ERROR "Please set FBX_SDK_DIR environment variable or pass -DFBX_SDK_DIR on command line when configuring")
endif()

set(target_name fbx-anim-combine)
project(${target_name} CXX)

message("Using FBX SDK:" ${fbx_sdk_root})

set(${target_name} fbx-anim-combine)

add_executable(${target_name} main.cpp)
target_compile_features(${target_name} PRIVATE cxx_std_17)
target_include_directories(${target_name} PRIVATE ${fbx_sdk_root}/include)

if (APPLE)
	target_link_libraries(${target_name} PRIVATE debug ${fbx_sdk_root}/lib/clang/debug/libfbxsdk.a)
	target_link_libraries(${target_name} PRIVATE optimized ${fbx_sdk_root}/lib/clang/release/libfbxsdk.a)

	target_link_libraries(${target_name} PRIVATE -lz -lxml2 -ldl -lpthread -liconv)
	set_target_properties(${target_name} PROPERTIES LINK_FLAGS "-framework Cocoa")
elseif(WIN32)
	target_compile_definitions(${target_name} PRIVATE -DFBXSDK_SHARED)
	target_link_libraries(${target_name} PRIVATE debug ${fbx_sdk_root}/lib/vs2019/x64/debug/libfbxsdk.lib)
	target_link_libraries(${target_name} PRIVATE optimized ${fbx_sdk_root}/lib/vs2019/x64/release/libfbxsdk.lib)
endif()