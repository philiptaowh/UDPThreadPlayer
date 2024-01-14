#include "widget.h"
#include "ui_widget.h"
#include <QMouseEvent>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setWindowFlag(Qt::FramelessWindowHint);
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

