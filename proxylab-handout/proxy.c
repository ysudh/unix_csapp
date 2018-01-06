#include <stdio.h>
#include "csapp.h"
/*
 *目标:设计多线程带缓存的代理服务器
 *实验要求:
 *1.Sequential Proxy: 接收客户端发送的 HTTP 请求，解析之后向目标服务器转发，获得响应之后再转发回客户端
 *2.Concurrent Proxy: 在第一步的基础上，支持多线程
 *3.Cache Web Objects: 使用 LRU 缓存单独的对象，而不是整个页面
 *注意:
HTTP/1.1 里默认将 connection 定义为 keep-alive，也就是一条 TCP 连接可以处理多个请求，不用每次都要重新建立 TCP 连接。
我们的简易 proxy 还无法提供这样的功能，所以在读 client 发过来的 header 的时候，如果是 Connection: keep-alive
或者 Proxy-Connection: keep-alive，我们都要把它们换成 Connection: close 或 Proxy-Connection: close
 */
 
 //Sequential Proxy
/*1.read the entirety of the request from the client and parse the request
 *2.determine whether the client has sent a valid HTTP request:
   (HTTP合法 判断URL地址的正则 表达式为:http(s)?://([\w-]+\.)+[\w-]+(/[\w- ./?%&=]*)?)
 *  if so, connection to the appropriate web server then request
 *3.read the server’s response and forward it to the client
 */
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
void read_requesthdrs(rio_t * rp);
int connect_server(char *hostname, char *port, char *pathname);
int read_body_repeat(rio_t *rio, char *body, int len, int fd);
int read_repeat(rio_t *rio, int fd,int serverfd);
int parse_uri(char *uri, char *hostname, char *pathname, char *port);
void doit(int fd);

int main(int argc, char **argv)
{
   // printf("%s", user_agent_hdr);
    int listenfd,connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;//一种新的通用地址结构
    
    if(argc != 2)
    {
        fprintf(stderr,"usage:%s <port>\n",argv[0]);
        exit(1);
    }
    /*socket() bind() listen()封装*/
    listenfd = Open_listenfd(argv[1]);
    while(1)
    {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        
        doit(connfd);     /*处理一个http事物 转发到server*/ 

        //gethostbyaddr(); 线程不安全的 mutex
        Close(connfd);
    }
    return 0;
}
void doit(int fd)
{
    char buf[MAXLINE],method[MAXLINE],uri[MAXLINE],version[MAXLINE];
    char filename[MAXLINE],hostname[MAXLINE];
    char port[MAXLINE];
    rio_t rio;
    int serverfd;
    int body_size;
    Rio_readinitb(&rio, fd);
    //method URI version(request line)
    //GET http://www.cmu.edu/hub/index.html HTTP/1.1
    if(!Rio_readlineb(&rio,buf,MAXLINE))
        return;
    printf("req line = %s\n",buf);
    if(sscanf(buf,"%s %s %s",method, uri, version)<3)
    {
        fprintf(stderr, "Bad Request line!\n");
        clienterror(fd, method, "400", "Bad Request","request line error");
        return ;
    }
    if(strcasecmp(method,"GET")!=0){
        fprintf(stderr,"Not implement:%s\n",method);
        clienterror(fd, method, "501", "Not Implemented",
                    "Proxy does not implement this method");
        return ;
    }
    //header-name:header-data(注意host)
    /* 根据hostname,Proxy 可以判断它是否可以在本地缓存中拥有一个被请求内容的副本*/
    read_requesthdrs(&rio);
    /* Parse URI from GET request */
    if(parse_uri(uri, hostname, filename, port)<0)
    {
        clienterror(fd, method, "400", "Bad Request","uri error");
        return ;
    }
    printf("%s + %s + %s\n",hostname, port, filename);
    /* connect to server and return socket fd */
    if((serverfd = connect_server(hostname, port, filename))<0)
        return ;
    body_size = read_repeat(&rio, fd, serverfd);
    printf("the total size of body :%d\n",body_size);
    /**/
    
}
/* 按行读header 打印但是未处理*/
void read_requesthdrs(rio_t * rp)
{
    char buf[MAXLINE];
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s\n",buf);
    while(strcmp(buf, "\r\n"))
    {
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s\n",buf);
    }
    return ;
}
/*uri:http://www.cmu.edu(:80)/hub/index.html/?xxx&yyyy(忽略参数)
 *filename:/hub/index.html(/index.html)
 *hostname:www.cmu.edu
 *port:80(default)
 *strchr strstr strpbrk等函数应用
 */
int parse_uri(char *uri, char *hostname, char *pathname, char *port)
{
    char *ptr;
    char *tmp;
    char *tmp2;
    /* 处理(忽略)http:// or https:// */  
    if((ptr = strstr(uri,"//")) == NULL)
        ptr = uri;
    else
        ptr = ptr + 2;
    //包含端口(:port)
    if((tmp = strchr(ptr,':'))!=NULL)
    {
        *tmp = '\0';
        sscanf(ptr,"%s",hostname);
        if((tmp2 = strchr(tmp+1,'/'))!=NULL)
        {
            sscanf(tmp2,"%s",pathname);
            *tmp2 = '\0';
            sscanf(tmp+1,"%s",port); 
        }
        else
            sscanf(tmp+1,"%s",port);
    }  
    else
    {
        if((tmp = strchr(ptr,'/'))!=NULL)
        {
            sscanf(tmp,"%s",pathname);
            tmp = '\0';
            sscanf(ptr,"%s",hostname);
        }
        else
            sscanf(ptr,"%s",hostname);
        *port = 80;
    }
    if(strlen(pathname)<=1)
        sscanf("/index.html","%s",pathname);    
    return 0;
}
/* connect to server and return socket fd */
int connect_server(char *hostname, char *port, char *pathname)
{
    /* You won't lose style points for including this long line in your code */
    static  char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
    static  char *accept_str = "Accept:text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nAccept-Encoding:gzip, deflate\r\n";
    static  char *connection = "Connection: close\r\nProxy-Connection: close\r\n";
    char buf[MAXLINE];
    int proxy_clientfd;
    /* connect to server */
    if((proxy_clientfd = Open_clientfd(hostname, port))<0)
        return proxy_clientfd ; //failed
    
    /* write request to server */
    sprintf(buf,"GET %s HTTP/1.0\r\n",pathname);
    Rio_writen(proxy_clientfd, buf, strlen(buf));
    sprintf(buf,"host:%s\r\n",hostname);
    Rio_writen(proxy_clientfd, buf, strlen(buf));
    Rio_writen(proxy_clientfd, user_agent_hdr, strlen(user_agent_hdr));
    Rio_writen(proxy_clientfd, accept_str, strlen(accept_str));
    Rio_writen(proxy_clientfd, connection, strlen(connection));
    Rio_writen(proxy_clientfd, "\r\n", strlen("\r\n"));
    printf("the server is done! \n");
    return proxy_clientfd;   
}
/* read response from server and repeat to client */
int read_repeat(rio_t *rio, int fd, int serverfd)
{
     int body_size;
     char body[MAXLINE];
     char buf[MAXLINE];
     int found = 0;
     int clength;
     Rio_readinitb(rio, serverfd);
     /*rio_readnb(从缓冲区读n bytes)和rio_readlineb(从缓冲区读一行)*/
     /*read header*/
     do{
         Rio_readlineb(rio, buf, MAXLINE);
         if(strstr(buf, "Content-length:"))
         {
             sscanf(buf, "Content-length:%d\r\n", &clength);
             found = 1;
         }
         Rio_writen(fd, buf, MAXLINE);
     }while(strcmp(buf, "\r\n"));
     /*read boby*/
     if(found)
     {
        printf("begin read boby!\n");
        body_size = read_body_repeat(rio, body, clength, fd);
     }
     else
     {
        printf("Not find Content-length!\n");
        int size;
        while((size = Rio_readnb(rio, buf, MAXLINE))>0)
        {
            Rio_writen(fd, buf, size);
            body_size += size;
        }
     }
     Close(serverfd); //
     return body_size;
}
int read_body_repeat(rio_t *rio, char *body, int len, int fd)
{
    int total_size;
    char buf[MAXLINE];
    int nread;
    int size;
    while(len>0)
    {
        nread = (len>MAXLINE) ? MAXLINE:len;
        if((size = Rio_readnb(rio, buf, nread)) != nread)
        {
            fprintf(stderr,"read body server error!\n");
            exit(0); 
        }
        Rio_writen(fd, buf, nread);
        len = len - nread;
        total_size += size; 
    }
    return total_size;
}
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
    char buf[MAXLINE],body[MAXLINE];
    /*Build the HTTP response body*/
    sprintf(body,"<html><title>Proxy Error</title>");
    sprintf(body,"%s<body bgcolor=""ffffff"">\r\n",body);
    sprintf(body,"%s%s:%s\r\n",body, errnum, shortmsg);
    sprintf(body,"%s<p>%s:%s\r\n",body,longmsg,cause);
    sprintf(body,"%s<hr><em>The Proxy Server</em>\r\n",body);
    /* Print the HTTP response */
    //response line <version status-code status-message>
    sprintf(buf, "HTTP/1.0 %s %s\r\n",errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    //response header 
    sprintf(buf,"Content-type:text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf,"Content-length:%d\r\n\r\n",(int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    //response body
    Rio_writen(fd, body, strlen(body));
}
void format_log_entry(char )
{

}