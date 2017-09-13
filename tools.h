#ifndef _TOOLS_H_
#define _TOOLS_H_
#include <string.h>
#include <stdlib.h>

//从begin索引处截取length个字符 (返回的指针需要手动释放)
char *substr(char const *strsrc, int begin, int length);
//从begin索引处截取到结束       (返回的指针需要手动释放)
char *substr_toend(char const *strsrc, int begin);
//从字符串右侧开始截取字符       (返回的指针需要手动释放)
char *subrstr(char const *data, int begin, int length);
//从右侧截取length长个字符      (返回的指针需要手动释放)
char *subrstr_bylen(char const *data, int length);
//销毁char指针内存
void free_char(char *ptr);
//查找字符转string的第一个字符ch所在的索引
int charAt(char const *string, char ch);
#endif