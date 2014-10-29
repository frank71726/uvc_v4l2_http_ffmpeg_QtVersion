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

    if(count == 0)
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
