# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

IF (WIN32)
ELSEIF (UNIX AND NOT APPLE)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -g -fPIC -std=c++11 -fno-rtti -fno-exceptions -Wno-unused-result -Wno-register -Wno-attributes")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -fPIC -std=c++11 -fno-rtti -fno-exceptions -Wno-unused-result -Wno-register -Wno-attributes")
ELSEIF (APPLE)
ENDIF()
