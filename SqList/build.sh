#!/bin/sh

gcc -std=c11 -W -g -DDEBUG -rdynamic ./SqList.c ./test.c
