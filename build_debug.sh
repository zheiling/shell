#!/bin/sh

gcc -Wall main.h edit_line.c helpers.c inpt.c list.c pipes.c process_line.c prog.c main.c -o zhshell -fsanitize=address,leak,undefined