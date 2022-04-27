#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <Windows.h>
#include <QMessageBox>
#include "Ping.h"
#include <thread>
#include <string.h>
#include <math.h>

char message[50000];
char IP[30];
int numSend=0;
int numReceive=0;
int times=4;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    qRegisterMetaType<Ping>("Ping");
    ui->setupUi(this);

}

MainWindow::~MainWindow()
{
    delete ui;
    WSACleanup();
}
void MainWindow::on_PingButton1_clicked()
{
    ui->PingButton1->setDisabled(true);
    ui->PingButton2->setDisabled(true);
    numSend=0;
    numReceive=0;
    times=4;
    strcat(message,"\n");
    char temp[100];//用来存储ping地址
    strcpy(temp,ui->Addr->text().toLatin1().data()); //ping 地址
    char tmp[10];//用来存储ping包数
    strcpy(tmp,ui->Addr_2->text().toLatin1().data()); //ping 包数
    sscanf(tmp,"%d",&times);
    if(times > 0)
    {
        Ping pingTemp;
        strcpy(pingTemp.in, temp);
        if(pingTemp.getIP())//获取IP
        {
            strcat(message,"正在 Ping ");
            strcat(message,pingTemp.host->h_name);
            strcat(message,"[");
            strcat(message,inet_ntoa(*(struct in_addr*)pingTemp.host->h_addr_list[0]));
            strcat(message,"] 具有 32 字节的数据:\n");
            ui->Result->setText(message);
            for(int i=0; i<times; i++)//分四个线程去ping
            {
                PingThread * ping = new PingThread(temp);//创建一个新的线程
                connect(ping,SIGNAL(isDone(Ping)),this,SLOT(Ping1Result(Ping)));//qt信号槽机制
                ping->start();
            }
        }
        else
        {
            strcat(message,"Ping 请求找不到主机 ");
            strcat(message,temp);
            strcat(message,"。请检查该名称，然后重试。\n");
            ui->Result->setText(message);
            PingThread * ping = new PingThread(temp);
            connect(ping,SIGNAL(isDone(Ping)),this,SLOT(Ping1Result(Ping)));
            ping->start();
        }
    }
    else
    {
        PingThread * ping = new PingThread(temp);
        connect(ping,SIGNAL(isDone(Ping)),this,SLOT(Ping1Result(Ping)));
        ping->start();
    }
}

void PingThread::run()
{
    Ping ping;
    strcpy(ping.in, addr);
    ping.sock=socket(AF_INET,SOCK_RAW,IPPROTO_ICMP);
    if(times >0)
    {
        if(ping.send())
        {
            numSend++;
            printf("%d\n",numSend);
        }
        if(ping.received)
        {
            numReceive++;
        }
        strcat(message,ping.message);
    }
    emit isDone(ping);
}
PingThread::PingThread(char _addr[100])
{
    strcpy(addr, _addr);
}

void MainWindow::Ping1Result(Ping ping)
{
    ui->PingButton1->setDisabled(false);
    ui->PingButton2->setDisabled(false);
    if(times >1 && numSend == times )
    {
        strcat(message,inet_ntoa(*(struct in_addr*)ping.host->h_addr_list[0]));
        strcat(message," 的 Ping 统计信息:\n    数据包: 已发送 = ");
        char temp[10];
        sprintf(temp,"%d",numSend);
        strcat(message,temp);
        strcat(message,"，已接收 = ");
        temp[0]='\0';
        sprintf(temp,"%d",numReceive);
        strcat(message,temp);
        strcat(message,"，丢失 = ");
        temp[0]='\0';
        sprintf(temp,"%d",numSend-numReceive);
        strcat(message,temp);
        strcat(message," (");
        temp[0]='\0';
        sprintf(temp,"%d",100*(numSend-numReceive)/numSend);
        strcat(message,temp);
        strcat(message,"% 丢失)。\n");
        strcat(message,"\n");
        numSend=0;
        numReceive=0;
    }
    ui->Result->setText(message);
    ui->Result->moveCursor(QTextCursor::End,QTextCursor::MoveAnchor); //移动光标到末尾
}


char *IPToString(u_int IP_int) //将IP地址转变为字符串->点分十进制
{
    IP[0]='\0';
    char temp[5];
    sprintf(temp,"%u",(IP_int >> 24) & 0x000000FF);
    strcat(IP,temp);
    strcat(IP,".");
    temp[0]='\0';
    sprintf(temp,"%u",(IP_int >> 16) & 0x000000FF);
    strcat(IP,temp);
    strcat(IP,".");
    temp[0]='\0';
    sprintf(temp,"%u",(IP_int >>8) & 0x000000FF);
    strcat(IP,temp);
    strcat(IP,".");
    temp[0]='\0';
    sprintf(temp,"%u",IP_int & 0x000000FF);
    strcat(IP,temp);
    temp[0]='\0';
    return IP;
}


void MainWindow::on_PingButton2_clicked()
{
    ui->PingButton1->setDisabled(true);
    ui->PingButton2->setDisabled(true);
    strcat(message,"\n");
    char *temp=(char *)malloc(100);
    strcpy(temp,ui->Addr_3->text().toLatin1().data());
    int i=0;
    int flag=0;
    while(temp[i]!='\0')
    {
        if(temp[i] == '/')//判断子网掩码部分是否存在
        {
            flag=1;
            break;
        }
        i++;
    }
    if(flag == 0)
    {
        strcat(message,"子网输入格式错误!\n(示例：112.80.248.75/28)\n");
        ui->Result->setText(message);
        ui->Result->moveCursor(QTextCursor::End,QTextCursor::MoveAnchor); //移动光标到末尾
    }
    else
    {
        int j=0;
        temp[i]='\0';
        i++;
        char t[3];
        while(temp[i]!='\0')//子网掩码
        {
            t[j]=temp[i];
            j++;
            i++;
        }
        t[j]='\0';
        //temp现在是一个Ip地址格式
        //t现在是一个数字
        u_int range = 0;
        sscanf(t,"%u",&range);
        range= 32-range;//主机号位数
        //子网掩码
        u_int subnet_mask=pow(2,32)-pow(2,range);//子网掩码
        u_int ip[4];
        u_int IP=0;
        i=0;
        j=0;
        int k=0;
        while(temp[j]!='\0')
        {
            if(temp[j] == '.')
            {
                temp[j] = '\0';
                sscanf(temp,"%u",&ip[k]);
                i=j+1;
                temp=&temp[i];
                i=0;
                j=0;
                k++;
                continue;
            }
            j++;
        }
        temp[j] = '\0';
        sscanf(temp,"%u",&ip[k]);
        i=j+1;
        k++;
        IP = ip[0]*256*256*256+ip[1]*256*256+ip[2]*256+ip[3];
        IP= subnet_mask & IP;//子网掩码相与
        u_int left = IP+1;
        int n=0;
        n=pow(2,range)-1;
        u_int right = (IP | n) -1;
        strcpy(message, "子网内主机的IP范围为：");
        strcat(message,IPToString(left));
        strcat(message, "--");
        strcat(message,IPToString(right));
        strcat(message,"\n");
        while(left <= right)
        {
            char _addr[30];
            strcpy(_addr,IPToString(left));
            numSend=0;
            numReceive=0;
            times=1;

            if(times > 0)
            {
                Ping pingTemp;
                strcpy(pingTemp.in, _addr);
                if(pingTemp.getIP())
                {
                    /*
                    strcat(message,"正在 Ping ");
                    strcat(message,pingTemp.host->h_name);
                    strcat(message,"[");
                    strcat(message,inet_ntoa(*(struct in_addr*)pingTemp.host->h_addr_list[0]));
                    strcat(message,"] 具有 32 字节的数据:\n");
                    */
                    ui->Result->setText(message);
                    PingThread * ping = new PingThread(_addr);
                    connect(ping,SIGNAL(isDone(Ping)),this,SLOT(Ping1Result(Ping)));
                    ping->start();
                }
                else
                {
                    strcat(message,"Ping 请求找不到主机 ");
                    strcat(message,_addr);
                    strcat(message,"。请检查该名称，然后重试。\n");
                    ui->Result->setText(message);
                }
            }
            left++;
        }
    }
}

