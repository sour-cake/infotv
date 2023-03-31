#!/bin/sh

MODE="$1"
PROGRAM="$2"

[ -z "$MODE" -o -z "$PROGRAM" ] && printf 'Usage: %s <mode> <program>\n' "$0" && exit 1
[ "$MODE" != 'debug' -a "$MODE" != 'release' ] && printf 'No such mode: "%s"\n' "$PROGRAM" && exit 1
[ \! -f "src/$PROGRAM.c" ] && printf 'No such program: "%s"\n' "$PROGRAM" && exit 1

LIBS="$(grep 'itv-libraries:' src/$PROGRAM.c | cut -d':' -f2)"

[ -z "$CC" ] && CC="gcc"

[ "$MODE" = 'debug' ] \
	&& MODEFL='-fsanitize=undefined -g3 -O0' \
	|| MODEFL='-O'

echo $CC \
	-std=c99 \
	-Wall -Wextra -Werror \
	$(pkg-config --cflags $LIBS) \
	src/$PROGRAM.c src/common.c \
	$MODEFL \
	-o bin/infotv-client \
	$(pkg-config --libs $LIBS)
