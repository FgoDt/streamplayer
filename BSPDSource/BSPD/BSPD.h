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

/**
 * �ر�BSPD
 */
extern "C" _DLLEXPORT int BSPDClose(BSPDContext *bspdctx);

/**
 * ����log�ص�
 */
extern "C" _DLLEXPORT int BSPDSetLogCallback(BSPDContext *bspdctx,BSPDLogCallback call);








#endif // ! __BSPD_H__
