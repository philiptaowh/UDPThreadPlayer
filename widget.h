#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QMediaPlayer>
#include <QTableWidgetItem>

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

private slots:

    void onPlayerDurationChanged(qint64 duration);

    void onPlayerPositionChanged(qint64 position);

    void onSliderMoved(int value);

    void on_rtp_Btn_clicked();

    void on_pb_Btn_clicked();

    void on_set_Btn_clicked();

    void on_Quit_Btn_clicked();

    void on_min_Btn_clicked();

    void on_max_Btn_clicked(bool checked);

    void on_open_Btn_clicked();

    void on_play_Btn_clicked();

    void on_tableWidget_itemClicked(QTableWidgetItem *item);

private:
    Ui::Widget *ui;
    QPointF m_lastPos;
    void  mouseMoveEvent(QMouseEvent *event) override;
    void  mousePressEvent(QMouseEvent *event) override;
    void  mouseReleaseEvent(QMouseEvent *event) override;

};
#endif // WIDGET_H
