#ifndef _HTTPD_H_
#define _HTTPD_H_

//header key-value结构
typedef struct key_value
{
	char key[30];
	char value[120];
} kv;
//设置key-value对
void set_kv(kv *kv_, char *line_data);
//从给定套接字构造header结构
int build_header_struct(int *c_sock, kv **header);
//执行请求
void *request(void *c_sock);
//逐行读取请求报文                       (返回的指针需要手动释放)
char *message_line(int *c_sock);
//从给定的字符串中逐个获取以空格分开的各个字串 (返回的指针需要手动释放)
char *read_byspace(char *line_data, int *posi);
//以给定key从请求header中获取value        (返回的指针需要手动释放)
char *get_value(char *key, kv * header, int count);
//从给定相对路径文本获取html               (返回的指针需要手动释放)
char *get_html(char const *path);
//获取header                           (返回的指针需要手动释放)
char *get_header( char const *content_type, int content_length);

//responsehtml文本
void send_html(char *html, int *c_sock);
//用cgi程序响应请求
void cgi_handle(char const *url, int *c_sock);
//响应资源文件
void response_res(char const *path, char const *content_type, int *c_sock);
//响应404页面
void response_404(int *c_sock);
#endif