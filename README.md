![Windows Build](https://github.com/onecoolx/picasso/workflows/Windows%20Build/badge.svg) ![Linux Build](https://github.com/onecoolx/picasso/workflows/Linux%20Build/badge.svg) ![MacOSX Build](https://github.com/onecoolx/picasso/workflows/MacOSX%20Build/badge.svg) ![Android Build](https://github.com/onecoolx/picasso/workflows/Android%20Build/badge.svg)
[![CodeFactor](https://www.codefactor.io/repository/github/onecoolx/picasso/badge)](https://www.codefactor.io/repository/github/onecoolx/picasso)

Copyright (C) 2008 ~ 2023 Zhang Ji Peng  (onecoolx@gmail.com)

Picasso is a high quality vector graphic rendering library. It has high performance and low footprint. Picasso provides a set of high level 2D graphics API, which can be used to a GUI system, rendering postscript, rendering svg images and so on. It support path, matrix, gradient, pattern, image and truetype font. 

## **How to build**

#### linux:
```
1. automake & autoconf
./autogen.sh
./configure
make
sudo make install;


2. gyp build
./build_linux.sh
cd proj
make
```
#### windows:
```
1. Install Active Python 2.7 on your windows system and register path environment variables.
2. Build project
./build_windows.bat
cd vcproj
<open "picasso.sln" with visual studio>
```
#### macosx:
```
1. Install python 2.7 on your shell environment.
2. Build project
./build_macosx.sh
cd proj
<open "picasso.xcodeproj" with Xcode>
```

## **Gallery**
### **alpha blending**
![](http://onecoolx.github.io/picasso/res/flowers.png)

### **svg rendering**
![](http://onecoolx.github.io/picasso/res/tiger.png)

### **gis maps**
![](http://onecoolx.github.io/picasso/res/gis.png)

### **instrument**
![](http://onecoolx.github.io/picasso/res/clock.png)
