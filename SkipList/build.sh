#!/bin/sh

gcc -std=c11 -W -g -DDEBUG -rdynamic ./test.c ./SkipList.c ../common/default_func.c
