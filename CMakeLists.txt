cmake_minimum_required (VERSION 2.6)
project (Driver)

# Flags / definitions for the reference expand_response
set(CMAKE_C_FLAGS "-std=c99")
add_definitions(-D_POSIX_C_SOURCE=200112L -Dlinux -D_GNU_SOURCE)


add_executable(refresponse driver.cpp response.c)
add_executable(myresponse driver.cpp myresponse.cpp)
