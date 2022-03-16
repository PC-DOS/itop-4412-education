#!/bin/bash

#指定交叉编译器前缀
CROSS_COMPILE=/usr/local/arm/arm-2009q3/bin/arm-none-linux-gnueabi-

#指定适用于C语言的编译器
CC=$(CROSS_COMPILE)gcc
#指定适用于C++语言的编译器
CXX=CC=$(CROSS_COMPILE)g++
#指定后处理器
STRIP=$(CROSS_COMPILE)strip

#执行make或make all时的编译和生成动作
all:
	$(CC) -W -o helloworld helloworld.c
	$(STRIP) helloworld

#执行make clean时的清理动作
clean:
	rm -f helloworld