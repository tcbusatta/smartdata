cmake_minimum_required (VERSION 2.4)
project (smartdata)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Valid values: Release, Debug" FORCE)
endif(NOT CMAKE_BUILD_TYPE)

ADD_DEFINITIONS(-D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64)

if(CMAKE_BUILD_TYPE MATCHES Debug)
	ADD_DEFINITIONS(-D_DEBUG)
elseif(CMAKE_BUILD_TYPE MATCHES Release)
else(CMAKE_BUILD_TYPE MATCHES Debug)
	message(FATAL_ERROR "Select build type")
endif(CMAKE_BUILD_TYPE MATCHES Debug)

include_directories (
	$(SMARTDATA_SOURCE_DIR)include
)

set(CMAKE_CXX_FLAGS_DEBUG "-Wall -O0 -pg -Wfatal-errors")
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -O3 -Wfatal-errors -Wuninitialized -fno-strict-aliasing -fomit-frame-pointer")

add_executable (smartdata
	src/main.cpp
)

target_link_libraries (smartdata ${ADDITIONAL_LIBS} pthread rt) 
