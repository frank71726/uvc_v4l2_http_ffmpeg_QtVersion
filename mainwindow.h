#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include "video_device.h"
#include "qvideooutput.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QDateTime>
#include <QFile>
#include <QDebug>
#include <QPainterPath>
#include <QPainter>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    unsigned char rgb_frame_buffer[640*480*3];
    ~MainWindow();
    QVideoOutput *videoOutput;

/*
    void paintEvent(QPaintEvent * event)
    {
        QMainWindow::paintEvent(event);

        QPainter p(this);
        p.setPen(QPen(Qt::red, 5));
        p.drawLine(650,140,995,140);
        p.drawLine(650,335,995,335);
        p.drawLine(650,430,995,430);
        qDebug() << "draw line";
    }
*/
public slots:
    void replyFinished (QNetworkReply *reply);

private slots:
    void on_PlayBut_released();
    void update();
    void on_SavePic_released();
    void on_actionRecord_released();
    void blinkSlot();
 //   void on_pushButton_released();
    void on_login_released();
    void on_FileSend_released();
    void on_FileSelect_released();

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    QTimer *recordingTimer;
    video_device *vd;
    size_t len;
    unsigned char *yuv_buffer_pointer;
    QString save_pic_name;
    bool save_picture_flag;
    QTemporaryFile *templateFile;

    QNetworkAccessManager *manager;
    QString fileName;
    QRect frame_rect;
};

#endif // MAINWINDOW_H
