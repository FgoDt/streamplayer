#ifndef __BSPD_H__
#define __BSPD_H__

#define _DLLEXPORT _declspec(dllexport)

extern "C" {
#include "bspd_coder.h"
}

typedef struct {
	int num;
	int x;
	int y;
	char * path;
}MCtx;



extern "C" _DLLEXPORT void BSPDTest();


/**
 *
 */
extern "C" _DLLEXPORT BSPDContext* BSPDCreateCtx();

/**
 * BSPD ������ڣ�ʹ��BSPDӦ���ȴ�����
 */
extern "C" _DLLEXPORT int BSPDOpen(BSPDContext *ctx ,char *input, char *options);

/**
 * ���BSPD�򿪳ɹ� ����ʹ��getyuv�õ�yuv����
 */
extern "C" _DLLEXPORT int BSPDGetYUV(BSPDContext *bspdctx,char *ydata,char *udata,char *vdata);

extern "C" _DLLEXPORT int BSPDGetYUVWithTime(BSPDContext *bspdctx, char *ydata, char *udata, char *vdata, int64_t *vpts, int64_t *apts, int64_t *vduration, int64_t *aduration);

extern "C" _DLLEXPORT int BSPDGetPCM(BSPDContext *bspdctx, char *rawdata);

/**
 * �ر�BSPD
 */
extern "C" _DLLEXPORT int BSPDClose(BSPDContext *bspdctx);

/**
 * ����log�ص�
 */
extern "C" _DLLEXPORT int BSPDSetLogCallback(BSPDContext *bspdctx,BSPDLogCallback call);








#endif // ! __BSPD_H__
