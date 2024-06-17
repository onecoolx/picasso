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
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_WEAK "YES")
    set(CMAKE_XCODE_ATTRIBUTE_WEAK_REFERENCES_IN_MANUAL_RETAIN_RELEASE "YES")
    set(CMAKE_XCODE_ATTRIBUTE_GCC_ENABLE_CPP_EXCEPTIONS "NO")
    set(CMAKE_XCODE_ATTRIBUTE_GCC_ENABLE_CPP_RTTI "NO")
endif()
