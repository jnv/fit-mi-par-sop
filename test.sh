#!/bin/sh
PROGRAM=sop-s

# <n> <c> <a>
./$PROGRAM 30 30 2 < in00.txt
./$PROGRAM 100 30 3 < in01.txt
