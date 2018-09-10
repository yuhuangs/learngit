#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <errno.h>

void error(char *msg);
int open_listener_socket();
int read_in(int socket, char *buf, int len);
void bind_to_port(int socket,int port);
int say(int socket,char* s);
void handle_shutdown(int sig);
int catch_signal(int sig,void (*handler)(int));
int listener_d;

int main(int argc,char *argv[])
{
    char *word="Internet Knock Protocol Server\r\nVersion 1.0\r\nKnock!Knock!\r\n";
    if(catch_signal(SIGINT,handle_shutdown)==-1)
    error("Can't set the interrupt handle");
    listener_d=open_listener_socket();
    bind_to_port(listener_d,30000);
    if(listen(listener_d,10)==-1) error("Can't listen");
    struct sockaddr_storage client_addr;
    unsigned int adress_size=sizeof(client_addr);
    puts("Waiting for connection");
    char buf[255];
    while(1){
        int connect_d=accept(listener_d,(struct sockaddr *)&client_addr,&adress_size);
        if(connect_d==-1) error("Can't open secondary socket");
        if(!fork()){
            close(listener_d);
            if(say(connect_d,word)!=-1){
                read_in(connect_d,buf,sizeof(buf));
                if(strncasecmp("Who's there?",buf,12))
                    say(connect_d,"You should say 'who's there?'!");
                else{
                    if(say(connect_d,"Oscar\r\n")!=-1){
                        read_in(connect_d,buf,sizeof(buf));
                    if(strncasecmp("Oscar who?",buf,10))
                        say(connect_d,"You should say 'Oscar who?'!");
                    else
                        say(connect_d,"Oscar silly question,you get a silly answer\r\n");
                }
            }
        }
        close(connect_d);
        exit(0);
        }
    }
    return 0;
}
void error(char *msg)
{
    fprintf(stderr,"%s: %s\n",msg,strerror(errno));
    exit(1);//停止运行程序
}
int read_in(int socket, char *buf, int len)//读取\n前的所有字符
{
    char *s=buf;
    int slen=len;
    int c=recv(socket,s,slen,0);
    while((c>0)&&(s[c-1]!='\n')){
        s+=c;
        slen-=c;
        c=recv(socket,s,slen,0);
    }
    if(c<0) return c;
    else if(c==0) buf[0]='\0';
    else s[c-1]='\0';
    return len -slen;
}

int open_listener_socket()
{
    int s=socket(PF_INET,SOCK_STREAM,0);
    if(s==-1){
        error("Can't open socket");
    }
    return s;
}

void bind_to_port(int socket,int port)
{
    struct sockaddr_in name;
    name.sin_family=PF_INET;
    name.sin_port=htons(30000);
    name.sin_addr.s_addr=htonl(INADDR_ANY);
    int reuse=1;
    if(setsockopt(socket,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(int))==-1)
        perror("无法设置套接字的“重新使用端口“选项");
    if(bind(socket,(struct sockaddr *) &name,sizeof(name))==-1){/*绑定端口*/
            error("无法绑定窗口");
    }
}

int say(int socket,char* s)
{
    int result=send(socket,s,strlen(s),0);
    if(result==-1)
        error("和客户端通信时发生了错误");
    return result;
}

void handle_shutdown(int sig)
{
    if(listener_d)
        close(listener_d);
    
    fprintf(stderr,"Bye!\n");
    exit(0);
}

int catch_signal(int sig,void (*handler)(int))
{
    struct sigaction action;
    action.sa_handler=handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags=0;
    return sigaction(sig,&action,NULL);
}
