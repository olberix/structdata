#!/bin/sh

rm -f cc.DATA
gcc test.c -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64
./a.out
echo ===========================
stat cc.DATA
echo ===========================
du -sh cc.DATA