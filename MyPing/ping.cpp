#include "ping.h"
#include <Windows.h>
#include <string.h>

bool Ping::getIP() //获取IP地址
{
    host = gethostbyname(in);
    if (host == NULL)
    {
        return false;
    }
    dst_IP.sin_family = AF_INET; //IPv4
    dst_IP.sin_addr.S_un.S_addr = *(u_long*)host->h_addr;
    puts(host->h_name);
    puts(inet_ntoa(*(struct in_addr*)host->h_addr_list[0]));  //inet_ntoa()将网络地址转换成“.”点隔的字符串格式
    return true;
}

bool Ping::send() //发送数据包
{
    if(getIP()==false) //获取IP失败
    {
        return false;
    }
    static int id = 1;
    static int seq = 1;
    EchoRequest req;
    req.time = GetTickCount(); //从0开始计时，返回自设备启动后的毫秒数
    req.icmp_header.type = 8;  //ICMP_ECHO
    req.icmp_header.code = 0;  //编码
    id = ::GetCurrentProcessId();//获取当前的进程ID
    req.icmp_header.id = id;//id++;
    req.icmp_header.seq = seq++;//序号加一
    req.icmp_header.check_sum = check_sum((u_short*)&req, sizeof(EchoRequest));//校验和字段

    int re = sendto(sock, (char*)&req, sizeof(req), 0, (sockaddr*)&dst_IP, sizeof(dst_IP));//将数据由指定的socket 传给目标主机
    // 成功则返回实际传送出去的字符数, 失败返回 -1,
    // message 为此次ping的一些信息，输出到
    if(re == SOCKET_ERROR)  //SOCKET_ERROR = -1
    {
        strcat(message,"发送错误，错误码:");
        char temp[10];
        sprintf(temp,"%d",WSAGetLastError()); //WSAGetLastError()返回该线程上一次Sockets API函数调用时的错误代码,即sendto()函数的错误调用
        strcat(message,temp);
        strcat(message,"\n");
        return false; //发送失败
    }
    if(receive())
    {
        received=true;//接收成功
    }
    else
    {
        received=false;//接收失败
    }
    return true;    //发送成功
}

u_short Ping::check_sum(u_short* buf, int len) //校验和计算 生成校验和字段
{
    unsigned int sum=0;
    unsigned short *cbuf;

    cbuf=(unsigned short *)buf;

    while(len>1)
    {
        sum+=*cbuf++;
        len-=2;
    }
    if(len)
    {
        sum+=*(unsigned char *)cbuf;
    }
    sum=(sum>>16)+(sum & 0xffff);
    sum+=(sum>>16);
    return ~sum;
}

bool Ping::receive() //接收并分析返回的数据包
{
    int timeout = 1000;//超时时间，设置为1000ms
    if(setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(timeout)) == SOCKET_ERROR) //设置套接口
    // WINSOCK_API_LINKAGE int WSAAPI setsockopt(SOCKET s,int level,int optname,const char *optval,int optlen);
    {
        strcat(message,"接收设置错误，错误码:");//设置套接口返回错误
        char temp[10];
        sprintf(temp,"%d",WSAGetLastError());
        strcat(message,temp);
        strcat(message,"\n");
        return false; //接收失败
        //message 用来展示终端信息
    }
    char temp[100];
    strcat(message,"来自 ");

    strcat(message,inet_ntoa(*(struct in_addr*)host->h_addr_list[0]));//地址 //inet_ntoa()将网络地址转换成“.”点隔的字符串格式

    strcat(message," 的回复: ");
    EchoResponse* res=new EchoResponse;
    int size = sizeof(sockaddr); //套接字大小
    int re = recvfrom(sock, (char*)res, sizeof(EchoResponse), 0, (sockaddr*)&dst_IP, &size);//接收到的返回的套接字
   //  WINSOCK_API_LINKAGE int WSAAPI recvfrom(SOCKET s,char *buf,int len,int flags,struct sockaddr *from,int *fromlen);
    if (re == SOCKET_ERROR)//出错
    {
        int code = WSAGetLastError();
        if(code==10060)
        {
            strcat(message,"请求超时。\n");
        }
        return false;
    }
    unsigned long long time = GetTickCount() - res->echo_request.time;//两个相减即为TTL时间
    int type = res->echo_request.icmp_header.type;
    int code = res->echo_request.icmp_header.code;
    char TTL[10]={'\0'};
    sprintf(TTL,"%d",(int)res->ip_header.ttl);
    delete res;
    if(type==0&&code==0)//输出到message信息中
    {
        strcat(message,"字节=32 时间=");
        sprintf(temp,"%I64u",time);
        strcat(message,temp);
        strcat(message,"ms TTL=");
        strcat(message,TTL);
        strcat(message,"\n");
        return true;
    }
    else
    {
        strcat(message,"请求超时");
        strcat(message,"\n");
        return false;
    }

}
