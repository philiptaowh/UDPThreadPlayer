#ifndef SRTHREAD_H
#define SRTHREAD_H

#include <QThread>
#include <QImage>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/videoio.hpp>

class SR_Thread : public QObject
{
    Q_OBJECT
public:
    explicit SR_Thread(QObject *parent1 = nullptr);
    ~SR_Thread();
    void SRThreadStart();

    int FPS = 30;// 录屏帧率

signals:
    void SRMsg(QString Msg);//将状态信息作为信号传入显示框

public slots:
    void onStartRecord(int W,int H);//开始录屏

    void onStopRecord();//停止录屏

    void onFramesend(QImage Frame);//接收一帧存起来

    void new_FPS(int f);

private:
    QThread *sr_Thread;
    cv::VideoWriter writer;
    QString path;
};

#endif // SRTHREAD_H
