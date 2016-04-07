#DLL=-DDLL_EXPORT
CC=gcc -Wall -O3 -g -msse2
INC=-I. -I../../ -I../../include -I../../build
CFLAGS=-DEXPORT -DUINCODE ${DLL} 

VPATH=./ ./png ./jpeg ./gif

objects= \
		 psx_image_io.o \
		 psx_image_loader.o \
		 psx_image_modules.o

all: libpsx_image.a

libpsx_image.a : ${objects}
	ar rcu libpsx_image.a ${objects}

%.o : %.c
	${CC} ${CFLAGS} -c $<  -o $@ ${INC}

clean:
	rm *.o *.a
