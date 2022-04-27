#ifndef PING_H
#define PING_H
#include <winsock.h>
#include <stdio.h>



class IPHeader
{
    public:
        //u_char 占1个字节
        //u_short 占两个字节
        //u_char version;        // 版本
        //u_char head_len;       // 首部长度
        u_char ver_headlen;      // 版本+首部长度
        u_char service;          // 服务类型
        u_short total_len;       // 总长度
        u_short id;              // 标识符
        u_short flag;            // 标记+片偏移
        u_char ttl;              // 存活时间
        u_char protocol;         // 协议
        u_short check_sum;       // 首部校验和
        u_int src_IP;            // 源IP地址
        u_int dst_IP;            // 目的IP地址
};

class ICMPHeader
{
    public:
        u_char type;             // 类型
        u_char code;             // 代码
        u_short check_sum;       // 校验和
        u_short id;              // 标示符 标识本进程
        u_short seq;             // 序列号
};




class Ping
{
    public:
        char in[100];                                           //从输入框读入
        sockaddr_in dst_IP;                                     //目标IP
        struct hostent *host;
        IPHeader iPHeader;
        ICMPHeader iCMPHeader;
        SOCKET sock;
        char message[5000];
        bool received;
        bool getIP();                                           // 得到ping的IP地址
        bool send();                                            // 发送ICMP报文请求
        bool receive();                                         // 接收ICMP报文,解析并回显
        u_short check_sum(u_short* buffer, int len);            // 计算校验和

};




//ICMP时间戳请求报文
struct EchoRequest
{
    ICMPHeader icmp_header;                     //ICMP头部
    unsigned long long time;                    //记录ping时间
};

#pragma pack(push) //保存对齐状态
#pragma pack(1)//设定为1字节对齐
struct EchoResponse
{
    IPHeader ip_header;
    EchoRequest echo_request;
};
#pragma pack(pop)//恢复对齐状态

#endif // PING_H
