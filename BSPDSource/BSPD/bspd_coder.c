#include "bspd_coder.h"
#include <time.h>
#include <sys\timeb.h>

#define MAX_PRINT_LEN 2048

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

int bc_parse_options(BSPDContext *ctx)
{
	return BSPD_ERRO_UNDEFINE;
}

int bc_set_default_options(BSPDContext *ctx) {
	return BSPD_ERRO_UNDEFINE;
}

int bc_init_coder(BSPDContext *ctx) {

	bc_log(ctx, BSPD_LOG_DEBUG, "bc init coder\n");

	if (ctx == NULL || ctx->pCoder == NULL)
	{
		return BSPD_USE_NULL_ERROR;
	}
	ctx->pCoder->initDone = -1;

	av_register_all();
	avformat_network_init();
	if (ctx->pCoder->pFormatCtx = avformat_alloc_context())
	{
		bc_log(ctx, BSPD_LOG_ERROR, "avformat alloc context error\n");
		return BSPD_AVLIB_ERROR;
	}

	if (avformat_open_input(&ctx->pCoder->pFormatCtx,ctx->inputPath,NULL,NULL)!=0)
	{
		bc_log(ctx, BSPD_LOG_ERROR, "avformat open input error\n");
		return BSPD_AVLIB_ERROR;
	}

	ctx->pCoder->fVIndex = -1;

	for (size_t i = 0; i < ctx->pCoder->pFormatCtx->nb_streams; i++)
	{
		if (ctx->pCoder->pFormatCtx->streams[ctx->pCoder->fVIndex]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			bc_log(ctx, BSPD_LOG_DEBUG, "get video stream index at :%d", ctx->pCoder->fVIndex);
			ctx->pCoder->fVIndex = i;
			break;
		}
	}
	if (ctx->pCoder->fVIndex == -1)
	{
		bc_log(ctx, BSPD_LOG_ERROR, "cant find video stream\n");
		return BSPD_AVLIB_ERROR;
	}

	ctx->pCoder->pCodecCtx = ctx->pCoder->pFormatCtx->streams[ctx->pCoder->fVIndex]->codec;
	ctx->pCoder->pCodec = avcodec_find_decoder(ctx->pCoder->pCodecCtx->codec_id);

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

	ctx->pCoder->pBuf = (unsigned char *)av_malloc(av_image_get_buffer_size(
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

	ctx->pCoder->imgSwsCtx = sws_getContext(ctx->pCoder->pCodecCtx->width,
		ctx->pCoder->pCodecCtx->height, ctx->pCoder->pCodecCtx->pix_fmt,
		ctx->pCoder->pCodecCtx->width, ctx->pCoder->pCodecCtx->height,
		AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

	if (ctx->pCoder->imgSwsCtx == NULL)
	{
		bc_log(ctx, BSPD_LOG_ERROR, "create imgswsctx error \n");
		return BSPD_AVLIB_ERROR;
	}

	ctx->pCoder->packet = (AVPacket*)malloc(sizeof(AVPacket));
	if (ctx->pCoder->packet == NULL)
	{
		bc_log(ctx, BSPD_LOG_ERROR, "malloc pcoder packet error \n");
		return BSPD_NO_MEMY;
	}
	
	ctx->pCoder->initDone = 1;
	bc_log(ctx, BSPD_LOG_DEBUG, "init bspd coder done\n");

	return BSPD_ERRO_UNDEFINE;
}

int bc_get_yuv(BSPDContext *ctx) {
	return BSPD_ERRO_UNDEFINE;
}

int bc_close(BSPDContext *ctx) {
	return BSPD_ERRO_UNDEFINE;
}


__inline char * timeString() {
	time_t t;
	time(&t);
	clock_t cput;
	cput = clock();
	struct tm * timeinfo = localtime(&t);
	static char timeStr[30];
	sprintf(timeStr, "[%.2d-%.2d %.2d:%.2d:%.2d] [ptime: %ld]", timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, cput );
	return timeStr;
}

int bc_log(BSPDContext *ctx,int LEVEL,const char *fmt, ...) 
{
	if (LEVEL < ctx->pCoder->LOGLEVEL)
	{
		return BSPD_OP_OK;
	}

	if (ctx == NULL || ctx->pCoder == NULL)
	{
		return BSPD_USE_NULL_ERROR;
	}

	char str[MAX_PRINT_LEN] = "";

	char *timestr = timeString();
	size_t tstrLen = strlen(timestr);
	tstrLen = tstrLen > MAX_PRINT_LEN - tstrLen ? MAX_PRINT_LEN - tstrLen : tstrLen;
	memcpy(str, timestr, tstrLen);
	str[tstrLen] = '\t';

	va_list args;
	va_start(args, fmt);
	int len;
	len = vsnprintf(str+tstrLen+1, MAX_PRINT_LEN - 1, fmt, args);
	va_end(args);

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