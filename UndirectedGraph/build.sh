#!/bin/sh

gcc -std=c11 -W -g -DDEBUG -rdynamic ./test.c ./UndirectedGraph.c ../common/default_func.c
