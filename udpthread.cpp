#include "udpthread.h"
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QMutex>

//线程代码
UDP_Thread::UDP_Thread(QObject *parent1)
{
    udpThread = new QThread();
    connect(udpThread,&QThread::started,this,&UDP_Thread::onUdpThreadCreate);       //线程开始，创建tcpServer对象
    connect(udpThread,&QThread::finished,this,&QObject::deleteLater);                    //线程结束，自动删除对象
    this->moveToThread(udpThread);

}

UDP_Thread::~UDP_Thread()
{
    disconnect(udpThread,&QThread::started,this,&UDP_Thread::onUdpThreadCreate);       //线程开始，创建tcpServer对象
    disconnect(udpThread,&QThread::finished,this,&QObject::deleteLater);                    //线程结束，自动删除对象
    if(udpThread)
    {
        if(udpThread->isRunning())
        {
            udpThread->quit();
            udpThread->wait();
        }
        delete udpThread;
        udpThread = NULL;
    }
    if(socket)
    {
        delete socket;
        socket = NULL;
    }
}

void UDP_Thread::stop()
{
    thread1_On = false;
}

void UDP_Thread::go_on()
{
    thread1_On = true;
}

void UDP_Thread::udpInit(QString _ip1, int _port1 ,QString _ip2, int _port2)
{
    U_ip = _ip1;
    U_port = _port1;
    D_ip = _ip2;
    D_port = _port2;
}// 初始化相关常量，顺序为上位机IP、上位机port、FPGA的IP和FPGA的port

void UDP_Thread::udpThreadStart()
{
    udpThread->start();
}

void UDP_Thread::udpBind(QString udpIp, int udpPort)
{
    qDebug() << socket->state();
    bool isok =  socket->bind(QHostAddress(udpIp),udpPort);
    if(isok)
        emit UDPMsg(QDateTime::currentDateTime().toString("hh:mm:ss")+"监听成功");
    else
        emit UDPMsg(QDateTime::currentDateTime().toString("hh:mm:ss")+"监听失败");
}// 建立监听，为读取数据做准备

void UDP_Thread::Array2file(QString path, QByteArray Data)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)){
        return;
    }
    emit UDPMsg(QDateTime::currentDateTime().toString("hh:mm:ss")+"新建文本");
    QTextStream out(&file);
    for(int i=0;i<Data.size();i++){
        out << Data.at(i);
    }
    file.close();
}

void UDP_Thread::onUdpThreadCreate()
{
    socket = new QUdpSocket();
    connect(socket,&QUdpSocket::readyRead,this,&UDP_Thread::onRecvUdpData);

}

//============ 实际功能代码 ================

void UDP_Thread::statementChange(){
    if (thread1_On==0){
        udpBind(U_ip,U_port);
        if(Mode_rec == 0){
            // 向下位机发送“1”
            QByteArray byte;
            byte.append('1');
            socket->writeDatagram(byte, QHostAddress(D_ip), D_port);
            emit UDPMsg(QDateTime::currentDateTime().toString("hh:mm:ss")+"向下位机发送‘1’");
        }
        thread1_On = true;

    }
    else{
        if(Mode_rec == 0){
            // 向下位机发送“0”
            QByteArray byte;
            byte.append('0');
            socket->writeDatagram(byte, QHostAddress(D_ip), D_port);
            emit UDPMsg(QDateTime::currentDateTime().toString("hh:mm:ss")+"向下位机发送‘0’");
        }
        thread1_On = false;
        socket->abort();
    }
}

void UDP_Thread::onRecvUdpData()
{
    if(thread1_On)
    {
        // 调试模式，会把收到的包数据显示在屏幕，且会把内容存储在txt中
        if(Mode_display == 0){
            QByteArray buff_temp;//定义缓冲区
            buff_temp.resize(socket->pendingDatagramSize());
            socket->readDatagram(buff_temp.data(),buff_temp.size());// 读取socket数据，读完之后就没了，而且必须没了才能接着收
            Array2file("D:/text.txt",buff_temp);
            qDebug()<< QDateTime::currentDateTime().toString("hh:mm:ss")<<"写入txt";
        }
        // 显示模式，能够显示连续的视频
        else if(Mode_display == 1){
            int Len;// 包长度标识
            Len = static_cast<int>(socket->pendingDatagramSize());
            len_count += Len;// 增加流量统计
            QByteArray buff_temp;// 定义缓冲区
            buff_temp.resize(socket->pendingDatagramSize());
            socket->readDatagram(buff_temp.data(),buff_temp.size());// 读取缓冲区数据
            // 8位帧头模式
            if(Mode_flag == 0){
                if(Len == W*2){
                    // 当发包为普通包时，直接将缓存存进总数组
                    bytes_Data.append(buff_temp);
                    p_num++;// 当前帧抓包数自增
                }
                else if(Len == W*2+8){
                    // 当发包为下一帧的第一包，那么就先处理上一帧的数据：将QbyteArray转uchar顺便把帧头剥掉，，再将uchar转QImage，最后向上位机发信号；再清空缓存数组，把新包内容存进来
                    if(p_num >= H){
                        uchar *p = reinterpret_cast<uchar *>(bytes_Data.data());// 用指针做QbyteArray转uchar
                        p=p+8;// 把帧头剥掉
                        uchar RGB_Buff[W*H*2];// 定义uchar缓存数组
                        for(int i=0; i<W*H*2; i+=2)
                        {
                            RGB_Buff[i]=*(p+1);
                            RGB_Buff[i+1]=*p;
                            if(Mode_color == 1){
                                //颜色转换算法
                            }
                            p += 2;
                        }// 按照RGB565把源数据存进新数组
                        emit img_OK(RGB_Buff,W,H);// 向上位机发信号
                        bytes_Data.clear();
                        bytes_Data.append(buff_temp);
                        p_num = 0;// 帧抓包数清0
                    }// 如果抓包数不够或出现帧头紊乱，就直接舍弃该帧数据

                    emit rec_count(len_count);// 不管是否抓包成功都将当前流量统计发给主线程
                    len_count = 0;// 清0流量统计
                    p_num++;// 当前帧抓包数自增
                }
            }
            // 2位帧头模式
            else if(Mode_flag == 1){
                int Len;// 包长度标识
                Len = static_cast<int>(socket->pendingDatagramSize());
                QByteArray buff_temp;//定义缓冲区
                buff_temp.resize(socket->pendingDatagramSize());
                socket->readDatagram(buff_temp.data(),buff_temp.size());
                emit rec_count(Len);// 不管是否抓包成功都将当前流量统计发给主线程
                if(Len == W*2+2){
                    if(buff_temp[0] == 1){
                        // 误差情况1：包不足，重新捉到首行->包数清空、包缓存清空
                        p_num = 0;
                        bytes_Data.clear();
                        // 首帧&行号越变情况：正常流程中不会读跃变的包
                        bytes_Data.append(buff_temp);
                        // 帧头标志位置为有效，否则不进入正常流程
                        frame_flag_2 = true;
                    }
                    if(frame_flag_2){ // 正常流程代码：只要帧头依次增加就将数据缓存，增加行数累计；
                        if(buff_temp[0]-last_num == 1 ){
                            // 当两帧之间无序号错误，则把数据存入数组，并增加有效行数累计
                            bytes_Data.append(buff_temp.mid(1,buff_temp.size()));// 剥去2位行头存储数据
                            p_num++;// 增加有效行数累计
                        }
                        last_num = buff_temp[0];// 无论是不是有效行数，只要包长没错都更新行数
                        if(p_num <= H){
                            uchar *p = reinterpret_cast<uchar *>(bytes_Data.data());// 用指针做QbyteArray转uchar
                            uchar RGB_Buff[W*H*2];// 定义uchar缓存数组
                            for(int i=0; i<bytes_Data.size(); i+=2)
                            {
                                RGB_Buff[i]=*(p+1);
                                RGB_Buff[i+1]=*p;
                                p += 2;
                            }// 按照RGB565把源数据存进新数组
                            emit img_OK(RGB_Buff,W,H);// 向上位机发信号

                            frame_flag_2 = false;// 帧头标志置为无效，所有功能停摆

                        }// 如果抓包数不够或出现帧头紊乱，就直接舍弃该帧数据
                    }

                }
            }
        }
    }
}



void UDP_Thread::new_IP(QString Uip_changed, QString Dip_changed){
    U_ip = Uip_changed;
    D_ip = Dip_changed;
    QString str = QDateTime::currentDateTime().toString("hh:mm:ss") + "IP发生改变";
    emit UDPMsg(str);
}

void UDP_Thread::new_port(quint16 Uport_changed, quint16 Dport_changed){
    U_port = Uport_changed;
    D_port = Dport_changed;
    QString str = QDateTime::currentDateTime().toString("hh:mm:ss") + "port发生改变";
    emit UDPMsg(str);
}

void UDP_Thread::new_w(int w_changed){
    W = w_changed;
    QString str = QDateTime::currentDateTime().toString("hh:mm:ss");
    str.append("W 改变：");
    str.append(std::to_string(W));
    emit UDPMsg(str);
}

void UDP_Thread::new_h(int h_changed){
    H = h_changed;
    QString str = QDateTime::currentDateTime().toString("hh:mm:ss") + "H 改变：";
    str.append(std::to_string(H));
    emit UDPMsg(str);
}

void UDP_Thread::new_vedionum(int num){
    Mode_display = num;
}

void UDP_Thread::new_FPGAmode(int index){
    Mode_rec = index;
    QString str = QDateTime::currentDateTime().toString("hh:mm:ss") + "收包模式改变：";
    str.append(std::to_string(Mode_rec));
    emit UDPMsg(str);
}

void UDP_Thread::new_flagmode(int index)
{
    Mode_flag = index;
    QString str = QDateTime::currentDateTime().toString("hh:mm:ss") + "帧头模式改变：";
    str.append(std::to_string(Mode_flag));
    emit UDPMsg(str);
}

void UDP_Thread::new_colormode(int index){
    Mode_color = index;
    QString str = QDateTime::currentDateTime().toString("hh:mm:ss") + "颜色模式改变：";
    str.append(std::to_string(Mode_color));
    emit UDPMsg(str);
}
