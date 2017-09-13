httpd: httpd.c tools.c
	gcc -o httpd httpd.c tools.c -lpthread
