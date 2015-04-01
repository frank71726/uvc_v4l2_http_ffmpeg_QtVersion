#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include "stdio.h"
#include "stddef.h"
#include "qvideooutput.h"
#include "v4l2grab.h"

extern "C" {
#include "/usr/local/include/libavcodec/avcodec.h"
#include "/usr/local/include/libavformat/avformat.h"
#include "/usr/local/include/libavdevice/avdevice.h"
#include "/usr/local/include/libavfilter/avfilter.h"
#include "/usr/local/include/libswscale/swscale.h"
}

QVideoOutput::QVideoOutput(int ff_width, int ff_heifgt, QObject *parent) :
    QObject(parent)
  , swsContext(0x0)
  , formatContext(0x0)
  , outputFormat(0x0)
  , videoStream(0x0)
  , videoCodec(0x0)
  , frame(0x0)
  , swsFlags(SWS_BICUBIC)
  , streamPixFmt(AV_PIX_FMT_YUV420P) // default pix_fmt
  , streamFrameRate(30)              // 30 images/s
  {
     // Init FFmpeg
     av_register_all();
     avcodec_register_all();
     avdevice_register_all();

     this->width = ff_width;
     this->height = ff_heifgt;
     ffmpeg_initiate();
  }

int QVideoOutput::ffmpeg_initiate(void)
{
    int ret;

    img_convert_ctx = sws_getContext(width, height, AV_PIX_FMT_YUYV422,
                      width, height, AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);

    if(img_convert_ctx == NULL)
    {
        qDebug() << "-------------Cannot initialize the conversion context!";
        return -1;
     }
     qDebug() << "-------------sws_getContext initialize the conversion context successful!";

     // Allocate video frame
     pFrame = av_frame_alloc();
     if(pFrame==NULL)
        return -1;

     pFrame->width = width;
     pFrame->height = height;
     pFrame->format = AV_PIX_FMT_YUYV422;
     // Allocate a buffer large enough for all data
     int size = avpicture_get_size(AV_PIX_FMT_YUYV422, pFrame->width, pFrame->height);
     yuv_buffer = (uint8_t*)av_malloc(size*sizeof(uint8_t));
     if(yuv_buffer==NULL)
        return -1;
     ret = avpicture_fill((AVPicture*)pFrame, yuv_buffer, AV_PIX_FMT_YUYV422, width, height);
     if(ret < 0)
        return -1;
     qDebug() << "-------------pFram alloc_fram successful!";

     // Allocate an AVFrame structure
     pFrameRGB = av_frame_alloc();
     if(pFrameRGB==NULL)
        return -1;
     qDebug() << "-------------pFramRGB alloc_fram successful!";

     // Determine required buffer size and allocate buffer
     numBytes = avpicture_get_size(AV_PIX_FMT_BGR24, width, height);
     buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
     if(buffer==NULL)
        return -1;
     qDebug("-------------%d - av_malloc successful!",numBytes);

     // Assign appropriate parts of buffer to image planes in pFrameRGB
     // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
     // of AVPicture
     ret = avpicture_fill((AVPicture *)pFrameRGB, buffer, AV_PIX_FMT_BGR24, width, height);
     if(ret < 0)
        return -1;
     qDebug("-------------%d - avpicture_fill successful!",ret);

     return 0;
}

int QVideoOutput::ffmpeg_yuyv_2_rgb888(const void *p, int size)
{
    int ret;

    memcpy(pFrame->data[0],(const u_int8_t *) p, size);
    sws_scale(img_convert_ctx, (const uint8_t * const *)pFrame->data, pFrame->linesize, 0, height, pFrameRGB->data, pFrameRGB->linesize);
}
////////////////////////////////////////////////////////////////////////////////
//  QVideoOutput::~QVideoOutput
//! @brief Destructor
////////////////////////////////////////////////////////////////////////////////
QVideoOutput::~QVideoOutput()
{

    //Free the RGB image
    av_free(yuv_buffer);
    av_free(buffer);
    qDebug() << "kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk";
    av_frame_free(&pFrameRGB);
    av_frame_free(&pFrame);
}

////////////////////////////////////////////////////////////////////////////////
//  QVideoOutput::addStream
////////////////////////////////////////////////////////////////////////////////
AVStream *QVideoOutput::addStream(AVFormatContext * inFormatContext, AVCodec **codec, AVCodecID codecId)
{
    AVCodecContext *c;
    AVStream *st;
    // find the encoder
    *codec = avcodec_find_encoder(codecId);
    if (!(*codec))
    {
        qDebug("Could not find encoder for '%s'\n", avcodec_get_name(codecId));
        return 0x0;
    }

    st = avformat_new_stream(inFormatContext, *codec);
    if (!st)
    {
        qDebug() << "Could not allocate stream\n";
        return 0x0;
    }
    st->id = inFormatContext->nb_streams-1;
    c = st->codec;
    switch ((*codec)->type)
    {
    case AVMEDIA_TYPE_AUDIO:
        st->id = 1;
        c->sample_fmt  = AV_SAMPLE_FMT_S16;
        c->bit_rate    = 64000;
        c->sample_rate = 44100;
        c->channels    = 2;
        break;
    case AVMEDIA_TYPE_VIDEO:
        avcodec_get_context_defaults3(c, *codec);
        c->codec_id = codecId;
        c->bit_rate = 400000;
        c->width    = width;
        c->height   = height;
        // timebase: This is the fundamental unit of time (in seconds) in terms
        // of which frame timestamps are represented. For fixed-fps content,
        // timebase should be 1/framerate and timestamp increments should be
        // identical to 1.
        c->time_base.den = streamFrameRate;
        c->time_base.num = 1;
        c->gop_size      = 12; // emit one intra frame every twelve frames at most
        c->pix_fmt       = streamPixFmt;
        if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO)
        {
            // just for testing, we also add B frames
            c->max_b_frames = 2;
        }
        if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO)
        {
            // Needed to avoid using macroblocks in which some coeffs overflow.
            // This does not happen with normal video, it just happens here as
            // the motion of the chroma plane does not match the luma plane.
            c->mb_decision = 2;
        }
    break;
    default:
        break;
    }
    // Some formats want stream headers to be separate.
    if (inFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;
    return st;
}

////////////////////////////////////////////////////////////////////////////////
//  QVideoOutput::openVideo
////////////////////////////////////////////////////////////////////////////////
bool QVideoOutput::openVideo(AVCodec *codec, AVStream *stream)
{
    int ret;
    AVCodecContext *c = stream->codec;
    // open the codec
    ret = avcodec_open2(c, codec, NULL);
    if (ret < 0)
    {
       //qDebug( "Could not open video codec: %s\n", av_err2str(ret));
        qDebug() << "Could not open video codec";
       return false;
    }
    // allocate and init a re-usable frame
    frame = avcodec_alloc_frame();
    if (!frame)
    {
       qDebug() << "Could not allocate video frame";
       return false;
    }
    // Allocate the encoded raw picture.
    ret = avpicture_alloc(&dstPicture, c->pix_fmt, c->width, c->height);
    if (ret < 0)
    {
        //qDebug("Could not allocate picture: %s\n", av_err2str(ret));
        qDebug() << "Could not allocate picture";
        return false;
    }
    // copy data and linesize picture pointers to frame
    *((AVPicture *)frame) = dstPicture;
    return true;
}
////////////////////////////////////////////////////////////////////////////////
//  QVideoOutput::openMediaFile
////////////////////////////////////////////////////////////////////////////////
bool QVideoOutput::openMediaFile(int width, int height, const QString &filename)
{
    QByteArray byteArray = filename.toUtf8();
    const char *cString = byteArray.constData();

    avformat_alloc_output_context2(&formatContext, NULL, NULL, cString);
    if (!formatContext)
    {
        qDebug() << "Could not deduce output format from file extension: using MPEG";
        avformat_alloc_output_context2(&formatContext, NULL, "mpeg", cString);
    }
    if (!formatContext)
    {
        return false;
    }
    outputFormat = formatContext->oformat;
    // Add the video streams using the default format codecs
    // and initialize the codecs.
    videoStream = NULL;
    if (outputFormat->video_codec != AV_CODEC_ID_NONE)
    {
        videoStream = addStream(formatContext, &videoCodec, outputFormat->video_codec);
    }

    // Now that all the parameters are set, we can open the audio and
    // video codecs and allocate the necessary encode buffers.
    if (videoStream)
       openVideo(videoCodec, videoStream);

    av_dump_format(formatContext, 0, cString, 1);
    int ret = 0;
    // open the output file, if needed
   if (!(outputFormat->flags & AVFMT_NOFILE))
   {
       ret = avio_open(&formatContext->pb, cString, AVIO_FLAG_WRITE);
       if (ret < 0)
       {
           //fprintf(stderr, "Could not open '%s': %s\n", filename, av_err2str(ret));
           qDebug("Could not open '%s'\n", cString);
           return false;
       }
   }
   // Write the stream header, if any.
   ret = avformat_write_header(formatContext, NULL);
   if (ret < 0)
   {
       //fprintf(stderr, "Error occurred when opening output file: %s\n", av_err2str(ret));
       qDebug() << "Error occurred when opening output file";
       return false;
   }
   if (frame)
       frame->pts = 0;

   return avpicture_alloc(&srcPicture, AV_PIX_FMT_RGBA, width, height) >= 0;
}

///////////////////////////////////////////////////////////////////////////////
//  QVideoOutput::newFrame
//! @brief Adds new frame to ouput stream
//! @param[in]  image :
////////////////////////////////////////////////////////////////////////////////
bool QVideoOutput::newFrame(const QImage & image)
{
   const int width  = image.width();
   const int height = image.height();

   // write video frames
   for (int y = 0; y < height; y++)
   {
      const uint8_t * scanline = image.scanLine(y);
      for (int x = 0; x < width*4; x++)
      {
         srcPicture.data[0][y * srcPicture.linesize[0] + x] = scanline[x];
      }
   }
   writeVideoFrame(srcPicture, width, height, formatContext, videoStream);
   frame->pts += av_rescale_q(1,
                              videoStream->codec->time_base,
                              videoStream->time_base);
   return true;
}
////////////////////////////////////////////////////////////////////////////////
//  QVideoOutput::writeVideoFrame
//! @brief Writes video frame
////////////////////////////////////////////////////////////////////////////////
bool QVideoOutput::writeVideoFrame(const AVPicture &src,
                                   int srcWidth,
                                   int srcHeight,
                                   AVFormatContext * inFormatContext,
                                   AVStream *stream)
{
    int ret;
    AVCodecContext *c = stream->codec;

    if (c->pix_fmt != AV_PIX_FMT_RGBA)
    {
       // as we only use RGBA picture, we must convert it
       // to the codec pixel format if needed
      if (!swsContext)
      {
           swsContext = sws_getContext(srcWidth,
                                       srcHeight,
                                       AV_PIX_FMT_BGRA,
                                       c->width,
                                       c->height,
                                       c->pix_fmt,
                                       swsFlags,
                                       NULL,
                                       NULL,
                                       NULL);
           if (!swsContext)
           {
               qDebug() << "Could not initialize the conversion context";
               return false;
           }
       }

       sws_scale(swsContext,
                 (const uint8_t * const *)src.data,
                 src.linesize,
                 0,
                 c->height,
                 dstPicture.data,
                 dstPicture.linesize);
    }
    if (inFormatContext->oformat->flags & AVFMT_RAWPICTURE)
    {
        // Raw video case - directly store the picture in the packet
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.flags        |= AV_PKT_FLAG_KEY;
        pkt.stream_index  = stream->index;
        pkt.data          = dstPicture.data[0];
        pkt.size          = sizeof(AVPicture);
        ret = av_interleaved_write_frame(inFormatContext, &pkt);
    }
    else
    {
        // encode the image
        AVPacket pkt;
        int gotOutput;
        av_init_packet(&pkt);
        pkt.data = NULL;    // packet data will be allocated by the encoder
        pkt.size = 0;
        ret = avcodec_encode_video2(c, &pkt, frame, &gotOutput);
        if (ret < 0)
        {
            //fprintf(stderr, "Error encoding video frame: %s\n", av_err2str(ret));
            qDebug() << "Error encoding video frame";
            return false;
        }
        // If size is zero, it means the image was buffered.
        if (gotOutput)
        {
            if (c->coded_frame->key_frame)
                pkt.flags |= AV_PKT_FLAG_KEY;
            pkt.stream_index = stream->index;
            // Write the compressed frame to the media file.
            ret = av_interleaved_write_frame(inFormatContext, &pkt);
        }
        else
        {
            ret = 0;
        }
    }
    if (ret != 0)
    {
       // fprintf(stderr, "Error while writing video frame: %s\n", av_err2str(ret));
        qDebug() << "Error while writing video frame";
        return false;
    }
    frameCount++;
    return true;
}

////////////////////////////////////////////////////////////////////////////////
//  QVideoOutput::closeMediaFile
//!
//! @brief Closes media file
//!
////////////////////////////////////////////////////////////////////////////////
bool QVideoOutput::closeMediaFile()
{
    av_free(srcPicture.data[0]);
    // Write the trailer, if any. The trailer must be written before you
    // close the CodecContexts open when you wrote the header; otherwise
    // av_write_trailer() may try to use memory that was freed on
    // av_codec_close().
    av_write_trailer(formatContext);
    // Close each codec.
    if (videoStream)
        closeVideo(videoStream);
    if (swsContext)
    {
       sws_freeContext(swsContext);
       swsContext = 0x0;
    }
    // Free the streams.
    for (unsigned int i = 0; i < formatContext->nb_streams; i++)
    {
        av_freep(&formatContext->streams[i]->codec);
        av_freep(&formatContext->streams[i]);
    }
    if (!(outputFormat->flags & AVFMT_NOFILE))
    {
       // Close the output file.
       avio_close(formatContext->pb);
    }
    // free the stream
    av_free(formatContext);

    return true;
}
////////////////////////////////////////////////////////////////////////////////
//  QVideoOutput::closeVideo
//! @brief Closes video
//! @param[in]  stream :
////////////////////////////////////////////////////////////////////////////////
void QVideoOutput::closeVideo(AVStream *stream)
{
    avcodec_close(stream->codec);
    av_free(dstPicture.data[0]);
    av_free(frame);
}
