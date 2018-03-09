// BSPD.cpp: 定义 DLL 应用程序的导出函数。

#include "BSPD.h"

#if _WIN32||_WIN64
#include <Windows.h>
#endif




_DLLEXPORT int BSPDGetPCM(BSPDContext *bspdctx, char *rawdata) {
    return BSPD_ERRO_UNDEFINE;
}

_DLLEXPORT int BSPDGetRawDataWithTime(BSPDContext * bspdctx, char * ydata, char * udata, char * vdata, int64_t * pts, int64_t * duration)
{
    int ret = bc_get_raw(bspdctx);

    if (ret == 1)
    {
        memcpy(ydata, bspdctx->pCoder->pFrameYUV->data[0], bspdctx->ysize);
        memcpy(udata, bspdctx->pCoder->pFrameYUV->data[1], bspdctx->ysize/4);
        memcpy(vdata, bspdctx->pCoder->pFrameYUV->data[2], bspdctx->ysize/4);
        *pts = bspdctx->timeStamp;
        *duration = bspdctx->vDuration;
    }
    else if(ret == 2)
    {
        int asize = bspdctx->pCoder->pSize;
        memcpy(ydata, bspdctx->pCoder->pBuf, bspdctx->pCoder->pSize);
        udata[0] = (asize >> 24) & 0xff;
        udata[1] = (asize >> 16) & 0xff;
        udata[2] = (asize >> 8) & 0xff;
        udata[3] = (asize) & 0xff;
        *pts = bspdctx->timeStamp;
        *duration = bspdctx->vDuration;

    }
    return ret;
}

_DLLEXPORT BSPDContext* BSPDCreateCtx(){
	BSPDContext *ctx = NULL;
	ctx = (BSPDContext*)malloc(sizeof(BSPDContext));
	if (ctx == NULL)
	{
		return NULL;
	}
	memset(ctx, 0, sizeof(BSPDContext));
    ctx->freeMark = BSPD_FREE_MARK;
    ctx->closeMark = BSPD_CLOSE_MARK;
	ctx->pCoder = (BSPDCoder*)malloc(sizeof(BSPDCoder));
	memset(ctx->pCoder, 0, sizeof(BSPDCoder));
	ctx->logCallback = NULL;
    ctx->pCoder->start_clock = -1;
    ctx->pCoder->fAIndex = -1;
    ctx->pCoder->fVIndex = -1;
    for (size_t i = 0; i < MAX_MEDIATYPE_INDEX; i++)
    {
        ctx->pCoder->allMediaTypeIndex[i] = -1;
    }
#if _DEBUG
	ctx->pCoder->LOGLEVEL = BSPD_LOG_DEBUG;
#else
    ctx->pCoder->LOGLEVEL = BSPD_LOG_ERROR;
#endif

	return ctx;
}

_DLLEXPORT int  BSPDOpen(BSPDContext *ctx, char * input, char * options)
{
   int flags = BSPD_OP_OK;

    if (BSPDISNULL(ctx))
    {
        return BSPD_USE_NULL_ERROR;
    }

    ctx->bspd_hb_abort = 0;

    size_t slen = strlen(input);

    if (input == NULL && slen <1)
    {
        bc_log(ctx,BSPD_LOG_ERROR, "use error input \n");
        return ctx->opcode = BSPD_INPUT_ERROR;
    }

    bc_log(ctx,BSPD_LOG_DEBUG, "BSPDOpen input:%s \n",input);

    ctx->inputPath = (char*)malloc(slen + 1);
    if (ctx->inputPath == NULL)
    {
        bc_log(ctx, BSPD_LOG_ERROR, "no enough memory alloc ctx->inputpath \n");
        return ctx->opcode = BSPD_NO_MEMY;
    }

    ctx->inputPath[slen]= '\0';
    memcpy(ctx->inputPath, input, slen);

    flags |= bc_set_default_options(ctx);
    if (options)
    {
        slen = strlen(options);
        if (slen > 0)
        {
            ctx->options = (char*)malloc(slen + 1);
            if (ctx->options == NULL)
            {
                bc_log(ctx, BSPD_LOG_ERROR, "no enough memory alloc ctx->options \n");
                return ctx->opcode = BSPD_NO_MEMY;
            }
            ctx->options[slen] = '\0';
            memcpy(ctx->options, options, slen);
            flags |= bc_parse_options(ctx);
        }
    }

    flags |= bc_init_coder(ctx);

    return flags;
}

_DLLEXPORT int BSPDGetYUV(BSPDContext *bspdctx,char *ydata,char *udata,char *vdata)
{
    if (bspdctx == NULL)
    {
        return BSPD_USE_NULL_ERROR;
    }

    if (ydata == NULL || udata == NULL || vdata == NULL)
    {
        bc_log(bspdctx, BSPD_LOG_ERROR, "y u v data maybe null\n ");
        return BSPD_USE_NULL_ERROR;
    }

    if (bc_get_yuv(bspdctx) == BSPD_OP_OK)
    {

        memcpy(ydata, bspdctx->pCoder->pFrameYUV->data[0], bspdctx->ysize);
        memcpy(udata, bspdctx->pCoder->pFrameYUV->data[1], bspdctx->ysize / 4);
        memcpy(vdata, bspdctx->pCoder->pFrameYUV->data[2], bspdctx->ysize / 4);
        return BSPD_OP_OK;
    }
    return BSPD_ERRO_UNDEFINE;
}

extern "C" _DLLEXPORT int BSPDGetYUVWithTime(BSPDContext *bspdctx, char *ydata, char *udata, char *vdata, int64_t *vpts, int64_t *apts, int64_t *vduration, int64_t *aduration) {
    int retval = BSPDGetYUV(bspdctx, ydata, udata, vdata);
    if (retval == BSPD_OP_OK)
    {
        if (vpts != NULL)
        {
            *vpts = bspdctx->timeStamp;
        }
        if (apts != NULL)
        {
        }
        if (vduration != NULL)
        {
            *vduration = bspdctx->vFps;
        }
        if (aduration != NULL)
        {
        }
    }
    return retval;
}

_DLLEXPORT int BSPDClose(BSPDContext * bspdctx)
{
    if (bspdctx == NULL || bspdctx->closeMark != BSPD_CLOSE_MARK || bspdctx->freeMark != BSPD_FREE_MARK )
    {
        return BSPD_USE_NULL_ERROR;
    }

	bc_log(bspdctx, BSPD_LOG_DEBUG, "BSPDClose\n");
    bc_close(bspdctx);
    if (bspdctx->inputPath)
    {
        free(bspdctx->inputPath);
    }
    if (bspdctx->options)
    {
        free(bspdctx->options);
    }

    if (bspdctx->pCoder)
    {
        free(bspdctx->pCoder);
        bspdctx->pCoder = NULL;
    }

    if (bspdctx)
    {
        memset(bspdctx,0,sizeof(BSPDContext));
        free(bspdctx);
        bspdctx = NULL;
    }

	return BSPD_ERRO_UNDEFINE;
}


_DLLEXPORT int BSPDSetLogCallback(BSPDContext * bspdctx,BSPDLogCallback call)
{
	if (bspdctx == NULL||bspdctx->pCoder == NULL )
	{
		return BSPD_USE_NULL_ERROR;
	}
	bspdctx->logCallback = call;
	return 0;
}


_DLLEXPORT BSPDPacketData* BSPDCreatePacket(BSPDContext *bspdctx,int *opcode) {
    if (BSPDISNULL(bspdctx) || opcode == NULL)
    {
        return NULL;
    }

    BSPDPacketData *ctx = (BSPDPacketData*)malloc(sizeof(BSPDPacketData));
    if (ctx==NULL)
    {
        bc_log(bspdctx, BSPD_LOG_ERROR, "NO MEM malloc bspdPackt");
        *opcode = -1;
        return ctx;
    }
    ctx->pkt = av_packet_alloc();
    if (ctx->pkt == NULL)
    {
        bc_log(bspdctx, BSPD_LOG_ERROR, "NO MEM malloc bspdpacketdata->avpacket");
        *opcode = -1;
    }
    *opcode = 0;

    return ctx;

}

_DLLEXPORT int BSPDGetPacket(BSPDContext *bspdctx, BSPDPacketData *pkt) {
    if (BSPDISNULL(bspdctx))
    {
        return BSPD_USE_NULL_ERROR;
    }

    return bc_get_packet(bspdctx, pkt);
}


_DLLEXPORT int BSPDAbort(BSPDContext *ctx){
    if (ctx == NULL)
    {
        return BSPD_USE_NULL_ERROR;
    }
    ctx->bspd_hb_abort = 0xff00ff;
    return BSPD_OP_OK;
}

#if __ANDROID_NDK__
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved){
    av_set_java_vm_flags = -1;
    av_set_java_vm_flags = av_jni_set_java_vm(vm,NULL);
    if(av_set_java_vm_flags == 0){
        av_set_java_vm_flags = BSPD_CLOSE_MARK;
    }
    return JNI_VERSION_1_4;
}
#endif
