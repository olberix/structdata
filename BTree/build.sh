#!/bin/sh

rm -f cc.DATA
gcc -std=c11 -W -g -DDEBUG -rdynamic -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 test.c 
./a.out
echo ===========================
stat cc.DATA
echo ===========================
du -sh cc.DATA
