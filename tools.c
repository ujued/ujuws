#include "tools.h"

//从begin索引处截取length个字符
char *substr(char const *strdata, int begin, int length)
{
	int end = strlen(strdata) - 1;
	
	if(begin > end)
	{
		return NULL;
	}else if(begin < 0)
	{
		return NULL;
	}else if(length < 0)
	{
		return NULL;
	}else if(length > (end + 1 - begin))
	{
		length = end + 1 - begin;
	}
	char *dest = (char *)malloc(length + 1);
	int i = 0;
	for(; i < length; i++)
	{
		dest[i] = strdata[begin + i];
	}
	dest[i] = '\0';
	return dest;
}
//从begin索引处截取到结束
char *substr_toend(char const *strdata, int begin)
{
	return substr(strdata, begin, strlen(strdata) - begin);
}
//从字符串右侧开始截取字符
char *subrstr(char const *data, int begin, int length)
{
	int data_len = strlen(data);
	if(begin < 0 || begin >= data_len || length < 1)
	{
		return NULL;
	}else if(length > begin + 1)
	{
		length = begin + 1;
	}
	int i = 0;
	char *sub_data = (char *)malloc(length + 1);
	sub_data[length] = '\0';
	for(i = begin; i > begin - length; i--)
	{
		sub_data[i-begin + length - 1] = data[i];
	}
	return sub_data;
}
//从右侧截取length长个字符
char *subrstr_bylen(char const *data, int length)
{
	return subrstr(data, strlen(data) - 1, length);
}
//销毁char指针内存
void free_char(char *ptr)
{
	if(ptr != NULL)
	{
		free(ptr);
		ptr = NULL;
	}
}
//查找字符转string的第一个字符ch所在的索引
int charAt(char const *string, char ch)
{
	int len = strlen(string);
	int i = 0;
	for(;i < len; i++)
	{
		if(string[i] == ch)
		{
			return i;
		}
	}
	return -1;
}