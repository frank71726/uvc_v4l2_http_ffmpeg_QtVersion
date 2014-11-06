#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTimer>
#include <QPixmap>
#include <QImage>
#include <QPainter>
#include <QThread>
#include <QString>
#include <QFile>
#include <QDebug>
#include <QtGui>
#include <QTextEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QRect>
#include <QPainterPath>
#include <QPainter>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QDateTime>
#include <QFile>
#include <QDebug>

#include <QSettings>
#include <QVariant>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/time.h>
#include <linux/videodev2.h>
#include <linux/version.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

#include "video_device.h"
#include "v4l2grab.h"
#include "qvideooutput.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
  ,ui(new Ui::MainWindow)
  ,save_picture_flag(0)
{
    ui->setupUi(this);
    vd = new video_device(tr("/dev/video0"));
    videoOutput = new QVideoOutput;

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));

    recordingTimer = new QTimer(this);
    connect(recordingTimer, SIGNAL(timeout()), this, SLOT(blinkSlot()));

    ui->actionRecord->setIcon(QIcon(":/picture/rec.png"));

    //login
    QSettings set("cyh.ini",QSettings::IniFormat,this);
    QVariant  usr = set.value("usr");
    QVariant  pw  = set.value("pw");
    if(!usr.isNull() && !pw.isNull())
    {
        ui->usr->setText(usr.toString());
        ui->pw->setText(pw.toString());
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::update()
{
    QPixmap pix;
    QByteArray aa ;

    BITMAPFILEHEADER   bf;
    BITMAPINFOHEADER   bi;

    //Set BITMAPINFOHEADER
    bi.biSize = 40;
    bi.biWidth = IMAGEWIDTH;
    bi.biHeight = IMAGEHEIGHT;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = 0;
    bi.biSizeImage = IMAGEWIDTH*IMAGEHEIGHT*3;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    //Set BITMAPFILEHEADER
    bf.bfType = 0x4d42;
    bf.bfSize = 54 + bi.biSizeImage;
    bf.bfReserved = 0;
    bf.bfOffBits = 54;

    if(-1 == vd->get_frame(&yuv_buffer_pointer, &len))
    {
        qDebug() << " get_frame fail";
    }

    videoOutput->ffmpeg_yuyv_2_rgb888(yuv_buffer_pointer, len);
   // vd->yuyv_2_rgb888(yuv_buffer_pointer,len, rgb_frame_buffer);

    aa.append((const char *)&bf, 14);
    aa.append((const char *)&bi, 40);
    //aa.append((const char *)rgb_frame_buffer, bi.biSizeImage);
    aa.append((const char *)videoOutput->pFrameRGB->data[0], bi.biSizeImage);

    pix.loadFromData(aa);
    ui->label->setPixmap(pix);

    if(save_picture_flag == 1)
    {
        save_picture_flag = 0;
        QFile rgbfile(save_pic_name);

        if(!rgbfile.open(QIODevice::ReadWrite | QIODevice::Text))
        {
            QMessageBox msgbox(QMessageBox::Warning, "WARNING", "please enter file name");
            msgbox.exec();
            return;
        }
        rgbfile.write((const char *)&bf, 14);
        rgbfile.write((const char *)&bi, 40);
        rgbfile.write((const char *)videoOutput->pFrameRGB->data[0], bi.biSizeImage);

        QFile yuvfile("yuv.yuv");
        if(!yuvfile.open(QIODevice::ReadWrite | QIODevice::Text))
        {
            QMessageBox msgbox(QMessageBox::Warning, "WARNING", "please enter file name");
            msgbox.exec();
            return;
        }
        yuvfile.write((const char *)videoOutput->pFrame->data[0], len);
        qDebug() << "save picture successfully";
    }

    if(-1 == vd->unget_frame())
    {
        qDebug() << " unget_frame fail";
    }
}

void MainWindow::on_PlayBut_released()
{
    static unsigned char count=0;
    QString _start = "Start";
    QString _stop = "Stop";

    if((count == 0))
    {
        ui->PlayBut->setText(_stop);
        timer->start(33);
        count = 1;
        ui->SavePic->setEnabled(1);
        qDebug() << "start showing";
    }
    else
    {
        ui->PlayBut->setText(_start);
        timer->stop();
        count = 0;
        ui->SavePic->setEnabled(0);
        qDebug() << "stop showing";
    }
}

void MainWindow::on_SavePic_released()
{
    save_pic_name =  ui->NameEdit->toPlainText();
    save_picture_flag = 1;
}

void MainWindow::on_actionRecord_released()
{
    static unsigned char count=0, recording=0;
    QMessageBox msgbox1;

    if((count == 0) && (recording == 0))
    {
        count = 1;
        ui->actionRecord->setText("recording...");
        templateFile = new QTemporaryFile("qt_temp.XXXXXX.avi");
        if(templateFile->open())
        {
            int width = rect().size().width();
             int height = rect().size().height();
             width += width%2;
             height += height%2;
             qDebug("width=%d , height=%d", width, height);

             qDebug("2width=%d , 2height=%d", rect().size().width(), rect().size().height());

            QString fileName = templateFile->fileName();
            msgbox1.setText(fileName);
            msgbox1.exec();
            //recording = videoOutput->openMediaFile(640, 480, fileName);
            recording = videoOutput->openMediaFile( rect().size().width(), rect().size().height(), fileName);
        }
        ui->actionRecord->setText("STOP REC");
        recordingTimer->start(40);
    }
    else if((count == 1) && (recording == 1))
    {
        count = 0;
        videoOutput->closeMediaFile();
        ui->actionRecord->setText("Record Video");
        recordingTimer->stop();
        recording = false;
        QString fileName = QFileDialog::getSaveFileName(this,
                                                      tr("Save File"),
                                                      QString(),
                                                      tr("Videos (*.avi)"));
        if (fileName.isNull() == false)
        {
           QFile::copy(templateFile->fileName(), fileName);
        }
        delete templateFile;
        templateFile = 0x0;

         qDebug() << "save avi file successfully";
    }
}

void MainWindow::blinkSlot()
{
    static int i=0;
    QRect r1(0,0,640,480);

    qDebug("fuck  %d",i);
    i++;

    //QPixmap pixmap(rect().size());
    QPixmap pixmap(r1.size());
    // Get a screen shot
    //render(&pixmap, QPoint(), QRegion(rect()));
    render(&pixmap, QPoint(), QRegion(r1));
    videoOutput->newFrame(pixmap.toImage());
  //  blinkCount++;
}

/*
void MainWindow::on_pushButton_released()
{
   //  download from server
   // manager = new QNetworkAccessManager(this);
   // connect(manager, SIGNAL(finished(QNetworkReply*)),
   //             this, SLOT(replyFinished(QNetworkReply*)));
   // manager->get(QNetworkRequest(QUrl("http://192.168.1.123:8080/_file_server_download/123.txt")));


    manager = new QNetworkAccessManager(this);
    QString path("/home/frank/Qt_prj/camera-v4l2-ffmpeg/test.jpg");
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QNetworkRequest request(QUrl("http://192.168.1.123:8080/_file_server_upload/"));
    QString bound="----WebKitFormBoundaryb62X3QGyAhb7Azg2"; //name of the boundary

    //according to rfc 1867 we need to put this string here:
   // QByteArray data(QString("--" + bound + "\r\n").toAscii());
    QByteArray data("------WebKitFormBoundaryb62X3QGyAhb7Azg2\r\n");
    data.append("Content-Disposition: form-data; name=\"file\"; filename=\"test.jpg\"\r\n");
    data.append("Content-Type: image/jpeg\r\n\r\n");
    data.append(file.readAll());   //let's read the file
    data.append("\r\n");
    //data.append("--" + bound + "--\r\n");  //closing boundary according to rfc 1867
    data.append("------WebKitFormBoundaryb62X3QGyAhb7Azg2--\r\n");  //closing boundary according to rfc 1867

    request.setRawHeader("Content-Type","multipart/form-data; boundary=----WebKitFormBoundaryb62X3QGyAhb7Azg2");
    request.setHeader(QNetworkRequest::ContentLengthHeader,data.size());
    connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(replyFinished(QNetworkReply*)));
    QNetworkReply *reply1 = manager->post(request,data); // perform POST request
}
    */

void MainWindow::on_FileSend_released()
{
    /*  download from server
    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(replyFinished(QNetworkReply*)));
    manager->get(QNetworkRequest(QUrl("http://192.168.1.123:8080/_file_server_download/123.txt")));
    */

    manager = new QNetworkAccessManager(this);
    QString path("/home/frank/Qt_prj/camera-v4l2-ffmpeg/test.jpg");
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QNetworkRequest request(QUrl("http://192.168.1.123:8080/_file_server_upload/"));
    QString bound="----WebKitFormBoundaryb62X3QGyAhb7Azg2"; //name of the boundary

    //according to rfc 1867 we need to put this string here:
   // QByteArray data(QString("--" + bound + "\r\n").toAscii());
    QByteArray data("------WebKitFormBoundaryb62X3QGyAhb7Azg2\r\n");
    data.append("Content-Disposition: form-data; name=\"file\"; filename=\"test.jpg\"\r\n");
    data.append("Content-Type: image/jpeg\r\n\r\n");
    data.append(file.readAll());   //let's read the file
    data.append("\r\n");
    //data.append("--" + bound + "--\r\n");  //closing boundary according to rfc 1867
    data.append("------WebKitFormBoundaryb62X3QGyAhb7Azg2--\r\n");  //closing boundary according to rfc 1867

    request.setRawHeader("Content-Type","multipart/form-data; boundary=----WebKitFormBoundaryb62X3QGyAhb7Azg2");
    request.setHeader(QNetworkRequest::ContentLengthHeader,data.size());
    connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(replyFinished(QNetworkReply*)));
    QNetworkReply *reply1 = manager->post(request,data); // perform POST request
}


void MainWindow::replyFinished(QNetworkReply *reply)
{
    if(reply->error())
    {
        qDebug() << "ERROR!";
        qDebug() << reply->errorString();
    }
    else
    {
        qDebug() << reply->header(QNetworkRequest::ContentTypeHeader).toString();
        qDebug() << reply->header(QNetworkRequest::LastModifiedHeader).toDateTime().toString();
        qDebug() << reply->header(QNetworkRequest::ContentLengthHeader).toULongLong();
        qDebug() << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
/*
        QFile *file = new QFile("/home/frank/downloaded.txt");
        if(file->open(QFile::Append))
        {
            file->write(reply->readAll());
            file->flush();
            file->close();
        }
        delete file;
    }

    reply->deleteLater();
    */
    }
}

void MainWindow::on_login_released()
{
    QMessageBox msgbox;
    QString  usr_login = ui->usr->text();
    QString  pw_login  = ui->pw->text();
    QString  usr_ok("rd2");
    QString  pw_ok("123456");

    if((usr_login == usr_ok) && (pw_login == pw_ok) && (ui->login->text() == "Login"))
    {
        qDebug() << "loging successfully!";
        ui->PlayBut->setEnabled(true);
        ui->actionRecord->setEnabled(true);
        ui->FileSend->setEnabled(true);
        ui->NameEdit->setEnabled(true);

        ui->usr->clear();
        ui->pw->clear();
        ui->login->setText("Logout");
    }
    else if(ui->login->text() == "Logout")
    {
         qDebug() << "logout!";
         ui->login->setText("Login");
         ui->PlayBut->setEnabled(false);
         ui->actionRecord->setEnabled(false);
         ui->FileSend->setEnabled(false);
         ui->NameEdit->setEnabled(false);

         if(ui->PlayBut->text() == "Stop")
            MainWindow::on_PlayBut_released();
    }
    else
    {
        msgbox.setIcon(QMessageBox::Critical);
        msgbox.setText("loging fail");
        msgbox.exec();
    }
}

