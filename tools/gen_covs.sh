#!/bin/sh

lcov -d . -c -o picasso.info
lcov --remove picasso.info -o picasso.info '/usr/*' '*/proj/*' '*/unit_tests/*' '*/third_party/*'
genhtml -o result picasso.info
