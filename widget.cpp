#include "widget.h"
#include "ui_widget.h"
#include "udpthread.h"
#include <QMouseEvent>
#include <QTableWidgetItem>
#include <QMediaPlayer>
#include <QFileDialog>
#include <QDir>
#include <QTimer>
#include <QDateTime>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setWindowFlag(Qt::FramelessWindowHint);//回放功能屏幕状态设置
    t_udp = new UDP_Thread(this);//新建接收用UDPSocket
    t_udp->udpInit("192.168.1.102",1234,"192.168.1.10",1234);
    t_udp->udpThreadStart();

    connect(t_udp,&UDP_Thread::UDPMsg,this,&Widget::on_displayMsg);// 线程向主函数传输日志
    connect(player,SIGNAL(durationChanged(qint64)),this,SLOT(onPlayerDurationChanged(qint64)));//视频长度改变，打印出来
    connect(player,SIGNAL(positionChanged(qint64)),this,SLOT(onPlayerPositionChanged(qint64)));//进度条位置改变，打印出来
    connect(ui->horizontalSlider,SIGNAL(sliderMoved(int)),this,SLOT(onSliderMoved(int)));//拖动进度条，改变视频的进度
    connect(this,&Widget::t_setChange,t_udp,&UDP_Thread::statementChange);// 父线程发送"开始"信号，run的真正功能部分开始运行
    connect(t_udp,&UDP_Thread::img_OK,this,&Widget::img_process);// 子线程回发数据，父线程处理数据
    connect(t_udp,&UDP_Thread::rec_count,this,&Widget::onrec_count);// 子线程发送流量数据
    connect(this,&Widget::ip_change,t_udp,&UDP_Thread::new_IP);//IP改变
    connect(this,&Widget::port_change,t_udp,&UDP_Thread::new_port);//port改变
    connect(this,&Widget::w_change,t_udp,&UDP_Thread::new_w);//分辨率w改变
    connect(this,&Widget::h_change,t_udp,&UDP_Thread::new_h);//分辨率h改变
    connect(this,&Widget::vedionum_change,t_udp,&UDP_Thread::new_vedionum);//视频路数改变
    connect(this,&Widget::frameflag_change,t_udp,&UDP_Thread::new_flagmode);//多路帧头改变
    connect(this,&Widget::colormode_change,t_udp,&UDP_Thread::new_colormode);//颜色模式改变
    connect(this,&Widget::FPGAclass_change,t_udp,&UDP_Thread::new_FPGAmode);//颜色模式改变

    ui->tableWidget->setColumnCount(1);//设置列数
    QStringList header;
    header<< tr("文件名") ;
    ui->tableWidget->setHorizontalHeaderLabels(header);//设置行头
    ui->tableWidget->setColumnWidth(0, 197);//使行长等于表格长

    timer = new QTimer();// 设置定时器，便于计算帧率和网速
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));// 槽函数连接定时器结束函数
    timer->start(1000);

    ui->display_widget->setCurrentIndex(0);//初始化显示屏
    ui->tool_widget->setCurrentIndex(0);//初始化工具栏
    ui->log_Browser->append("你好，欢迎使用系统");
}

Widget::~Widget()
{
    delete ui;
}

// 图片显示代码
void Widget::img_process(uchar *RGB_Buff, int W, int H){
    // 将子线程图片数据指针传给父线程
    Frame = QImage (RGB_Buff,W,H,QImage::Format_RGB16);
    // 修改显示框并显示图片
    ui->img_label->resize(Frame.size());
    ui->img_label->setPixmap(QPixmap::fromImage(Frame));
    ui->img_label->show();
    Frame_num++;// 帧率累计
}

// 保存照片代码
void Widget::img_save()
{
    QString Msg = "";
    bool isok = Frame.isNull();
    if(isok){
        Msg = QDateTime::currentDateTime().toString("hh:mm:ss") + " 暂无图片数据";
        ui->log_Browser->append(Msg);
    }
    else{
        QString path = QDir::currentPath();
        path.replace("/","\\");
        path.append("\\"+QDateTime::currentDateTime().toString("/yyyy-MM-dd_HH：mm：ss"));
        path.append(".png");
        isok = Frame.save(path,"PNG",100);
        if(isok){
            Msg = QDateTime::currentDateTime().toString("hh:mm:ss") + " 图片已保存 " + path;
            ui->log_Browser->append(Msg);
        }
        else{
            Msg = QDateTime::currentDateTime().toString("hh:mm:ss") + " 图片保存失败";
            ui->log_Browser->append(Msg);
        }
    }

}

// 计算帧率和传输速率代码
void Widget::onTimeout(){
    ui->statusbar->setText(tr("FPS：%1 | bps：%2").arg(Frame_num).arg(Mb));
    Frame_num = 0;
    Mb = 0;
    timer->start(1000);

}

// 传输量累计代码
void Widget::onrec_count(unsigned int p){
    Mb += p;
}

//开始接收数据代码
void Widget::on_Start_Stop_clicked()
{
    if(ui->Start_Stop->text() == "开始")
    {
        emit t_setChange();
        ui->Start_Stop->setText(tr("停止"));
    }


    else if(ui->Start_Stop->text()=="停止")
    {
        emit t_setChange();
        ui->Start_Stop->setText(tr("开始"));
    }
}

//鼠标移动事件
void Widget::mouseMoveEvent(QMouseEvent *event)
{
    //移动窗口
    this->move(this->x()+(event->globalPosition().x()-m_lastPos.x()),this->y()+(event->globalPosition().y()-m_lastPos.y()));
    //记录窗口坐标
    m_lastPos = event->globalPosition();
}

//鼠标按下事件
void Widget::mousePressEvent(QMouseEvent *event)
{
    //记录鼠标按下时的窗口坐标
    m_lastPos = event->globalPosition();
}

//鼠标松开事件
void Widget::mouseReleaseEvent(QMouseEvent *event)
{
    //记录鼠标松开时窗口的坐标
    m_lastPos = event->globalPosition();
}

//切换直播界面
void Widget::on_rtp_Btn_clicked()
{
    ui->display_widget->setCurrentIndex(0);
    ui->tool_widget->setCurrentIndex(0);
    QString Msg = QDateTime::currentDateTime().toString("hh:mm:ss") + " 进入直播模式";
    ui->log_Browser->append(Msg);
}

//切换回放界面
void Widget::on_pb_Btn_clicked()
{
    ui->display_widget->setCurrentIndex(1);
    ui->tool_widget->setCurrentIndex(1);
    QString Msg = QDateTime::currentDateTime().toString("hh:mm:ss") + " 进入回放模式";
    ui->log_Browser->append(Msg);
}

//切换设置界面
void Widget::on_set_Btn_clicked()
{
    ui->display_widget->setCurrentIndex(2);
    ui->tool_widget->setCurrentIndex(2);
    QString Msg = QDateTime::currentDateTime().toString("hh:mm:ss") + " 进入设置";
    ui->log_Browser->append(Msg);
}

//重写退出按钮
void Widget::on_Quit_Btn_clicked()
{
    this->close();
}

//重写最小化按钮
void Widget::on_min_Btn_clicked()
{
    this->showMinimized();

}

//重写最大化按钮
void Widget::on_max_Btn_clicked(bool checked)
{
    if(!checked){
        this->showMaximized();
        ui->max_Btn->setCheckable(true);
    }
    else{
        this->showNormal();
        ui->max_Btn->setCheckable(false);
    }
}

//回放模式进度条最大值设置
void Widget::onPlayerDurationChanged(qint64 duration){
    ui->horizontalSlider->setMaximum(duration);  //最大值
}

//回放模式进度条自动变化设置
void Widget::onPlayerPositionChanged(qint64 position){
    ui->horizontalSlider->setValue(position);  //进度条进度跟着变
}

//回放模式进度条拖动变化设置
void Widget::onSliderMoved(int value){
    player->setPosition(value);  //视频跟着进度条变
}

//回放模式打开文件夹
void Widget::on_open_Btn_clicked()
{
    QString path=QDir::currentPath();
    path.replace("/","\\");
    QStringList filter;
    filter<<"*.mp4"<<"*.avi";

    dirname = QFileDialog::getExistingDirectory();
    QDir dirlist(dirname);

    QStringList MP4s = dirlist.entryList(filter, QDir::Files);
    for(int i = 0; i < MP4s.size(); i++){
        int index = ui->tableWidget->rowCount();//确认目前列表中的行数
        ui->tableWidget->insertRow(index);//在表的最末插入新行
        ui->tableWidget->setItem(index,0,new QTableWidgetItem(MP4s.at(i)));//在新行中加入项目
    }
}

//回放模式播放按钮
void Widget::on_play_Btn_clicked()
{
    if(ui->play_Btn->text() == "暂停"){
        ui->play_Btn->setText("播放");
        player->play();
    }else{
        ui->play_Btn->setText("暂停");
        player->pause();
    }
}

//回放模式选择视频
void Widget::on_tableWidget_itemClicked(QTableWidgetItem *item)
{
    QString dir = dirname+"/"+item->text();
    player->setSource(QUrl(dir));
    player->setVideoOutput(ui->img_label2);
    player->play();
}

//本机IP改变
void Widget::on_UIP_lineEdit_textChanged(const QString &arg1)
{
    this->set_flag[0] = 1;
}

//本机port改变
void Widget::on_UPt_lineEdit_textChanged(const QString &arg1)
{
    this->set_flag[1] = 1;
}

//FPGAIP改变
void Widget::on_DIP_lineEdit_textChanged(const QString &arg1)
{
    this->set_flag[0] = 1;
}

//FPGAport改变
void Widget::on_DPt_lineEdit_textChanged(const QString &arg1)
{
    this->set_flag[1] = 1;
}

//分辨率W改变
void Widget::on_W_lineEdit_textChanged(const QString &arg1)
{
    this->set_flag[2] = 1;
}

//分辨率H改变
void Widget::on_H_lineEdit_textChanged(const QString &arg1)
{
    this->set_flag[3] = 1;
}

//数据开始发送模式改变
void Widget::on_comboBox_Send_currentIndexChanged(int index)
{
    emit FPGAclass_change(index);
}

//帧头模式改变
void Widget::on_comboBox_Flag_currentIndexChanged(int index)
{
    emit frameflag_change(index);
}

//色彩模式改变
void Widget::on_comboBox_Color_currentIndexChanged(int index)
{
    emit colormode_change(index);
}

//调试&播放模式改变
void Widget::on_testmode_Btn_clicked()
{
    int index;
    if(ui->testmode_Btn->text() == "调试模式"){
        ui->testmode_Btn->setText("播放模式");
        index = 0;
        QString Msg = QDateTime::currentDateTime().toString("hh:mm:ss") + " 更改为调试模式";
        ui->log_Browser->append(Msg);
    }else{
        ui->testmode_Btn->setText("调试模式");
        index = 1;
        QString Msg = QDateTime::currentDateTime().toString("hh:mm:ss") + " 更改为播放模式";
        ui->log_Browser->append(Msg);
    }
    emit vedionum_change(index);
}

//日志框显示内容
void Widget::on_displayMsg(QString Msg){
    ui->log_Browser->append(Msg);
}

//设置确认
void Widget::on_setc_Btn_clicked()
{
    if(this->set_flag[0]){
        QString D_ip;
        D_ip=ui->DIP_lineEdit->text();
        QString U_ip;
        U_ip=ui->UIP_lineEdit->text();
        emit ip_change(U_ip,D_ip);
    }
    if(this->set_flag[1]){
        quint16 D_port;
        D_port=ui->DPt_lineEdit->text().toInt();
        quint16 U_port;
        U_port=ui->UPt_lineEdit->text().toInt();
        emit port_change(U_port,D_port);
    }
    if(this->set_flag[2]){
        int W;
        W=ui->W_lineEdit->text().toInt();
        emit w_change(W);
    }
    if(this->set_flag[3]){
        int H;
        H=ui->H_lineEdit->text().toInt();
        emit h_change(H);
    }
    for(int i = 0; i<4 ; i++){
        this->set_flag[i] = 0;
    }
}

//截屏
void Widget::on_catch_Btn_clicked()
{
    img_save();
}

