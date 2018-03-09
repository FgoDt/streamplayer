#ifndef __BSPD_H__
#define __BSPD_H__

#if _WIN32||_WIN64
#define _DLLEXPORT _declspec(dllexport)
#else
#define _DLLEXPORT 
#endif

#ifdef __cplusplus
extern "C"{
#endif
    
#include "bspd_coder.h"
#if __ANDROID_NDK__
#include <libavcodec/jni.h>
#endif

#include <time.h>
#ifdef __cplusplus
}
#endif


typedef struct {
	int num;
	int x;
	int y;
	char * path;
}MCtx;


#ifdef __cplusplus
extern "C"
#endif
_DLLEXPORT void BSPDTest();


/**
 * 创建解码实例
 **/
#ifdef __cplusplus
extern "C"
#endif
_DLLEXPORT BSPDContext* BSPDCreateCtx();

/**
 * 打开流
 * @param ctx the bsp ctx
 * @param input the media path can be http url rtmp url local path
 * @param option the bsp option 
 *          -d for debug
 *          -ha decode with audio
 *          -ch audio channels
 *          -sr audio sample rate
 **/
#ifdef __cplusplus
extern "C"
#endif
_DLLEXPORT int BSPDOpen(BSPDContext *ctx ,char *input, char *options);


/**
 * 获取解码后的YUV数据
 * 数据格式为YUV420p
 * @param bspdctx the bsp ctx
 * @param ydata get y data
 * @param udata get u data
 * @param vdata get v data
 **/
#ifdef __cplusplus
extern "C"
#endif
_DLLEXPORT int BSPDGetYUV(BSPDContext *bspdctx,char *ydata,char *udata,char *vdata);


/**
 * 获取解码后的YUV数据和时间
 * 数据格式为YUV420p
 * @param bspdctx the bsp ctx
 * @param ydata get y data
 * @param udata get u data
 * @param vdata get v data
 * @param vpts video pts
 * @param vduration video duration
 **/
#ifdef __cplusplus
extern "C"
#endif
_DLLEXPORT int BSPDGetYUVWithTime(BSPDContext *bspdctx, char *ydata, char *udata, char *vdata, int64_t *vpts, int64_t *apts, int64_t *vduration, int64_t *aduration);

attribute_deprecated
#ifdef __cplusplus
extern "C"
#endif
_DLLEXPORT int BSPDGetPCM(BSPDContext *bspdctx, char *rawdata);

/**
* 获取解码后的数据和时间
* video数据格式为YUV420p
* audio数据格式为pcm float32
* @param bspdctx the bsp ctx
* @param ydata 
*               if return is video ydata is y data
*               if return is audio ydata is pcm data
* @param udata
*               if return is vidoe udata is u data
*               if return is audio udata first 4 byte is audio pcm data size
* @param vdata get v data
* @param pts  is audio/video pts
* @param duration is audio/video duration
* return  1 for video data 
*         2 for audio data 
*         other for error 
**/
#ifdef __cplusplus
extern "C"
#endif
_DLLEXPORT int BSPDGetRawDataWithTime(BSPDContext *bspdctx, char *ydata, char *udata, char *vdata, int64_t *pts, int64_t *duration);

attribute_deprecated
#ifdef __cplusplus
extern "C"
#endif
_DLLEXPORT BSPDPacketData* BSPDCreatePacket(BSPDContext *bspdctx,int *opcode);

attribute_deprecated
#ifdef __cplusplus
extern "C"
#endif
_DLLEXPORT int BSPDGetPacket(BSPDContext *bspdctx,BSPDPacketData *pkt);

attribute_deprecated
#ifdef __cplusplus
extern "C"
#endif
_DLLEXPORT int BSPDDecodePacketV(BSPDContext *bspdctx,BSPDPacketData *pkt);

attribute_deprecated
#ifdef __cplusplus
extern "C"
#endif
_DLLEXPORT int BSPDDecodePacketA(BSPDContext *bspdctx,BSPDPacketData *pkt);

attribute_deprecated
#ifdef __cplusplus
extern "C"
#endif
_DLLEXPORT int BSPDFreePacket(BSPDContext *bspdctx,BSPDPacketData *pkt);

/**
 * 关闭解码器
 * 非线程安全的方法 使用前应该调用 BSPDAbort(BSPDContext *bspdctx)
 * @param bspdctx is ctx
 **/
#ifdef __cplusplus
extern "C"
#endif
_DLLEXPORT int BSPDClose(BSPDContext *bspdctx);

/**
 * 设置Log回调 通过回调可以自定义LOG输出
 * @param bspdctx bsp ctx
 * @param call callback func
 **/
#ifdef __cplusplus
extern "C"
#endif
_DLLEXPORT int BSPDSetLogCallback(BSPDContext *bspdctx,BSPDLogCallback call);

/**
 * 终止正在进行的解码操作
 * 有些操作会比较耗时 使用此方法可以快速终止操作
 * @param bspdctx is bsp ctx
 **/
#ifdef __cplusplus
extern "C"
#endif
_DLLEXPORT int BSPDAbort(BSPDContext *bspdctx);

/**
 * 获取视频流解码后的宽和高
 * @param bspdctx bspdctx is bsp ctx
 * @param w w is video width
 * #param h h is video height
 */
#ifdef __cplusplus
extern "C"
#endif
_DLLEXPORT int BSPDGetDecWH(BSPDContext *bspdctx, int *w, int *h);

#endif // ! __BSPD_H__
