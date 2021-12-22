#!/bin/sh

gcc -std=c11 -W -g -DDEBUG -rdynamic ./DuCirLinkList.c ./test.c
