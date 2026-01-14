#!/bin/sh

GCC_VERSION=`gcc --version | head -n1 | awk '{print $3}'`
GCC_MAJOR_VERSION=$(echo $GCC_VERSION | cut -d. -f1,1)

lcov --ignore-errors mismatch --gcov-tool /usr/bin/gcov-${GCC_MAJOR_VERSION} -d . -c -o picasso.info
lcov --remove picasso.info -o picasso.info '/usr/*' '*/proj/*' '*/unit_tests/*' '*/third_party/*'
genhtml -o result picasso.info
