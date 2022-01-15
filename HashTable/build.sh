#!/bin/sh

gcc -std=c11 -W -g -DDEBUG -rdynamic ./test.c ../RBTree/RBTree.c ../SqList/SqList.c ../DuCirLinkList/DuCirLinkList.c ../SqStack/SqStack.c ../DlQueue/DlQueue.c ./HashTable.c
