#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define ERR_EXIT(m) \
    do \
    { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while(0)

int main()
{
    //创建服务器的套接字.套接字是一个文件描述符
    //使用什么协议(这里是TCP/IP协议)
    int sockfd;
    if ((sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0)
      ERR_EXIT("socket");
    
    //create an ip addr & bind the socket to it
    struct sockaddr_in addr; // 使用IPV4的协议结构体,但是绑定给套接字的时候,地址必须是通用协议地址,所以要类型转换
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5188); // 绑定端口号的时候使用网络字节序
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // 老师推荐使用,说这个会绑定当前IP
    //addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 也可以使用这个.点分IP=>网络字节序的32位无符号证书

    if(bind(sockfd,(struct sockaddr*)&addr,sizeof(addr))<0) // "专门协议地址=>通用协议地址"使用小括号类型转换就好了
      ERR_EXIT("bind");

    //listen
    if(listen(sockfd,SOMAXCONN)<0) // SOMAXCONN是服务器套接字允许建立的最大队列,包括未连接+已连接的队列
      ERR_EXIT("listen");
    
    //accept & get the connection socket
    struct sockaddr peer_addr;
    socklen_t peer_len = sizeof(peer_addr); // 使用accept获得对方的套接字等信息时,对方套接字的长度一定要初始化
    int conn_sockfd;
    if ((conn_sockfd = accept(sockfd,(struct sockaddr*)&peer_addr,&peer_len))<0)
      ERR_EXIT("accept");


    //connect 
    char recv_buf[1024]; // 接收到的数据要输出到屏幕上,所以需要设置字符数组作为缓冲区,而这个也是1024字节
    while(1)
    {
        memset(recv_buf,0,sizeof(recv_buf));
        int ret = read(conn_sockfd,&recv_buf,sizeof(recv_buf)); // 接收到的数据是通过connection套接字来的.直接将这个套接字当成文件(因为是文件描述符)来处理,所以read/write就行了
        fputs(recv_buf,stdout);
        write(conn_sockfd,recv_buf,ret);
    }


    // close all the sockets
    close(conn_sockfd);
    close(sockfd);
    return 0;



    


    
    


}