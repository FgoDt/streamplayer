#include "bspd_coder.h"
#include <time.h>
#include <libavutil/time.h>
#include <libavutil/hwcontext.h>
#define MAX_PRINT_LEN 2048

//#include "FDCore.h"
#if  0
#include "bspd_mutex.h"
#include "bspd_cond.h"

typedef struct BSPDPacketList {
    AVPacket pkt;
    struct BSPDPacketList *next;
    int serial;
}BSPDPacketList;

typedef struct PacketQueue {
    BSPDPacketList *first_pkt, *last_pkt;
    int nb_packets;
    int size;
    int64_t duration;
    int abort_request;
    int serial;
    BSPDMutex *mutex;
    BSPDCond *cond;
}PacketQueue;


int bspd_packet_queue_put_private(PacketQueue *q, AVPacket *pkt) {
    BSPDPacketList *pkt1;
    if (q->abort_request)
    {
        return -1;
    }

    pkt1 = av_malloc(sizeof(BSPDPacketList));
    if (!pkt1)
    {
        return -1;
    }

    pkt1->pkt = *pkt;
    pkt1->next = NULL;
    /*  if (pkt == &fl)
      {

      }*/
}

int bspd_init_queue(BSPDFrameQueue *q,int max_size) {
    if (q==NULL)
    {
        return BSPD_USE_NULL_ERROR;
    }
    memset(q, 0, sizeof(BSPDFrameQueue));

    q->mutex = bspd_create_mutex();
    if (q->mutex == NULL)
    {
        return BSPD_USE_NULL_ERROR;
    }

    if (!(q->cond = bspd_create_cond()))
    {
        return BSPD_NO_MEMY;
    }


    q->max_size = max_size;
    for (int i = 0; i < q->max_size; i++)
    {
        if (!(q->queue[i].pYuvData = av_frame_alloc())) {
            return BSPD_NO_MEMY;
        }
    }

    return BSPD_OP_OK;
}

int bspd_queue_destory(BSPDFrameQueue *q) {
    int i;
    for ( i = 0; i < q->max_size; i++)
    {
        BSPDFrameData* fd = &q->queue[i];
        av_frame_unref(fd->pYuvData);
        av_frame_free(&fd->pYuvData);
    }
    bspd_destroy_mutex(q->mutex);
    bspd_destroy_cond(q->cond);
    return BSPD_OP_OK;
}

void bspd_queue_signal(BSPDFrameQueue *q)
{
    bspd_lock_mutex(q->mutex);
    bspd_cond_signal(q->cond);
    bspd_unlock_mutex(q->mutex);
}



BSPDFrameData* bspd_queue_peek(BSPDFrameQueue *q) {
    return &q->queue[(q->rindex + q->rindex_show) % q->max_size];
}

BSPDFrameData* bspd_queue_peek_next(BSPDFrameQueue *q) {
    return &q->queue[(q->rindex + q->rindex_show+1) % q->max_size];
}

BSPDFrameData * bspd_queue_peek_last(BSPDFrameQueue *q) {
    return &q->queue[q->rindex];
}

BSPDFrameData* bspd_queue_peek_w(BSPDFrameQueue *q) {
    bspd_lock_mutex(q->mutex);
    while (q->size>= q->max_size)
    {
        bspd_cond_wait(q->cond, q->mutex);
    }
    bspd_unlock_mutex(q->mutex);

    return &q->queue[q->windex];
}

void bspd_queue_push(BSPDFrameQueue *q) {
    if (++ q->windex == q->max_size)
    {
        q->windex = 0;
    }
    bspd_lock_mutex(q->mutex);
    q->size++;
    bspd_cond_signal(q->cond);
    bspd_unlock_mutex(q->mutex);
}

void bspd_queue_next(BSPDFrameQueue *q) {
    av_frame_unref(q->queue[q->rindex].pYuvData);
    av_frame_free(&q->queue[q->rindex].pYuvData);
    if (++q->rindex == q->max_size)
    {
        q->rindex = 0;
    }

    bspd_lock_mutex(q->mutex);
    q->size--;
    bspd_cond_signal(q->cond);
    bspd_unlock_mutex(q->mutex);
}

int bspd_queue_nb_remaining(BSPDFrameQueue *q) {
    return q->size - q->rindex_show;
}
#endif

enum AVPixelFormat find_fmt_by_hw_type(const enum AVHWDeviceType type) {
    enum AVPixelFormat fmt;

    switch (type)
    {
    case AV_HWDEVICE_TYPE_VAAPI:
        fmt = AV_PIX_FMT_VAAPI;
        break;
    case AV_HWDEVICE_TYPE_CUDA:
        fmt = AV_PIX_FMT_CUDA;
        break;
    case AV_HWDEVICE_TYPE_D3D11VA:
        fmt = AV_PIX_FMT_D3D11;
        break;
    case AV_HWDEVICE_TYPE_DXVA2:
        fmt = AV_PIX_FMT_DXVA2_VLD;
        break;
    case AV_HWDEVICE_TYPE_VIDEOTOOLBOX:
        fmt = AV_PIX_FMT_VIDEOTOOLBOX;
        break;
    default:
        fmt = AV_PIX_FMT_NONE;
        break;
    }

    return fmt;
}

int hw_decoder_init(BSPDContext *ctx, const enum AVHWDeviceType type) {
    if (BSPDISNULL(ctx))
    {
        return BSPD_USE_NULL_ERROR;
    }
    int err = 0;
    if ((err = av_hwdevice_ctx_create( &ctx->pCoder->hwBufCtx,type,NULL,NULL,0))<0)
    {
        bc_log(ctx, AV_LOG_ERROR, "create hw device error");
        return err;
    }
    ctx->pCoder->pCodecCtx->hw_device_ctx = av_buffer_ref(ctx->pCoder->hwBufCtx);
    return err;
}

enum AVPixelFormat get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts) {
   /* if (BSPDISNULL(ctx))
    {
        return AV_PIX_FMT_NONE;
    }*/
    const enum AVPixelFormat *p;
    for ( p = pix_fmts; *p != -1; p++)
    {
        if (*p == ctx->pix_fmt )
        {
            return *p;
        }
    }
    return AV_PIX_FMT_NONE;
}

int decodec_interrupt_cb(void *ctx){
    if (((BSPDContext*)ctx)->bspd_hb_abort == 0xff00ff){
        return  1;
    }
    if(((BSPDContext*)ctx)->hb == -1 ){
        return 0;
        int64_t cur = av_gettime();
        if (cur - ((BSPDContext*)ctx)->pCoder->start_clock > ((BSPDContext*)ctx)->pCoder->timeout*1000) {
            ((BSPDContext*)ctx)->pCoder->istimeout = 0x1;
            return 1;
        }
        else
        {
            return 0;
        }
    } else{
        if(((BSPDContext*)ctx)->hb > 200){
            return 1;
        } else{
            ((BSPDContext*)ctx)->hb++;
            return 0;
        }
    }
}

void setval(BSPDContext *ctx) {
    if (ctx == NULL ||ctx->pCoder == NULL||ctx->pCoder->initDone<0)
    {
        bc_log(ctx, BSPD_AVLIB_ERROR, "set val error\n");
        return;
    }

    ctx->width = ctx->pCoder->pCodecCtx->width;
    ctx->height = ctx->pCoder->pCodecCtx->height;

    ctx->codedWidth = ctx->pCoder->pCodecCtx->coded_width;
    ctx->codedHeight= ctx->pCoder->pCodecCtx->coded_height;

    ctx->ysize = ctx->width*ctx->height;
    ctx->vFps = ctx->pCoder->pCodecCtx->framerate.num;
}

// bspd option "dhc ... # xxx xxx xxx "
// use #split bspd option and ffmpeg option
int bc_parse_options(BSPDContext *ctx)
{

    if (BSPDISNULL(ctx))
    {
        return BSPD_USE_NULL_ERROR;
    }

    if (ctx->options == NULL || strlen(ctx->options)==0)
    {
        return BSPD_USE_NULL_ERROR;
    }

   char* index = strchr(ctx->options, '#');

   const char *bspoption = NULL;
   char *ffoption = NULL;

   //no ffmpeg option
   if (index == NULL)
   {
       bspoption = ctx->options;
       int stl = strlen(bspoption);
       char argv[100][100] = { 0 };
       char temp[100] = { 0 };
       int i = 0;
       int j = 0;
       for(;bspoption[0] !='\0' || j != 0 ;bspoption++,stl--)
       {
           if (stl<0)
           {
               break;
           }
           if (bspoption[0] == ' ' || bspoption[0] == '\0')
           {
               temp[j] = '\0';
               j++;
               memcpy(argv[i], temp, j);

               i++;
               j=0;
           }
           else
           {
               temp[j] = bspoption[0];
               j++;
           }
       }
       j = 0;
       char *arg;
       while (j<i)
       {
           arg = argv[j];
           if (0==strcmp(arg,"-d"))
           {
               ctx->pCoder->LOGLEVEL = AV_LOG_DEBUG;
               j++;
               continue;
           }
           else if(0==strcmp(arg,"-hw"))
           {
               ctx->pCoder->useHW = 1;
               j++;
               continue;
           }
           else if(0 == strcmp(arg,"-ha"))
           {
               ctx->pCoder->hasAudio = 1;
               j++;
               continue;
           }
           else if (0==strcmp(arg, "-psize"))
           {
               j++;
               ctx->pCoder->pSize = atoi(argv[j]);
               j++;
               continue;
           }
           else if( 0 == strcmp(arg,"-timeout"))
           {
               j++;
               ctx->pCoder->timeout = atoi(argv[j]);
               j++;
               continue;
           }
           else if (0 == strcmp(arg, "-ch")) {
               j++;
               ctx->pCoder->channles = atoi(argv[j]);
               j++;
               continue;
           }
           else if(0 == strcmp(arg,"-sr"))
           {
               j++;
               ctx->pCoder->sampleRate = atoi(argv[j]);
               j++;
               continue;
           }
           else 
           {
               j++;
           }

       }
   }
   //has ffmpeg option
   else
   {
       
   }

    return BSPD_OP_OK;
}

int bc_set_default_options(BSPDContext *ctx) {
    if (BSPDISNULL(ctx))
    {
        return BSPD_USE_NULL_ERROR;
    }
    ctx->pCoder->LOGLEVEL = AV_LOG_ERROR;
    ctx->pCoder->useHW = 0;
    ctx->pCoder->pSize = 1024000;
    ctx->pCoder->timeout = 15000;
    return BSPD_OP_OK;
}

int bc_init_coder(BSPDContext *ctx) {



    bc_log(ctx, BSPD_LOG_DEBUG, "bc init coder\n");

    if (ctx == NULL || ctx->pCoder == NULL)
    {
        return BSPD_USE_NULL_ERROR;
    }
    ctx->pCoder->initDone = -1;

   // ctx->pCoder->fdSCtx = NULL;
    /////
   // ctx->pCoder->fdcCtx = create_fd_ctx();
   // fd_connect("192.168.3.69",1025,ctx->pCoder->fdcCtx);
   // ////
   // ctx->pCoder->fdSCtx = create_fd_ctx();
   // fd_connect("192.168.3.69",1027,ctx->pCoder->fdSCtx);


    av_register_all();
    avformat_network_init();
    if (!(ctx->pCoder->pFormatCtx = avformat_alloc_context()))
    {
        bc_log(ctx, BSPD_LOG_ERROR, "avformat alloc context error\n");
        return BSPD_AVLIB_ERROR;
    }
    ctx->hb = -1;
    ctx->pCoder->pFormatCtx->interrupt_callback.callback = decodec_interrupt_cb;
    ctx->pCoder->pFormatCtx->interrupt_callback.opaque = ctx;

    //// fix me use options
    ctx->pCoder->pFormatCtx->probesize = ctx->pCoder->pSize;
    /////

    //ctx->hb = 0;
    if (avformat_open_input(&ctx->pCoder->pFormatCtx,ctx->inputPath,NULL,NULL)!=0)
    {
        if (ctx->pCoder->istimeout == 0x1)
        {
            bc_log(ctx, BSPD_LOG_ERROR, "open input timeout");
            return BSPD_TIMEOUT;
        }
        bc_log(ctx, BSPD_LOG_ERROR, "avformat open input error\n");
        return BSPD_AVLIB_ERROR;
    }
    if (ctx->bspd_hb_abort == 0xff00ff)
    {
        return BSPD_AVLIB_ERROR;
    }
    ctx->hb = -2;

    if (avformat_find_stream_info(ctx->pCoder->pFormatCtx,NULL)<0)
    {
        bc_log(ctx, BSPD_LOG_ERROR, "avformat find stream info error\n");
        return BSPD_AVLIB_ERROR;
    }

    ctx->pCoder->fVIndex = -1;
    ctx->pCoder->fAIndex = -1;

    for (int i = 0; i < (int)ctx->pCoder->pFormatCtx->nb_streams&&i<MAX_MEDIATYPE_INDEX; i++)
    {
        if (ctx->pCoder->pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            bc_log(ctx, BSPD_LOG_DEBUG, "get video stream index at :%d\n", i);
            ctx->pCoder->fVIndex = i;
        }
        if (ctx->pCoder->pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            bc_log(ctx, BSPD_LOG_DEBUG, "get audio stream index at :%d\n", i);
            ctx->pCoder->fAIndex = i;
        }
        ctx->pCoder->allMediaTypeIndex[i] = ctx->pCoder->pFormatCtx->streams[i]->codec->codec_type;

    }
    if (ctx->pCoder->fVIndex == -1)
    {
        bc_log(ctx, BSPD_LOG_ERROR, "cant find video stream\n");
        return BSPD_AVLIB_ERROR;
    }

    if (ctx->pCoder->fAIndex == -1)
    {
        bc_log(ctx, BSPD_LOG_DEBUG, "no audio find\n");
    }

    //init audio codec
    if (ctx->pCoder->hasAudio)
    {
        ctx->pCoder->pACodecCtx = avcodec_alloc_context3(NULL);
        int ret = avcodec_parameters_to_context(ctx->pCoder->pACodecCtx, ctx->pCoder->pFormatCtx->streams[ctx->pCoder->fAIndex]->codecpar);
        if (ret<0)
        {
            bc_log(ctx, BSPD_LOG_ERROR, "audio codec ctx parse error\n");
            return BSPD_AVLIB_ERROR;
        }
        ctx->pCoder->pACodec = avcodec_find_decoder(ctx->pCoder->pACodecCtx->codec_id);
        if (ctx->pCoder->pACodec == NULL)
        {
            bc_log(ctx, BSPD_LOG_ERROR, "no audio codec found!");
            return BSPD_NO_CODEC_FOUND;
        }
    }


    //init vido codec
    ctx->pCoder->pCodecCtx = avcodec_alloc_context3(NULL);
    //ctx->pCoder->pCodecCtx = ctx->pCoder->pFormatCtx->streams[ctx->pCoder->fVIndex]->codec;
    int ret = avcodec_parameters_to_context(ctx->pCoder->pCodecCtx, ctx->pCoder->pFormatCtx->streams[ctx->pCoder->fVIndex]->codecpar);
    if (ret<0)
    {
        bc_log(ctx, BSPD_LOG_ERROR, "codec ctx parse error\n");
        return BSPD_AVLIB_ERROR;
    }
    ctx->pCoder->pCodec = avcodec_find_decoder(ctx->pCoder->pCodecCtx->codec_id);
    if(ctx->pCoder->pCodec == NULL){
        bc_log(ctx,BSPD_LOG_ERROR,"no codec found!");
        return BSPD_NO_CODEC_FOUND;
    }
#if __ANDROID_NDK__
    if(av_set_java_vm_flags == NULL) {
        AVCodec *MCCodec = NULL;
        MCCodec = avcodec_find_decoder_by_name("h264_mediacodec");
        if (MCCodec!= NULL) {
           // bc_log(ctx,BSPD_LOG_ERROR,"HAS MCCODEC");
            ctx->pCoder->pCodec = MCCodec;
            ctx->pCoder->pCodecCtx->codec_id = ctx->pCoder->pCodec->id;
        }
    }
#endif
    enum AVHWDeviceType hwType ;

    hwType = AV_HWDEVICE_TYPE_NONE;
    if (ctx->pCoder->useHW)
    {
#if _WIN32||_WIN64
        hwType = av_hwdevice_find_type_by_name("dxva2");
#endif
#if TARGET_OS_IPHONE
        hwType = av_hwdevice_find_type_by_name("videotoolbox");
#endif
        if (hwType != AV_HWDEVICE_TYPE_NONE)
        {
            ctx->pCoder->hwPixFmt = find_fmt_by_hw_type(hwType);
        }
        if (ctx->pCoder->hwPixFmt != -1)
        {
            ctx->pCoder->pCodecCtx->pix_fmt = ctx->pCoder->hwPixFmt;
            ctx->pCoder->pCodecCtx->get_format = get_hw_format;
            ctx->pCoder->hwInitDone = !hw_decoder_init(ctx, hwType);
        }
    }

    if (ctx->pCoder->pCodec == NULL)
    {
        bc_log(ctx, BSPD_LOG_ERROR, "no codec found\n");
        return BSPD_AVLIB_ERROR;
    }

    if (avcodec_open2(ctx->pCoder->pCodecCtx,ctx->pCoder->pCodec,NULL)<0)
    {
        bc_log(ctx, BSPD_LOG_ERROR, "avcodec open error\n");
        return BSPD_AVLIB_ERROR;
    }

    if (ctx->pCoder->hasAudio == 1 && ctx->pCoder->pACodecCtx !=NULL
        && avcodec_open2(ctx->pCoder->pACodecCtx,ctx->pCoder->pACodec,NULL)<0)
    {
        bc_log(ctx, BSPD_LOG_ERROR, "audio codec open error\n");
        return BSPD_AVLIB_ERROR;
    }

    ctx->pCoder->pFrame = av_frame_alloc();
    if (ctx->pCoder->pFrame == NULL)
    {
        bc_log(ctx, BSPD_LOG_ERROR, "alloc frame error no mem \n");
        return BSPD_NO_MEMY;
    }

    ctx->pCoder->pFrameYUV = av_frame_alloc();

    if (ctx->pCoder->pFrameYUV == NULL)
    {
        bc_log(ctx, BSPD_LOG_ERROR, "alloc frameyuv error no mem \n");
        return BSPD_NO_MEMY;
    }

    ctx->pCoder->pBuf = (unsigned char *)malloc(av_image_get_buffer_size(
            AV_PIX_FMT_YUV420P, ctx->pCoder->pCodecCtx->width, ctx->pCoder->pCodecCtx->height, 1));

    if (ctx->pCoder->pBuf == NULL)
    {
        bc_log(ctx, BSPD_LOG_ERROR, "malloc pbuf error\n");
        return BSPD_NO_MEMY;
    }


    av_image_fill_arrays(ctx->pCoder->pFrameYUV->data,
                         ctx->pCoder->pFrameYUV->linesize, ctx->pCoder->pBuf,
                         AV_PIX_FMT_YUV420P, ctx->pCoder->pCodecCtx->width,
                         ctx->pCoder->pCodecCtx->height, 1);

    if (ctx->pCoder->useHW&&ctx->pCoder->hwInitDone)
    {
        //硬编码获得的纹理是nv12格式
        ctx->pCoder->imgSwsCtx = sws_getContext(ctx->pCoder->pCodecCtx->width,
            ctx->pCoder->pCodecCtx->height, AV_PIX_FMT_NV12,
            ctx->pCoder->pCodecCtx->width, ctx->pCoder->pCodecCtx->height,
            AV_PIX_FMT_YUV420P, SWS_POINT, NULL, NULL, NULL);
    }
    else
    {
        ctx->pCoder->imgSwsCtx = sws_getContext(ctx->pCoder->pCodecCtx->width,
            ctx->pCoder->pCodecCtx->height, ctx->pCoder->pCodecCtx->pix_fmt,
            ctx->pCoder->pCodecCtx->width, ctx->pCoder->pCodecCtx->height,
            AV_PIX_FMT_YUV420P, SWS_POINT, NULL, NULL, NULL);
    }

    if (ctx->pCoder->imgSwsCtx == NULL)
    {
        bc_log(ctx, BSPD_LOG_ERROR, "create imgswsctx error \n");
        return BSPD_AVLIB_ERROR;
    }

    if (ctx->pCoder->hasAudio)
    {
        int sr = 0, ch = 0;
        sr = ctx->pCoder->sampleRate;
        ch = ctx->pCoder->channles;
        if (ch == 0)
        {
            ch = ctx->pCoder->pACodecCtx->channels;
            ctx->pCoder->channles = ch;
        }
        if (sr == 0)
        {
            sr = ctx->pCoder->pACodecCtx->sample_rate;
        }
        int och = av_get_default_channel_layout(ch);
        int ich = av_get_default_channel_layout(ctx->pCoder->pACodecCtx->channels);
        ctx->pCoder->pcmSwrCtx = swr_alloc();
        //unity3d pcm is flt fmt
        ctx->pCoder->pcmSwrCtx = swr_alloc_set_opts(ctx->pCoder->pcmSwrCtx, och,
            AV_SAMPLE_FMT_FLT, sr, ich,
            ctx->pCoder->pACodecCtx->sample_fmt, ctx->pCoder->pACodecCtx->sample_rate,
            0, NULL);
       int err = swr_init(ctx->pCoder->pcmSwrCtx);
       if (err != 0)
       {
           return BSPD_AVLIB_ERROR;
       }
    }
    ctx->pCoder->packet = av_packet_alloc();
    if (ctx->pCoder->packet == NULL)
    {
        bc_log(ctx, BSPD_LOG_ERROR, "malloc pcoder packet error \n");
        return BSPD_NO_MEMY;
    }

    ctx->pCoder->initDone = 1;
    bc_log(ctx, BSPD_LOG_DEBUG, "init bspd coder done\n");

    setval(ctx);

    return BSPD_OP_OK;
}

int bc_get_yuv(BSPDContext *ctx) {
    if (ctx == NULL||
        ctx->freeMark != BSPD_FREE_MARK ||
        ctx->closeMark != BSPD_CLOSE_MARK)
    {
        return BSPD_USE_NULL_ERROR;
    }

    if (ctx->pCoder->pFormatCtx == NULL ||
        ctx->pCoder->pCodec == NULL ||
        ctx->pCoder->pCodecCtx == NULL ||
        ctx->pCoder->packet == NULL)
    {
        bc_log(ctx, BSPD_LOG_ERROR, "formatctx pcodec codecctx packet maybe null\n");
        return BSPD_USE_NULL_ERROR;
    }
    int ret;

    int retval;
    while (1)
    {
        ctx->hb = 0;
       // bc_log(ctx,BSPD_LOG_DEBUG,"in readframe");
        if (av_read_frame(ctx->pCoder->pFormatCtx, ctx->pCoder->packet) < 0)
        {
            bc_log(ctx, BSPD_LOG_ERROR, "av read frame error \n");
            retval = BSPD_ERRO_UNDEFINE;
            break;
        }
      //  bc_log(ctx,BSPD_LOG_DEBUG,"in stream index");
        if (ctx->pCoder->packet->stream_index != ctx->pCoder->fVIndex)
        {
            av_packet_unref(ctx->pCoder->packet);
            continue;
        }

       // bc_log(ctx,BSPD_LOG_DEBUG,"in decodec video2");
       // ret = avcodec_decode_video2(ctx->pCoder->pCodecCtx, ctx->pCoder->pFrame, &got, ctx->pCoder->packet);
          ret = avcodec_send_packet(ctx->pCoder->pCodecCtx, ctx->pCoder->packet);
       // bc_log(ctx,BSPD_LOG_DEBUG,"out decode video2 and got:%d",got);
        if (ret < 0)
        {
            bc_log(ctx, BSPD_LOG_ERROR, "decode video2 error ret is %d\n", ret);
            retval = BSPD_ERRO_UNDEFINE;
            break;
        }


        ret = avcodec_receive_frame(ctx->pCoder->pCodecCtx, ctx->pCoder->pFrame);

        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            av_packet_unref(ctx->pCoder->packet);
            continue;
        }
       
            //bc_log(ctx,BSPD_LOG_ERROR,"pix_fmt %d, format %d",ctx->pCoder->pCodecCtx->pix_fmt,ctx->pCoder->pFrame->format);
            if(ctx->pCoder->pCodecCtx->pix_fmt!= ctx->pCoder->pFrame->format)
            {
                bc_log(ctx,BSPD_LOG_ERROR,"pix_fmt %d, format %d",ctx->pCoder->pCodecCtx->pix_fmt,ctx->pCoder->pFrame->format);
            }

            if (ctx->pCoder->hwInitDone)
            {
                if (ctx->pCoder->phwImgFrame == NULL)
                {
                    ctx->pCoder->phwImgFrame = av_frame_alloc();
                }
                if ((ret = av_hwframe_transfer_data(ctx->pCoder->phwImgFrame,ctx->pCoder->pFrame,0))<0)
                {
                    bc_log(ctx, BSPD_LOG_ERROR, "hw transfer data error");
                    return BSPD_AVLIB_ERROR;
                }

                sws_scale(ctx->pCoder->imgSwsCtx, ctx->pCoder->phwImgFrame->data,
                    ctx->pCoder->phwImgFrame->linesize, 0, ctx->pCoder->pCodecCtx->height,
                    ctx->pCoder->pFrameYUV->data, ctx->pCoder->pFrameYUV->linesize);
                ctx->ysize = ctx->pCoder->pCodecCtx->height * ctx->pCoder->pCodecCtx->width;
            }
            else
            {
                sws_scale(ctx->pCoder->imgSwsCtx, ctx->pCoder->pFrame->data,
                    ctx->pCoder->pFrame->linesize, 0, ctx->pCoder->pCodecCtx->height,
                    ctx->pCoder->pFrameYUV->data, ctx->pCoder->pFrameYUV->linesize);
                ctx->ysize = ctx->pCoder->pCodecCtx->height * ctx->pCoder->pCodecCtx->width;
            }
            if (!strcmp(ctx->pCoder->pFormatCtx->iformat->name,"mov,mp4,m4a,3gp,3g2,mj2"))
            {
                ctx->timeStamp = av_q2d(ctx->pCoder->pCodecCtx->time_base)*ctx->pCoder->packet->pts;
            }
            else
            {
                ctx->timeStamp = ctx->pCoder->pFrame->pts;
            }
            //ctx->vDuration = av_q2d((AVRational) { ctx->pCoder->pFrame->nb_samples, ctx->pCoder->pFrame->sample_rate });
            ctx->vDuration = ctx->pCoder->pFrame->pkt_duration;
           // fd_stream(ctx->pCoder->fdSCtx,ctx->pCoder->pFrameYUV->data[0],ctx->ysize);
           // fd_stream(ctx->pCoder->fdSCtx,ctx->pCoder->pFrameYUV->data[1],ctx->ysize/4);
           // fd_stream(ctx->pCoder->fdSCtx,ctx->pCoder->pFrameYUV->data[2],ctx->ysize/4);
            av_packet_unref(ctx->pCoder->packet);
            return BSPD_OP_OK;

        av_packet_unref(ctx->pCoder->packet);

    }
    av_packet_unref(ctx->pCoder->packet);

    return BSPD_ERRO_UNDEFINE;
}

int bc_decode_audio(BSPDContext *ctx) {
    if (BSPDISNULL(ctx))
    {
        return BSPD_ERRO_UNDEFINE;
    }
    int ret = avcodec_send_packet(ctx->pCoder->pACodecCtx, ctx->pCoder->packet);

    if (ret<0)
    {
        bc_log(ctx, BSPD_LOG_ERROR, "send audio packet error code: %d\n", ret);
        return BSPD_AVLIB_ERROR;
    }

    ret = avcodec_receive_frame(ctx->pCoder->pACodecCtx, ctx->pCoder->pFrame);

    av_packet_unref(ctx->pCoder->packet);

    return ret;
}

int bc_decode_video(BSPDContext *ctx) {
    int ret = avcodec_send_packet(ctx->pCoder->pCodecCtx, ctx->pCoder->packet);

    if (ret<0)
    {
        bc_log(ctx, BSPD_LOG_ERROR, "send video packet error code: %d\n",ret);
        return BSPD_AVLIB_ERROR;
    }

    ret = avcodec_receive_frame(ctx->pCoder->pCodecCtx, ctx->pCoder->pFrame);


    av_packet_unref(ctx->pCoder->packet);

    return ret;
}

int bc_sws_pic(BSPDContext *ctx) {
    if (BSPDISNULL(ctx))
    {
        return BSPD_USE_NULL_ERROR;
    }

    int ret = 0;
    if (ctx->pCoder->pCodecCtx->pix_fmt != ctx->pCoder->pFrame->format)
    {
        bc_log(ctx, BSPD_LOG_ERROR, "pix_fmt %d, format %d", ctx->pCoder->pCodecCtx->pix_fmt, ctx->pCoder->pFrame->format);
    }

    if (ctx->pCoder->hwInitDone)
    {
        if (ctx->pCoder->phwImgFrame == NULL)
        {
            ctx->pCoder->phwImgFrame = av_frame_alloc();
        }
        if ((ret = av_hwframe_transfer_data(ctx->pCoder->phwImgFrame, ctx->pCoder->pFrame, 0)) < 0)
        {
            bc_log(ctx, BSPD_LOG_ERROR, "hw transfer data error");
            return BSPD_AVLIB_ERROR;
        }

        sws_scale(ctx->pCoder->imgSwsCtx, ctx->pCoder->phwImgFrame->data,
            ctx->pCoder->phwImgFrame->linesize, 0, ctx->pCoder->pCodecCtx->height,
            ctx->pCoder->pFrameYUV->data, ctx->pCoder->pFrameYUV->linesize);
        ctx->ysize = ctx->pCoder->pCodecCtx->height * ctx->pCoder->pCodecCtx->width;
    }
    else
    {
        sws_scale(ctx->pCoder->imgSwsCtx, ctx->pCoder->pFrame->data,
            ctx->pCoder->pFrame->linesize, 0, ctx->pCoder->pCodecCtx->height,
            ctx->pCoder->pFrameYUV->data, ctx->pCoder->pFrameYUV->linesize);
        ctx->ysize = ctx->pCoder->pCodecCtx->height * ctx->pCoder->pCodecCtx->width;
    }
    if (!strcmp(ctx->pCoder->pFormatCtx->iformat->name, "mov,mp4,m4a,3gp,3g2,mj2"))
    {
        ctx->timeStamp = av_q2d(ctx->pCoder->pCodecCtx->time_base)*ctx->pCoder->packet->pts;
    }
    else
    {
        ctx->timeStamp = ctx->pCoder->pFrame->pts;
    }
    ctx->vDuration = ctx->pCoder->pFrame->pkt_duration;
    return BSPD_OP_OK;
}

int bc_swr_pcm(BSPDContext *ctx) {
    if (BSPDISNULL(ctx))
    {
        return BSPD_USE_NULL_ERROR;
    }

    if (ctx->pCoder->pcmSwrCtx != NULL)
    {
        if (ctx->pCoder->pBuf == NULL)
        {
            ctx->pCoder->pBuf = (unsigned char*)malloc(102400);
        }
        int ret = swr_convert(ctx->pCoder->pcmSwrCtx, &ctx->pCoder->pBuf,
            ctx->pCoder->pFrame->nb_samples,
            (const uint8_t**)ctx->pCoder->pFrame->extended_data, ctx->pCoder->pFrame->nb_samples);

        if (ret>0)
        {
            int flag = 0;
            if (ctx->pCoder->sampleRate != 0)
            {
                flag = 1;
            }
            ctx->pCoder->pSize = av_samples_get_buffer_size(NULL, ctx->pCoder->channles, ret, AV_SAMPLE_FMT_FLT, flag);
            ctx->timeStamp = ctx->pCoder->pFrame->pkt_pts;
            ctx->vDuration = ctx->pCoder->pFrame->pkt_duration;
            return BSPD_OP_OK;
        }
        
    }
    return BSPD_AVLIB_ERROR;
}

int bc_get_raw(BSPDContext * ctx)
{
    if (BSPDISNULL(ctx))
    {
        return BSPD_USE_NULL_ERROR;
    }

    if (ctx->pCoder->pFormatCtx == NULL ||
        ctx->pCoder->pCodec == NULL ||
        ctx->pCoder->pCodecCtx == NULL ||
        ctx->pCoder->packet == NULL)
    {
        bc_log(ctx, BSPD_LOG_ERROR, "formatctx pcodec codecctx packet maybe null\n");
        return BSPD_USE_NULL_ERROR;
    }
    int ret;

    int retval;
    while (1)
    {
        ctx->hb = 0;
        int flag = 0;
        if (av_read_frame(ctx->pCoder->pFormatCtx, ctx->pCoder->packet) < 0)
        {
            bc_log(ctx, BSPD_LOG_ERROR, "av read frame error \n");
            retval = BSPD_ERRO_UNDEFINE;
            break;
        }
        int index = -1;
        index = ctx->pCoder->packet->stream_index;
        if (ctx->pCoder->packet->stream_index == ctx->pCoder->fAIndex
            && ctx->pCoder->hasAudio)
        {
            flag = bc_decode_audio(ctx);
        }
        else if (ctx->pCoder->packet->stream_index == ctx->pCoder->fVIndex)
        {
            flag = bc_decode_video(ctx);
        }
        else
        {
            av_packet_unref(ctx->pCoder->packet);
        }

        if (flag == AVERROR(EAGAIN))
        {
            continue;
        }

        if (index == ctx->pCoder->fAIndex&&ctx->pCoder->hasAudio)
        {
            ret = bc_swr_pcm(ctx);
            if (ret == BSPD_OP_OK)
            {
                return 2;
            }
        }
        else if (index == ctx->pCoder->fVIndex)
        {
            ret = bc_sws_pic(ctx);
            if (ret == BSPD_OP_OK)
            {
                return 1;
            }
        }


    }
    return BSPD_ERRO_UNDEFINE;
}

int bc_close(BSPDContext *ctx) {
    ctx->hb = 300;

    if (ctx == NULL || ctx->pCoder == NULL)
    {
        return BSPD_USE_NULL_ERROR;
    }


    if (ctx->pCoder->pFrame)
    {
        av_frame_unref(ctx->pCoder->pFrame);
        av_frame_free(&ctx->pCoder->pFrame);
    }

    if (ctx->pCoder->pFrameYUV)
    {
        av_frame_unref(ctx->pCoder->pFrameYUV);
        av_frame_free(&ctx->pCoder->pFrameYUV);
    }
    if (ctx->pCoder->packet)
    {
        av_packet_unref(ctx->pCoder->packet);
        av_packet_free(&ctx->pCoder->packet);
    }


    if (ctx->pCoder->imgSwsCtx)
    {
        sws_freeContext(ctx->pCoder->imgSwsCtx);
    }

    if (ctx->pCoder->phwImgFrame)
    {
        av_frame_unref(ctx->pCoder->phwImgFrame);
        av_frame_free(&ctx->pCoder->phwImgFrame);
    }


    if (ctx->pCoder->pCodecCtx)
    {
        avcodec_close(ctx->pCoder->pCodecCtx);
        avcodec_free_context(&ctx->pCoder->pCodecCtx);

    }

    if (ctx->pCoder->pACodecCtx)
    {
        avcodec_close(ctx->pCoder->pACodecCtx);
        avcodec_free_context(&ctx->pCoder->pACodecCtx);
    }

    if (ctx->pCoder->pFormatCtx)
    {
        avformat_close_input(&ctx->pCoder->pFormatCtx);
        avformat_free_context(ctx->pCoder->pFormatCtx);
    }


    if (ctx->pCoder->pBuf)
    {
        free(ctx->pCoder->pBuf);
    }


    return BSPD_OP_OK;
}


__inline static char * timeString(int64_t *start_clock) {
    time_t t;
    time(&t);
    int64_t cput;
    cput = av_gettime();
    if (start_clock != NULL && *start_clock == -1)
    {
        *start_clock = cput;
    }
    cput = cput - *start_clock;
    cput /= 1000;
    struct tm * timeinfo = localtime(&t);
    static char timeStr[100];
    //av_gettime_relative();
    sprintf(timeStr, "[%.2d-%.2d %.2d:%.2d:%.2d] [ptime: %I64d]", timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, cput );
    return timeStr;
}

int bc_log(BSPDContext *ctx,int LEVEL,const char *fmt, ...)
{
   // return  0;
    if (ctx == NULL)
    {
        return BSPD_USE_NULL_ERROR;
    }
    if (LEVEL < ctx->pCoder->LOGLEVEL)
    {
        return BSPD_OP_OK;
    }

    if (ctx == NULL || ctx->pCoder == NULL)
    {
        return BSPD_USE_NULL_ERROR;
    }

    char str[MAX_PRINT_LEN] = "";

    char *timestr = timeString(&ctx->pCoder->start_clock);
    size_t tstrLen = strlen(timestr);
    tstrLen = tstrLen > MAX_PRINT_LEN - tstrLen ? MAX_PRINT_LEN - tstrLen : tstrLen;
    memcpy(str, timestr, tstrLen);
    str[tstrLen] = '\t';

    va_list args;
    va_start(args, fmt);
    int len;
    len = vsnprintf(str+tstrLen+1, MAX_PRINT_LEN - 1, fmt, args);
    va_end(args);
   // fd_log(ctx->pCoder->fdcCtx,str);

    if (ctx->logCallback!=NULL)
    {
        ctx->logCallback(str);
    }
    else
    {
        return BSPD_USE_NULL_ERROR;
    }

    return BSPD_OP_OK;
}

int bc_get_AV_packet(BSPDContext *ctx, BSPDPacketData *p) {
    int flags = -1;
    do
    {
        flags = bc_get_packet(ctx, p);
        if (p->pktType == AVMEDIA_TYPE_AUDIO &&p->pktType == AVMEDIA_TYPE_VIDEO)
        {
            break;
        }
    } while (flags == BSPD_OP_OK);
    return flags;
}

int bc_get_packet(BSPDContext *ctx, BSPDPacketData *pkt) {
    if (BSPDISNULL(ctx)||BSPPKTISNULL(pkt))
    {
        return BSPD_USE_NULL_ERROR;
    }

    if (ctx->pCoder->pFormatCtx == NULL ||
        ctx->pCoder->pCodec == NULL ||
        ctx->pCoder->pCodecCtx == NULL ||
        ctx->pCoder->packet == NULL)
    {
        bc_log(ctx, BSPD_LOG_ERROR, "formatctx pcodec codecctx packet maybe null\n");
        return BSPD_USE_NULL_ERROR;
    }

    int retval;

    av_packet_unref(pkt->pkt);

    if (av_read_frame(ctx->pCoder->pFormatCtx, pkt->pkt) < 0)
    {
        bc_log(ctx, BSPD_LOG_ERROR, "av read frame error \n");
        retval = BSPD_ERRO_UNDEFINE;
    }
    if (ctx->pCoder->fAIndex == pkt->pkt->stream_index)
    {
        pkt->pktType = AVMEDIA_TYPE_AUDIO;
    }
    else if(ctx->pCoder->fVIndex == pkt->pkt->stream_index)
    {
        pkt->pktType = AVMEDIA_TYPE_VIDEO;
    }
    else
    {
        pkt->pktType = AVMEDIA_TYPE_UNKNOWN;
    }

    return BSPD_OP_OK;
}

int bc_decode_audio_packet(BSPDContext *ctx, BSPDPacketData *p) {
    int sflags, rflags;
    sflags =  avcodec_send_packet(ctx->pCoder->pCodecCtx, p->pkt);
}
int bc_decode_video_packet(BSPDContext *ctx, BSPDPacketData *p) {
    return BSPD_AVLIB_ERROR;
}

int bc_test() {
    BSPDFrameQueue *q = malloc(sizeof(BSPDFrameQueue));
    return BSPD_OP_OK;
}