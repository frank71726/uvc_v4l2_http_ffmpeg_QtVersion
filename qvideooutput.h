#ifndef QVIDEOOUTPUT_H
#define QVIDEOOUTPUT_H

#include <QObject>
#include <QImage>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define __STDC_CONSTANT_MACROS

extern "C" {
#include "libavcodec/avcodec.h"
//#include "/libavutil/mathematics.h"
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include "libavfilter/avfilter.h"
#include "libswscale/swscale.h"

}

class QVideoOutput : public QObject
{
    Q_OBJECT
public:
    explicit QVideoOutput(int ff_width, int ff_heifgt,QObject *parent = 0 );
    ~QVideoOutput();
    bool openMediaFile(int width, int height, const QString &filename);
    AVStream * addStream(AVFormatContext *inFormatContext,
                         AVCodec **codec,
                         AVCodecID codecId);
    bool openVideo(AVCodec *codec, AVStream *st);
    bool newFrame(const QImage & image);
    bool writeVideoFrame(const AVPicture &src,
                            int srcWidth,
                            int srcHeight,
                            AVFormatContext *inFormatContext,
                            AVStream *st);
    bool closeMediaFile();
    void closeVideo(AVStream *st);
    int ffmpeg_initiate(void);
    int ffmpeg_yuyv_2_rgb888(const void *p, int size);

    AVFrame         *pFrameRGB;
    AVFrame         *pFrame;
signals:

public slots:

protected:
   // Protected members ////////////////////////////////////////////////////////
   AVFormatContext * formatContext;
   AVOutputFormat  * outputFormat;
   AVStream        * videoStream;
   AVCodec         * videoCodec;
   SwsContext      * swsContext;
   AVFrame         * frame;
   AVPicture srcPicture;
   AVPicture dstPicture;
   AVPixelFormat streamPixFmt;
   int swsFlags;
   int streamFrameRate;
   int width;
   int height;
   int frameCount;

   struct SwsContext *img_convert_ctx;
   int             numBytes;
   uint8_t         *buffer;
   uint8_t         *yuv_buffer;
};

#endif // QVIDEOOUTPUT_H
