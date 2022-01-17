#!/bin/sh

gcc -std=c11 -W -gdwarf-2 -g3 -DDEBUG -g -rdynamic -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 test.c BTree.c ../DlQueue/DlQueue.c ../DuCirLinkList/DuCirLinkList.c ../SqStack/SqStack.c ../SqList/SqList.c ../common/default_func.c
