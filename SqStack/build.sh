#!/bin/sh

gcc -std=c11 -W -g -DDEBUG -rdynamic ../SqList/SqList.c ./SqStack.c ./test.c
