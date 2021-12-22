#!/bin/sh

gcc -std=c11 -W -g -DDEBUG -rdynamic ../DuCirLinkList/DuCirLinkList.c ./DlQueue.c ./test.c
