/* Author: ujued. 
 * 2017年09月09日*/
 
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include "httpd.h"
#include "tools.h"

int main(int argc, char **argv)
{
    int port = 8002;
    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);
    local_addr.sin_addr.s_addr = INADDR_ANY;
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sock == -1)
    {
        perror("socket error!");
        return -1;
    }
    if(bind(server_sock, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0)
    {
        perror("bind error!");
        return -1;
    }
    if(listen(server_sock, 20) < 0)
    {
        perror("listen error!");
        return -1;
    }
    printf("server running on port %d.\n", port);
    socklen_t lcoal_addr_length = sizeof(local_addr);
    pthread_t tid;
	
	//监听请求
    while(1)
    {
        int c_sock = accept(server_sock, (struct sockaddr *)&local_addr, &lcoal_addr_length);
        if(c_sock == -1)
        {
            perror("accept error!");
            return -1;
        }
        if(pthread_create(&tid, NULL, request, &c_sock) != 0)
        {
            perror("thread error！");
            return -1;
        }
    }
    //关闭服务端套接字
    close(server_sock);
	return 0;
}
//执行请求
void *request(void *c_sock)
{
    int client_sock = *(int *)c_sock;
	int i = 0;
	char *line_data = message_line(c_sock);
	char *method = read_byspace(line_data, &i);
	char *url = read_byspace(line_data, &i);
	char *protocal = read_byspace(line_data, &i);
	//默认请求
	if(strcmp(url, "/") == 0)
	{
		free(url);
		url = (char *)malloc(12);
		strcpy(url, "/index.html\0");
	}
	//请求cgi
	if(strcmp(substr(url, 1, 7), "cgi-bin") == 0)
	{
		int url_len = strlen(url);
		if(url_len == 8 || url_len == 9)
		{
			free(url);
			url = (char *)malloc(20);
			strcpy(url, "/cgi-bin/index.cgi\0");
		}
		cgi_handle(substr_toend(url, 9), c_sock);
	}
	//请求资源
	else
	{
		char content_type[32];
		char *body = NULL;
		char body_flag = 'f';
		char *suffix = subrstr_bylen(url, 4);
		//判断请求多媒体资源格式
		if(strcmp(suffix, ".png") == 0)
		{
			strcpy(content_type, "image/png");
		}else if(strcmp(suffix, ".jpg") == 0)
		{
			strcpy(content_type, "image/jpg");
		}else if(strcmp(suffix, ".gif") == 0)
		{
			strcpy(content_type, "image/gif");
		}else if(strcmp(suffix, ".zip") == 0)
		{
			strcpy(content_type, "application/zip");
		}else if(strcmp(suffix, ".mp3") == 0)
		{
			strcpy(content_type, "audio/mp3");
		}else if(strcmp(suffix, ".doc") == 0)
		{
			strcpy(content_type, "application/msword");
		}else
		{
			free(suffix);
			suffix = subrstr_bylen(url, 5);
			if(strcmp(suffix, ".jpeg") == 0)
			{
				strcpy(content_type, "image/jpeg");
			}else if(strcmp(suffix, ".html") == 0)
			{
				strcpy(content_type, "text/html;charset=utf-8");
				body_flag = 't';
			}else if(strcmp(suffix, ".docx") == 0)
			{
				strcpy(content_type, "application/msword");
			}else
			{
				strcpy(content_type, "application/octet-stream");
			}
		}
		if(body_flag == 't')
		{
			body = get_html(url);
			if(body == NULL)
			{
				response_404(c_sock);
			}else
			{
				int body_len = strlen(body);
				char *resp_header = get_header(content_type, body_len);
				send(client_sock, resp_header, strlen(resp_header), 0);
				send(client_sock, body, body_len, 0);
			}
		}else{
			char rel_url[2048];
			sprintf(rel_url, ".%s", url);
			response_res(rel_url, content_type, c_sock);
		}
		
		free_char(suffix);
		free_char(body);
	}
	//关闭客户请求套接字
    close(client_sock);
	//释放无用堆内存
	free(line_data);
	free(method);
	free(protocal);
	free(url);
}
//逐行读取请求报文
char *message_line(int *c_sock)
{
	int client_sock = *c_sock;
	char temp[2048];
	int i = 0;
	char c = '\0';
	while(1)
	{
		int re_code = recv(client_sock, &c, 1, 0);
		if(c == '\r' || c == '\n')
		{
			if(i == 0) return NULL;
			recv(client_sock, &c, 1, 0);
			char *line_data = (char *)malloc(i + 1);
			memcpy(line_data, temp, i);
			line_data[i] = '\0';
			return line_data;
		}
		temp[i++] = c;
	}
	return NULL;
}
//从给定的字符串中逐个获取以空格分开的各个字串
char *read_byspace(char *line_data, int *posi)
{
	int i = *posi;
	char c = '\0';
	char *buf = (char *)malloc(2033);
	int j = 0;
	while(1)
	{
		c = line_data[i++];
		if(c == ' ' || c == '\0' || c == '\n') 
		{
			char *re_buf = (char *)malloc(j + 1);
			memcpy(re_buf, buf, j);
			re_buf[j] = '\0';
			*posi = i;
			return re_buf;
		}
		buf[j++] = c;
	}
	return NULL;
}
//设置key-value对
void set_kv(kv *kv_, char *line_data)
{
	char c = '\0';
	int i = 0, j = 0;
	char tmp[1024] = {0};
	//设置key
	while(1)
	{
		c = line_data[i++];
		if(c == ':')
		{
			break;
		}
		tmp[j++] = c;
	}
	
	memcpy((*kv_).key, tmp, j + 1);
	//设置value
	memset(tmp, 0, j + 1);
	j = 0;
	while(1)
	{
		c = line_data[i++];
		if(c == '\0')
		{
			break;
		}
		tmp[j++] = c;
	}
	memcpy((*kv_).value, tmp, j + 1);
}
//从给定套接字构造header结构(返回kv个数)
int build_header_struct(int *c_sock, kv **header)
{
	kv kvs[10] = {0};
	char * line_data;
	int i = 0;
	while(1)
	{
		line_data = message_line(c_sock);
		
		if(line_data == NULL)
		{
			break;
		}else
		{
			set_kv(&kvs[i], line_data);
		}
		i++;
	}
	*header = (kv *)malloc(sizeof(kv)*i);
	memcpy(*header, kvs, sizeof(kv)*i);
	return i;
}
//以给定key从请求header中获取value
char *get_value(char *key, kv * header, int count)
{
	int i = 0;
	for(;i < count; i++)
	{

		if(strcmp(header[i].key, key) == 0)
		{
			return header[i].value;
		}
	}
	return NULL;
}
//从给定相对路径文本获取html
char *get_html(char const *path)
{
	char rel_path[2048] = ".";
	strcat(rel_path, path);
	if(access(rel_path, R_OK) != -1){
		char part_data[1024];
		char html[512000];
		FILE *f = fopen(rel_path, "r+");
		int _break_flag = 0;
		do{
			_break_flag = fread(part_data, 1024, 1, f);
			strcat(html, part_data);
		}while(_break_flag > 0);
		fclose(f);
		int html_len = strlen(html);
		char *rel_html = (char *)malloc(html_len + 1);
		memcpy(rel_html, html, html_len);
		rel_html[html_len] = '\0';
		return rel_html;
 	}
	return NULL;
}

//获取header
char *get_header( char const *content_type, int content_length)
{
	char *resp_header = (char *)malloc(255);
	resp_header[0] = '\0';
	strcat(resp_header, "HTTP/1.1 200 ok\r\n");
	strcat(resp_header, "Server:ujuws\r\n");
	char temp_header_part[64];
	sprintf(temp_header_part, "Content-Type:%s\r\nContent-Length:%d\r\n", content_type, content_length);
	strcat(resp_header, temp_header_part);
	strcat(resp_header, "\r\n");
	strcat(resp_header, "\0");
	return resp_header;
}
//responsehtml文本
void send_html(char *html, int *c_sock)
{
	char *header = get_header("text/html;charset=utf-8", strlen(html));
	char resp_content[51200];
	sprintf(resp_content, "%s%s", header, html);
	send(*c_sock, resp_content, strlen(resp_content), 0);
	free(header);
}
//用cgi程序响应请求
void cgi_handle(char const *url, int *c_sock)
{
	int ind = charAt(url, '?');
	char *args = NULL;//请求参数
	if(ind == -1) ind = strlen(url);
	else args = substr_toend(url, ind + 1);
	char *cgi_executor = substr(url, 0, ind);//请求cgi程序
	char rel_cgi_path[strlen(cgi_executor) + 11];//cgi程序路径
	sprintf(rel_cgi_path, "./cgi-bin/%s", cgi_executor);
	if(access(rel_cgi_path, R_OK) == -1)//不存在返回404页面
	{
		response_404(c_sock);
		return;
	}
	int fd[2];
	int *reader = &fd[0];
	int *writer = &fd[1];
	int result = pipe(fd);
	if(result == -1)
	{
		perror("pipe error!");
		exit(-1);
	}
	pid_t pid = fork();
	if(pid == -1)
	{
		perror("fork error!");
		exit(-1);
	}else if(pid == 0)//cgi脚本执行子进程
	{
		close(*reader);
		dup2(*writer, 1);
		dup2(*writer, 2);
		execl(rel_cgi_path, cgi_executor, args, NULL);
		close(*writer);
	}else//响应进程
	{
		close(*writer);
		char html[51200];
		char buf[1024];
		while(read(*reader, html, 1024) != 0)
		{
			strcat(html, buf);
		}
		send_html(html, c_sock);
		close(*reader);
		free_char(cgi_executor);
		free_char(args);
	}
}
//响应资源文件
void response_res(char const *path, char const *content_type, int *c_sock)
{
	FILE *fp = fopen(path, "r+");
	
	if(fp == NULL)
	{
		response_404(c_sock);
	}else
	{
		fseek(fp, 0, SEEK_END);
		int content_len = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		char *resp_header = get_header(content_type, content_len);
		send(*c_sock, resp_header, strlen(resp_header), 0);
		int read_len = 0;
		char *part_data = (char *)malloc(1024);
		while((read_len = fread(part_data, 1, 1024, fp)) > 0 )
		{
			send(*c_sock, part_data, read_len, 0);
		}
		char d[1024*1024] = {0};
		send(*c_sock, d, 1024*1024, 0);
		free_char(resp_header);
		free_char(part_data);
		fclose(fp);
	}
}
//响应404页面
void response_404(int *c_sock)
{
	char temp[1024];
	temp[0] = '\0';
	strcat(temp, "<!doctype html>\n");
	strcat(temp, "<html>\n");
	strcat(temp, "<head><title>404 not found!</title></head>\n");
	strcat(temp, "<body><div style=\"margin-top:50px;font-size:16px;color:#666;font-family:Consolas;text-align:center;line-height:30px;\">");
	strcat(temp, "404 not found! 你找的资源不存在！<br />ujuws/1.0</div></body>\n");
	strcat(temp, "</html>");
	strcat(temp, "\0");
	char *resp_header = get_header("text/html;charset=utf-8", strlen(temp));
	send(*c_sock, resp_header, strlen(resp_header), 0);
	send(*c_sock, temp, strlen(temp), 0);
	free(resp_header);
}