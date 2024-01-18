#include "SRthread.h"
#include <QDir>
#include <QTimer>
#include <QImage>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/videoio.hpp>

SR_Thread::SR_Thread(QObject *parent1)
{
    sr_Thread = new QThread();
    connect(sr_Thread,&QThread::finished,this,&QObject::deleteLater);                    //线程结束，自动删除对象
    this->moveToThread(sr_Thread);

}

SR_Thread::~SR_Thread()
{
    disconnect(sr_Thread,&QThread::finished,this,&QObject::deleteLater);                    //线程结束，自动删除对象
    if(sr_Thread)
    {
        if(sr_Thread->isRunning())
        {
            sr_Thread->quit();
            sr_Thread->wait();
        }
        delete sr_Thread;
        sr_Thread = NULL;
    }
}

void SR_Thread::SRThreadStart()
{
    sr_Thread->start();
}

void SR_Thread::onStartRecord(int W,int H)
{
    //初始化视频写入对象，需要准备：路径、编解码器、帧率、宽度和高度；由于没有准备ffmpeg的包，所以就直接用avi的编解码器了
    path = QDir::currentPath();
    path.replace("/","\\");
    path.append("\\"+QDateTime::currentDateTime().toString("/yyyy-MM-dd_HH：mm：ss"));
    path.append("_vedio.avi");// 确认路径
    std::string str = path.toStdString();
    cv::Size size = cv::Size(W,H);
    writer.open(str,cv::VideoWriter::fourcc('M','J','P','G'),FPS,size,true);//新建一个视频
    QString Msg = QDateTime::currentDateTime().toString("hh:mm:ss") + " 开始录屏";
    emit SRMsg(Msg);
}


void SR_Thread::onStopRecord()
{
    writer.release();
    QString Msg = QDateTime::currentDateTime().toString("hh:mm:ss") + " 录屏已保存";
    Msg.append(path);
    emit SRMsg(Msg);
}


void SR_Thread::onFramesend(QImage Frame)
{
    cv::Mat mat = cv::Mat(Frame.height(), Frame.width(), CV_8UC2, (void*)Frame.constBits(), Frame.bytesPerLine());//RGB565，8位数据2通道，因此对应CV_8UC2
    writer.write(mat);
}

void SR_Thread::new_FPS(int f)
{
    FPS = f;
}
