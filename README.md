<a href="https://github.com/sponsors/onecoolx" target="_blank"><img align="left" src="http://onecoolx.github.io/picasso/res/sponsor.png" height="28px"></a>
[![Windows Build](https://github.com/onecoolx/picasso/workflows/Windows%20Build/badge.svg)](https://github.com/onecoolx/picasso/actions/workflows/windows-cpp.yml) [![Linux Build](https://github.com/onecoolx/picasso/workflows/Linux%20Build/badge.svg)](https://github.com/onecoolx/picasso/actions/workflows/linux-cpp.yml) [![MacOSX Build](https://github.com/onecoolx/picasso/workflows/MacOSX%20Build/badge.svg)](https://github.com/onecoolx/picasso/actions/workflows/macosx-cpp.yml) [![Android Build](https://github.com/onecoolx/picasso/workflows/Android%20Build/badge.svg)](https://github.com/onecoolx/picasso/actions/workflows/android-cpp.yml)
[![CMake Build](https://github.com/onecoolx/picasso/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/onecoolx/picasso/actions/workflows/cmake-multi-platform.yml)
[![WebAssembly Build](https://github.com/onecoolx/picasso/actions/workflows/wasm.yml/badge.svg)](https://github.com/onecoolx/picasso/actions/workflows/wasm.yml)
[![CodeFactor](https://www.codefactor.io/repository/github/onecoolx/picasso/badge)](https://www.codefactor.io/repository/github/onecoolx/picasso)

Copyright (C) 2008 ~ 2025 Zhang Ji Peng  (onecoolx@gmail.com)

Picasso is a high quality vector graphic rendering library. It has high performance and low footprint. Picasso provides a set of high level 2D graphics API, which can be used to a GUI system, rendering postscript, rendering svg images and so on. It support path, matrix, gradient, pattern, image and truetype font. 

## **Features**
- Path Filling, Stroking
- Solid Color, Gradient, Pattern
- Stroke Dashing
- Linecap Butt, Round, Square
- Linejoin Miter, Round, Bevel
- Transform and Matrix
- Filling Rule
- Blur
- Shadow
- Clipping
- Compisiting
- Font and Text
- Image Decoders jpeg, png, webp, gif
- SVG

## **How to build**

#### linux:
```
1. Install cmake above v3.16 on your system
 $ sudo apt install cmake

2. Build project
 ./build_linux.sh
 cd proj
 make
```
#### windows:
```
1. Install cmake above v3.16 on your windows system and register path environment variables.
 c:\> winget install --id Kitware.CMake -e

2. Build project
 build_windows.bat
 cd proj
 <open "picasso.sln" with visual studio>
```
#### macosx:
```
1. Install cmake above v3.16 on your system
 $ brew install cmake

2. Build project
 ./build_macosx.sh
 cd proj
 <open "picasso.xcodeproj" with Xcode>
```

## **Documents**
[API Reference Documents](http://onecoolx.github.io/picasso/html/modules.html)

## **Gallery**
### **alpha blending**
![](http://onecoolx.github.io/picasso/res/flowers.png)

### **svg rendering**
![](http://onecoolx.github.io/picasso/res/tiger.png)

### **gis maps**
![](http://onecoolx.github.io/picasso/res/gis.png)

### **instrument**
![](http://onecoolx.github.io/picasso/res/clock.png)
