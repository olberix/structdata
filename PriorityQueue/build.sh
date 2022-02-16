#!/bin/sh

gcc -std=c11 -W -g -DDEBUG -rdynamic ./PriorityList.c ./test.c
