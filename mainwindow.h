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

public slots:
    void replyFinished (QNetworkReply *reply);

private slots:
    void on_PlayBut_released();
    void update();
    void on_SavePic_released();
    void on_actionRecord_released();
    void blinkSlot();
    void on_pushButton_released();


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
};

#endif // MAINWINDOW_H
