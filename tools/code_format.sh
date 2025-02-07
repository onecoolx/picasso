#!/bin/sh

astyle --options=code_style.ini --recursive "../src/*.cpp,*.c,*.h"
astyle --options=code_style.ini --recursive "../ext/*.cpp,*.c,*.h"
astyle --options=code_style.ini --recursive "../include/*.h"
