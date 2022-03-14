#!/bin/sh

gcc -std=c11 -W -g -DDEBUG -rdynamic ./test.c ./DirectedGraph.c ../common/default_func.c ../SkipList/SkipList.c ../DlQueue/DlQueue.c ../DuCirLinkList/DuCirLinkList.c ../SqStack/SqStack.c ../SqList/SqList.c ../PriorityQueue/PriorityQueue.c
