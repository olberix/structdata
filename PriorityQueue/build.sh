#!/bin/sh

gcc -std=c11 -W -g -DDEBUG -rdynamic ../SqList/SqList.c ../common/default_func.c ./PriorityQueue.c ./test.c
