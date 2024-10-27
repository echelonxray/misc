#!/bin/sh

set -e
set -x

gcc -Wall -Wextra \
  -Wno-unused-parameter -Wno-unused-variable \
  ./x11_application.c -o ./x11_application.out \
  -lX11
gcc -Wall -Wextra \
  -Wno-unused-parameter -Wno-unused-label \
  ./egl_x11_application.c -o ./egl_x11_application.out \
  -lX11 -lEGL -lGL -lm

exit 0
