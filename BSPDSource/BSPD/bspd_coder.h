#ifndef __BSPD_CODER_H__
#define __BSPD_CODER_H__

#if _WIN32|_WIN64
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#include <libavcodec\avcodec.h>
#include <libavformat\avformat.h>
#include <libavutil\imgutils.h>
#include <libswscale\swscale.h>



/**
 * BSPD OPTIONCODE define 
 */
#define BSPD_OP_OK				0
#define BSPD_OPEN_ERRO			1
#define BSPD_FIND_INFO_ERRO		2
#define BSPD_NO_VIDEOS_FOUND	4	
#define BSPD_NO_CODEC_FOUND		8
#define BSPD_OPEN_CODEC_ERRO	16
#define BSPD_NO_MEMY			0x20
#define BSPD_INPUT_ERROR		0x40	//bspdopen input 参数错误
#define BSPD_OPTIONS_ERROR		0x80	//bspdopen options 参数错误
#define BSPD_USE_NULL_ERROR		0x100
#define BSPD_AVLIB_ERROR		0x200
#define BSPD_ERRO_UNDEFINE		0x7fffffff //[0111 ... 1111]

/**
 * LOG LEVEL DEFINE
 */
#define BSPD_LOG_ERROR			1
#define BSPD_LOG_DEBUG			255

typedef void(__stdcall *BSPDLogCallback)(char *log);

typedef	struct {
	AVFormatContext *pFormatCtx;
	AVCodecContext	*pCodecCtx;
	AVCodec			*pCodec;
	AVFrame			*pFrame;
	AVFrame			*pFrameYUV;
	struct SwsContext *imgSwsCtx;
	AVPacket		*packet;
	AVDictionary	*optDic;
	unsigned char	*pBuf; //temp buf use to rw frame data
	int				LOGLEVEL;
	int				fVIndex;//first video stream index
	int				initDone;
}BSPDCoder;

 /**
 * BSPD context
 * 用于各个解码函数之间数据传递
 */
typedef	struct {

	BSPDCoder		*pCoder;

	//输入媒体url
	char			*inputPath;
	//打开输入的 options
	char			*options;
	//解码宽度
	int				codedWidth;
	//解码高度
	int				codedHeight;
	//视频宽度
	int				width;
	//视频高度
	int				height;
	//YUV中Ydata size
	int				ysize;
	//操作返回code
	int				opcode;
	//frame 时间戳
	int				timeStamp;
	//video fps
	int				vFps;
	//video duration 
	int				vDuration;

	BSPDLogCallback	logCallback;
}BSPDContext;


int bc_parse_options(BSPDContext *ctx);

int bc_set_default_options(BSPDContext *ctx);

int bc_init_coder(BSPDContext *ctx);

int bc_get_yuv(BSPDContext *ctx);

int bc_close(BSPDContext *ctx);

/**
 * bc log 
 * 使用bc log 来输出log 日志
 *
 * @param ctx BSPDContext 
 * @param LEVEL log 输出等级
 * @param fmt 日志输出格式
 * @param ... 参数
 */
int bc_log(BSPDContext *ctx,int LEVEL,const char * fmt, ...);

#endif // !__BSPD_CODER_H__