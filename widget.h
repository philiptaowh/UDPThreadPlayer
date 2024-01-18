#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QMediaPlayer>
#include <QTableWidgetItem>

#include "udpthread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    QMediaPlayer *player =new QMediaPlayer;
    QString dirname;
    QImage Frame;// 累计的帧
    QTimer *timer;// 计时器
    int Frame_num=0;// 帧数统计
    unsigned int Mb = 0;// 流量统计
    int FPS = 30;// 录屏帧率

    void img_save();// 保存图片

    void debug();//特殊的妙妙工具

signals:
    void t_setChange();//发送改变run函数起止状态的信号

    void ip_change(QString Uip_changed, QString Dip_changed);

    void port_change(quint16 Uport_changed, quint16 Dport_changed);

    void w_change(int w);

    void h_change(int h);

    void vedionum_change(int num);//实际上是调试模式和视频模式的切换

    void FPGAclass_change(int FPGA);//自己写的直接连续发送图像和正点需要等待上位机信号后再发送图像的模式切换

    void frameflag_change(int flag);//帧头模式切换，支持正点8位头和2位行序号头的切换

    void colormode_change(int index);//颜色模式切换，两种常用的16bit色彩格式，RGB565和YUV422

private slots:

    void onPlayerDurationChanged(qint64 duration);// 改变滑条的最大值

    void onPlayerPositionChanged(qint64 position);// 视频进度改动滑条

    void onSliderMoved(int value);// 通过滑条改动视频进度

    void on_displayMsg(QString Msg);// 将来自线程的信息显示在日志栏

    void rawdata_process(QByteArray data);// 接收调试模式的纯数据，显示在文字框中

    void img_process(uchar *RGB_Buff, int W, int H);// 处理子线程存好的图像数组，显示在框中

    void onrec_count(unsigned int p);// 上传数据量统计

    void onTimeout();// 计时器到时间响应函数，用于计算帧率和信息速率

    void on_rtp_Btn_clicked();

    void on_pb_Btn_clicked();

    void on_set_Btn_clicked();

    void on_Quit_Btn_clicked();

    void on_min_Btn_clicked();

    void on_max_Btn_clicked(bool checked);

    void on_open_Btn_clicked();

    void on_play_Btn_clicked();

    void on_tableWidget_itemClicked(QTableWidgetItem *item);

    void on_UIP_lineEdit_textChanged(const QString &arg1);

    void on_UPt_lineEdit_textChanged(const QString &arg1);

    void on_DIP_lineEdit_textChanged(const QString &arg1);

    void on_DPt_lineEdit_textChanged(const QString &arg1);

    void on_W_lineEdit_textChanged(const QString &arg1);

    void on_H_lineEdit_textChanged(const QString &arg1);

    void on_comboBox_Send_currentIndexChanged(int index);

    void on_comboBox_Flag_currentIndexChanged(int index);

    void on_comboBox_Color_currentIndexChanged(int index);

    void on_lineEdit_textChanged(const QString &arg1);

    void on_testmode_Btn_clicked();

    void on_Start_Stop_clicked();

    void on_setc_Btn_clicked();

    void on_catch_Btn_clicked();

    void on_logClr_Btn_clicked();

    void on_logSave_Btn_clicked();

    void on_catch2_Btn_clicked();

private:
    Ui::Widget *ui;
    UDP_Thread *t_udp;
    QPointF m_lastPos;
    int set_flag[5] = {0,0,0,0,0};
    void  mouseMoveEvent(QMouseEvent *event) override;
    void  mousePressEvent(QMouseEvent *event) override;
    void  mouseReleaseEvent(QMouseEvent *event) override;

};
#endif // WIDGET_H
