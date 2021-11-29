#!/bin/sh

clear
rm -f a.out
gcc -std=c11 -W -gdwarf-2 -g3 -DDEBUG -g -rdynamic -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 test.c BTree.c ../DlQueue/DlQueue.c ../DuCirLinkList/DuCirLinkList.c ../SqStack/SqStack.c ../SqList/SqList.c
# echo copy--------
# rm -f ../../cc.DATA
# cp ../../reverse.DATA ../../cc.DATA
echo run---------
./a.out
# echo ===========================
# stat cc.DATA
# echo ===========================
# du -sh cc.DATA

# gcc check.c -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -o ../../dc