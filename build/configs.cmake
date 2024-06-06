# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

if (WIN32)
    set(CMAKE_C_FLAGS_DEBUG  "${CMAKE_C_FLAGS_DEBUG} /D_DEBUG /W3 /WX /Gm- /Gd /GR- /Zc:wchar_t-")
    set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} /D_DEBUG /W3 /WX /Gm- /Gd /GR- /Zc:wchar_t-")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /W3 /WX /Ot /GF /Gm- /GR- /Gy /GL /Zc:wchar_t-")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /W3 /WX /Ot /GF /Gm- /GR- /Gy /GL /Zc:wchar_t-")
elseif (UNIX AND NOT APPLE)
    set(CMAKE_C_FLAGS_DEBUG "-D_DEBUG -O0 -Wall -g -Wno-unused-result")
    set(CMAKE_CXX_FLAGS_DEBUG "-D_DEBUG -O0 -Wall -g -std=c++11 -fno-rtti -fno-exceptions -Wno-unused-result -Wno-register -Wno-attributes")
    set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG -O3 -Wall -Wno-unused-result")
    set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -Wall -std=c++11 -fno-rtti -fno-exceptions -Wno-unused-result -Wno-register -Wno-attributes")
elseif (APPLE)
endif()
