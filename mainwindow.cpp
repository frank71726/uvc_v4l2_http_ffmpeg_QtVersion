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
#include <QComboBox>
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

    //combobox
    for(int i =0; i<=10; i++)
        ui->video_sel->addItem("/dev/video" + QString::number(i));
    ui->size_sel->addItem("640x480");
    ui->size_sel->addItem("1280x720");
    ui->VType->addItem("MJPG");
    ui->VType->addItem("H264");

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));

    recordingTimer = new QTimer(this);
    connect(recordingTimer, SIGNAL(timeout()), this, SLOT(blinkSlot()));

    ui->actionRecord->setIcon(QIcon(":/picture/rec.png"));
    ui->label->setPixmap(QPixmap(":/picture/start.png"));

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
    if(ui->size_sel->currentText() == "640x480")        bi.biWidth = 640;
    else                                                bi.biWidth = 1280;

    if(ui->size_sel->currentText() == "640x480")        bi.biHeight = 480;
    else                                                bi.biHeight = 720;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = 0;
    if(ui->size_sel->currentText() == "640x480")        bi.biSizeImage = 640*480*3;
    else                                                bi.biSizeImage = 1280*720*3;
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

    if(ui->size_sel->currentText() == "640x480")
        ui->label->setGeometry(10, 0, 640, 480);
    else
        ui->label->setGeometry(10, 0, 1280, 720);

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
        ui->actionRecord->setEnabled(true);
        ui->NameEdit->setEnabled(true);
        ui->video_sel->setEnabled(false);
        ui->size_sel->setEnabled(false);
        //v4l2 setting
        if(ui->size_sel->currentText() == "640x480")
        {
            MainWindow::setGeometry(0,0,990,600);
            ui->Fun_list->setGeometry(670,0,310,510);
            vd = new video_device(ui->video_sel->currentText(), 640, 480);
            videoOutput = new QVideoOutput(640, 480);
            frame_rect.setRect(0,0,640,480);
        }
        else if(ui->size_sel->currentText() == "1280x720")
        {
            MainWindow::setGeometry(0,0,1635,720);
            ui->Fun_list->setGeometry(1310,0,310,510);

            vd = new video_device(ui->video_sel->currentText(), 1280, 720);
            videoOutput = new QVideoOutput(1280, 720);
            frame_rect.setRect(0,0,1280,720);
        }
        else
            qDebug() << "video_device constructor fail";

        ui->PlayBut->setText(_stop);
        timer->start(33);
        count = 1;
        ui->SavePic->setEnabled(1);
        qDebug() << "start showing";
    }
    else
    {
        ui->actionRecord->setEnabled(false);
        ui->NameEdit->setEnabled(false);
        ui->video_sel->setEnabled(true);
        ui->size_sel->setEnabled(true);
        ui->PlayBut->setText(_start);
        timer->stop();
        count = 0;
        ui->SavePic->setEnabled(0);
        delete vd;
        delete videoOutput;
        ui->label->setPixmap(QPixmap(":/picture/start.png"));
        qDebug() << "stop showing";
    }
}

void MainWindow::on_SavePic_released()
{
    save_pic_name =  ui->NameEdit->toPlainText(); + ".bmp";
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
        recordingTimer->stop();
        count = 0;
        videoOutput->closeMediaFile();
        ui->actionRecord->setText("Record Video");

        recording = false;
        QString fileName = QFileDialog::getSaveFileName(this,
                                                      tr("Save File"),
                                                      "",
                                                      tr("Videos (*.avi)"));
        fileName.append(".avi");
        if (fileName.isNull() == false)
        {
           QFile::copy(templateFile->fileName(), fileName);
        }
        delete templateFile;
        templateFile = 0x0;

        ui->actionRecord->setText("START REC");
        qDebug() << "save avi file successfully";
    }
}

void MainWindow::blinkSlot()
{
    static int i=0;

    qDebug("frame  %d",i);
    i++;

    QPixmap pixmap(frame_rect.size());
    // Get a screen shot
    render(&pixmap, QPoint(), QRegion(frame_rect));
    videoOutput->newFrame(pixmap.toImage());
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
    QMessageBox msgbox;
    QString fserver_ip = ui->ip_addr->text();
    QString fserver_port = ui->ip_port->text();

    if(fserver_ip.isEmpty())
    {
        msgbox.setIcon(QMessageBox::Critical);
        msgbox.setText("please input ip address !");
        msgbox.exec();
        return;
    }
    if(fserver_port.isEmpty())
    {
        msgbox.setIcon(QMessageBox::Critical);
        msgbox.setText("please input port number !");
        msgbox.exec();
        return;
    }

    QString fserver_url = "http://"+fserver_ip+":"+fserver_port+"/_file_server_upload/";
    QUrl original_url(fserver_url);
    QString bound="----WebKitFormBoundaryb62X3QGyAhb7Azg2"; //name of the boundary

    QString path(fileName);
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return;
    QStringList name_list = path.split("/");
    QString fn = name_list.value(name_list.length()-1);
    QStringList fn_type_list = fn.split(".");
    QString fn_type = fn_type_list.value(fn_type_list.length()-1);
    qDebug() << fserver_url;
    qDebug() << fn;
    qDebug() << fn_type;

    manager = new QNetworkAccessManager(this);
    //QNetworkRequest request(QUrl("http://192.168.1.123:8080/_file_server_upload/"));
    QNetworkRequest request(original_url);

    //according to rfc 1867 we need to put this string here:
    //QByteArray data(QString("--" + bound + "\r\n").toAscii());
    QByteArray data("------WebKitFormBoundaryb62X3QGyAhb7Azg2\r\n");
    //data.append("Content-Disposition: form-data; name=\"file\"; filename=\"1.jpg\"\r\n");
    QString data_1 = "Content-Disposition: form-data; name=\"file\"; filename=\"" + fn + "\"\r\n";
    data.append(data_1);
    //data.append("Content-Type: image/jpeg\r\n\r\n");
    if(fn_type == "jpg")
    {
        data.append("Content-Type: image/jpeg\r\n\r\n");
    }else if(fn_type == "avi")
    {
        data.append("Content-Type: video/avi\r\n\r\n");
    }else
    {
        qDebug() << "not support file !";
    }

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
    QString  usr_ok("1");
    QString  pw_ok("1");

    if((usr_login == usr_ok) && (pw_login == pw_ok) && (ui->login->text() == "Login"))
    {
        qDebug() << "loging successfully!";
        ui->PlayBut->setEnabled(true);

        ui->ip_addr->setEnabled(true);
        ui->ip_port->setEnabled(true);
        ui->FileSelect->setEnabled(true);
        ui->video_sel->setEnabled(true);
        ui->size_sel->setEnabled(true);

        ui->usr->clear();
        ui->usr->setEnabled(false);
        ui->pw->clear();
        ui->pw->setEnabled(false);
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
         ui->NameEdit->clear();
         ui->ip_addr->setEnabled(false);
         ui->ip_port->setEnabled(false);
         ui->FileSelect->setEnabled(false);
         ui->video_sel->setEnabled(false);
         ui->size_sel->setEnabled(false);
         ui->usr->setEnabled(true);
         ui->pw->setEnabled(true);

         if(ui->PlayBut->text() == "Stop")
            MainWindow::on_PlayBut_released();
         ui->label->setPixmap(QPixmap(":/picture/start.png"));
    }
    else
    {
        msgbox.setIcon(QMessageBox::Critical);
        msgbox.setText("loging fail");
        msgbox.exec();
    }
}

void MainWindow::on_FileSelect_released()
{
    QMessageBox msgbox;

    fileName = QFileDialog::getOpenFileName(this, "Open File...",
                QString(), tr("Images(*.jpg);;Video(*.avi)"));
    if(fileName.isEmpty())
    {
        msgbox.setIcon(QMessageBox::Critical);
        msgbox.setText("Please select file !");
        msgbox.exec();
        return;
    }
    ui->FileSend->setEnabled(true);
}
