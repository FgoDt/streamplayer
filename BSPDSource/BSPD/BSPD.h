#ifndef __BSPD_H__
#define __BSPD_H__

#if _WIN32||_WIN64
#define _DLLEXPORT _declspec(dllexport)
#else
#define _DLLEXPORT 
#endif

extern "C"{

#include "bspd_coder.h"
#if __ANDROID_NDK__
#include <libavcodec/jni.h>
#endif

#include <time.h>
}


typedef struct {
	int num;
	int x;
	int y;
	char * path;
}MCtx;



extern "C" _DLLEXPORT void BSPDTest();


extern "C" _DLLEXPORT BSPDContext* BSPDCreateCtx();

extern "C" _DLLEXPORT int BSPDOpen(BSPDContext *ctx ,char *input, char *options);


extern "C" _DLLEXPORT int BSPDGetYUV(BSPDContext *bspdctx,char *ydata,char *udata,char *vdata);

extern "C" _DLLEXPORT int BSPDGetYUVWithTime(BSPDContext *bspdctx, char *ydata, char *udata, char *vdata, int64_t *vpts, int64_t *apts, int64_t *vduration, int64_t *aduration);

extern "C" _DLLEXPORT int BSPDGetPCM(BSPDContext *bspdctx, char *rawdata);


extern "C" _DLLEXPORT BSPDPacketData* BSPDCreatePacket(BSPDContext *bspdctx,int *opcode);

extern "C" _DLLEXPORT int BSPDGetPacket(BSPDContext *bspdctx,BSPDPacketData *pkt);

extern "C" _DLLEXPORT int BSPDDecodePacketV(BSPDContext *bspdctx,BSPDPacketData *pkt);

extern "C" _DLLEXPORT int BSPDDecodePacketA(BSPDContext *bspdctx,BSPDPacketData *pkt);

extern "C" _DLLEXPORT int BSPDFreePacket(BSPDContext *bspdctx,BSPDPacketData *pkt);

extern "C" _DLLEXPORT int BSPDClose(BSPDContext *bspdctx);

extern "C" _DLLEXPORT int BSPDSetLogCallback(BSPDContext *bspdctx,BSPDLogCallback call);

extern "C" _DLLEXPORT int BSPDAbort();

#endif // ! __BSPD_H__
