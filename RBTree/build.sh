#!/bin/sh

gcc -std=c11 -W -g -DDEBUG -rdynamic ./test.c ./RBTree.c ../SqList/SqList.c ../SqStack/SqStack.c ../DuCirLinkList/DuCirLinkList.c ../DlQueue/DlQueue.c ../common/default_func.c
