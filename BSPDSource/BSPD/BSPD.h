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







extern "C" _DLLEXPORT void  getctx(MCtx *ctx);
extern "C" _DLLEXPORT  MCtx* createctx();

typedef void(__stdcall *CTCallback)(int tick);

//extern "C" _declspec(dllexport) int open(char *input, char *options);

extern "C" void _DLLEXPORT setcallback(CTCallback callback);

extern "C" _DLLEXPORT long long dlltest();

#endif // ! __BSPD_H__
