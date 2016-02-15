Picasso is a high quality vector graphic rendering library. It has high performance and low footprint. Picasso provides a set of high level 2D graphics API, which can be used to a GUI system, rendering postscript, rendering svg images and so on. It support path, matrix, gradient, pattern, image and truetype font. 

=================================================
### **alpha blending**
![](https://github.com/onecoolx/picasso/blob/master/demos/flowers.png)

### **svg rendering**
![](https://github.com/onecoolx/picasso/blob/master/demos/tiger.png)

### **gis maps**
![](https://github.com/onecoolx/picasso/blob/master/demos/gis.png)

### **instrument**
![](https://github.com/onecoolx/picasso/blob/master/demos/clock.png)


HOW TO BUILD:

------------------------------------
linux:
1. automake & autoconf
./autogen.sh
./configure
make
sudo make install;

2. gyp build
./build_proj.sh
cd proj
make


------------------------------------
windows:
1. Install Active Python 2.7 on your windows system and register path environment variables.

2. Build project
./build_proj.bat
cd vcproj
<open "picasso.sln" with visual studio>
