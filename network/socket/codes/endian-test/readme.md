    
# 查看主机的字节序
1. 定义1个整数
2. 用指针指向该整数的地址
3. 获取指针的第0,1,2,3个地址
```
vim endian.c
##############################################################
#include <stdio.h>

int main()
{
    unsigned int x = 0x12345678;
    unsigned char *p = (unsigned char*)&x;
    printf("%0x %0x %0x %0x\n",p[0],p[1],p[2],p[3]);
    return 0;
}
##############################################################

vim Makefile
##############################################################
.PHONY:clean all
CC=gcc
CFLAGS=-Wall -g
BIN=endian
all:$(BIN)
$.o:%.c
    $(CC) $(CFLAGS) -c $< -o $@
clean:
    rm -f *.o $(BIN)
##############################################################

make 

./endian
#-----------------------------------------------------------------
# 78 56 34 12
#-----------------------------------------------------------------

```