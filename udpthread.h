#ifndef UDPTHREAD_H
#define UDPTHREAD_H

#include <QThread>
#include <QUdpSocket>
#include <QMutex>
#include <QPixmap>
#include <QDebug>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/videoio.hpp>

class UDP_Thread : public QObject
{
    Q_OBJECT
public:
    explicit UDP_Thread(QObject *parent1 = nullptr);
    ~UDP_Thread();
    void stop(); //暂停线程
    void go_on(); //继续线程

    // udpSocket需要的相关变量
    QString U_ip;// 上位机IP，用来给bind监听
    quint16 U_port;
    QString D_ip;// 下位IP，用来向FPGA发数据
    quint16 D_port;
    unsigned int len_count = 0;// 总长度计数

    // 图像相关的全局变量
    int W = 640;//视频宽度
    int H = 480;//视频高度

    // 8位帧头接收模式相关变量
    unsigned char Frame_Flag[4] = {0xf0, 0x5a, 0xa5, 0x0f};//帧头常量
    //正常包长度：2*W
    //帧头包长度：2*W+8
    int p_num = 0;//收报数，在两个帧头之间，正常值为H
    QByteArray bytes_Data; //图像原始数据队列
    QPixmap pixmap_Data; //图像数据队列
    QImage Image;//图像对象

    // 2位行头接收模式相关变量
    int last_num = -1;// 上一个包头值
    bool frame_flag_2 = 0;// 帧头标志位

    // 子线程核心代码
    //    void run()override;
    bool thread1_On = 0;//进程状态标识，在父进程使用开启关闭按键改动
    int Mode_display = 1;// 多路视频标识，默认就是一路，在父线程中改动
    int Mode_rec = 1;// 接收模式标识，默认0为正点格式；设为1为高云的连续模式
    int Mode_flag = 0;// 帧头模式标识，默认0为8位帧头；设为1为2位行头
    int Mode_color = 0;// 颜色模式标识，默认0为RGB565；设为1为YUV422

    void udpInit(QString U_ip,int U_port, QString D_ip,int D_port);
    void udpThreadStart();
    void udpBind(QString udpIp, int udpPort);

    // 调试用函数
    void Array2file(QString path,QByteArray Data);

signals:
    void img_OK(uchar *RGB_Buff, int _W, int _H);//收集够了一帧的信号发该信号给父进程

    void rec_count(unsigned p);// 接收字节数向mainwindow传输

    void UDPMsg(QString Msg);//将状态信息作为信号传入显示框

public slots:
    void onUdpThreadCreate();

    void statementChange();//修改thread1_On的槽函数

    void new_IP(QString Uip_changed, QString Dip_changed);// IP地址通过设置修改

    void new_port(quint16 Uport_changed, quint16 Dport_changed);// port通过设置修改

    void new_w(int w_changed);// 分辨率w通过设置修改

    void new_h(int h_changed);// 分辨率h通过设置修改

    void new_vedionum(int num);// 视频路数通过设置修改

    void new_colormode(int index);// 颜色模式通过设置修改

    void new_flagmode(int index);// 帧头模式通过设置修改

    void new_FPGAmode(int index);// 接收模式通过设置修改

    void onRecvUdpData();// 重构函数onRecv。与readread信号相连，socket收到包后自动触发该函数
private:
    QThread *udpThread;
    QUdpSocket *socket;
    QTimer *timer;
    QMutex mutex;//互斥量，用来暂停和继续线程
};

#endif // UDPTHREAD_H
