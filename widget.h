#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:

    void on_rtp_Btn_clicked();

    void on_pb_Btn_clicked();

    void on_set_Btn_clicked();

    void on_Quit_Btn_clicked();

    void on_min_Btn_clicked();

    void on_max_Btn_clicked(bool checked);

private:
    Ui::Widget *ui;
    QPointF m_lastPos;
    void  mouseMoveEvent(QMouseEvent *event) override;
    void  mousePressEvent(QMouseEvent *event) override;
    void  mouseReleaseEvent(QMouseEvent *event) override;

};
#endif // WIDGET_H
