#include "csapp.h"
//测试uri的解析
int parse_uri(char *uri, char *hostname, char *pathname, char *port);
int main()
{
    
    return 0;
}
int parse_uri(char *uri, char *hostname, char *pathname, char *port)
{
    char *ptr;
    char *tmp;
    char *tmp2;
    /* 处理(忽略)http:// or https:// */  
    if((ptr = strstr(uri,"://")) == NULL)
        return -1;
    else
        ptr = ptr + 3;
    //(:port)
    if((tmp = strchr(ptr,':'))!=NULL)
    {
        *tmp = '\0';
        sscanf(ptr,"%s",hostname);
        if((tmp2 = strchr(ptr,'/'))!=NULL)
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