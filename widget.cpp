#include "widget.h"
#include "ui_widget.h"
#include <QMouseEvent>
#include <QTableWidgetItem>
#include <QMediaPlayer>
#include <QFileDialog>
#include <QDir>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setWindowFlag(Qt::FramelessWindowHint);

    connect(player,SIGNAL(durationChanged(qint64)),this,SLOT(onPlayerDurationChanged(qint64)));//视频长度改变，打印出来
    connect(player,SIGNAL(positionChanged(qint64)),this,SLOT(onPlayerPositionChanged(qint64)));//进度条位置改变，打印出来
    connect(ui->horizontalSlider,SIGNAL(sliderMoved(int)),this,SLOT(onSliderMoved(int)));//拖动进度条，改变视频的进度

    ui->tableWidget->setColumnCount(1);//设置列数
    QStringList header;
    header<< tr("文件名") ;
    ui->tableWidget->setHorizontalHeaderLabels(header);//设置行头
    ui->tableWidget->setColumnWidth(0, 197);//使行长等于表格长
}

Widget::~Widget()
{
    delete ui;
}

// 鼠标事件重写

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
}

//切换回放界面
void Widget::on_pb_Btn_clicked()
{
    ui->display_widget->setCurrentIndex(1);
    ui->tool_widget->setCurrentIndex(1);
}

//切换设置界面
void Widget::on_set_Btn_clicked()
{
    ui->display_widget->setCurrentIndex(2);
    ui->tool_widget->setCurrentIndex(2);
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
    filter<<"*.mp4";

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

