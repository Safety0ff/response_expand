cmake_minimum_required (VERSION 2.6)
project (Driver)

if (EXISTS response.c)
  # N.B. GCC specific flags/defs
  set(CMAKE_C_FLAGS "-std=c99")
  add_definitions(-D_POSIX_C_SOURCE=200112L -Dlinux -D_GNU_SOURCE)
  add_executable(refresponse driver.cpp response.c)
endif ()

add_executable(myresponse driver.cpp myresponse.cpp)

# When on 64bit, also build 32bit
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
  add_executable (myresponse32 driver.cpp myresponse.cpp)
  # N.B. GCC specific flags
  set_target_properties (myresponse32 PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
endif ()
