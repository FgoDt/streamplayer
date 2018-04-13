#include <SDL.h>
#include <Windows.h>

SDL_Window *window = NULL;


#define HAVE_GETSYSTEMTIMEASFILETIME 1

#define INT64_C(x)   (x ## LL)

int64_t av_gettime(void)
{
#if HAVE_GETTIMEOFDAY
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;
#elif HAVE_GETSYSTEMTIMEASFILETIME
    FILETIME ft;
    int64_t t;
    GetSystemTimeAsFileTime(&ft);
    t = (int64_t)ft.dwHighDateTime << 32 | ft.dwLowDateTime;
    return t / 10 - 11644473600000000; /* Jan 1, 1601 */
#else
    return -1;
#endif
}


int64_t av_gettime_relative(void)
{
#if HAVE_CLOCK_GETTIME && defined(CLOCK_MONOTONIC)
#ifdef __APPLE__
    if (clock_gettime)
#endif
    {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return (int64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
    }
#endif
    return av_gettime() + 42 * 60 * 60 * INT64_C(1000000);
}

typedef enum btype {
    audio,
    video
}btype;

typedef struct bclock {
    int64_t start;
    int64_t lastupdate;
    double pts;
    double dur;
}bclock;

typedef struct bdata {
    byte *ydata;
    byte *udata;
    byte *vdata;
    int pts;
    int dur;
    int asize;//audio size;
}bdata;


#define QUEUE_SIZE 30
typedef struct bqueue {
    SDL_mutex *mutex;
    SDL_cond *cond;
    bdata *queue[QUEUE_SIZE];
    btype type;
    int nb;
    int size;
    int rindex;
    int windex;
}bqueue;



void init_bqueue(bqueue *q, btype type,int y) {
    q->type = type;

    q->mutex = SDL_CreateMutex();
    q->rindex = 0;
    q->windex = 0;
    q->nb = 0;
    q->size = 0;
    q->size = QUEUE_SIZE;

    for (int i = 0; i < QUEUE_SIZE; i++)
    {
        q->queue[i] = (bdata*)malloc(sizeof(bdata));
        bdata *data = q->queue[i];
        data->ydata = (byte*)malloc(y);
        data->udata = (byte*)malloc(y / 4);
        data->vdata = (byte*)malloc(y / 4);
    }

}

bdata* rpeek_bqueue(bqueue *q) {
    if (q->nb<1)
    {
        return NULL;
    }
    return q->queue[q->rindex];
}

bdata* wpeek_bqueue(bqueue*q) {
    if (q->nb+1>=q->size)
    {
        return NULL;
    }
    return q->queue[q->windex];
}

int push_bqueue(bqueue* q) {
    SDL_LockMutex(q->mutex);
    if (++q->windex>=q->size)
    {
        q->windex = 0;
    }
    q->nb++;
    SDL_UnlockMutex(q->mutex);
    return 1;
}

int pop_bqueue(bqueue* q) {
    SDL_LockMutex(q->mutex);
    if (++q->rindex>=q->size)
    {
        q->rindex = 0;
    }
    q->nb--;
    SDL_UnlockMutex(q->mutex);
}


#ifndef getpcmlenfunc
#define getpcmlenfunc

int getpcmlen(char *udata);
#endif // !getpcmlen





#ifndef bspdheader
#define bspdheader

typedef void* (*BSPDCreateCtx)();
typedef int(*BSPDOpen)(void *ctx, char* input, char *options);
typedef int(*BSPDGetYUV)(void *ctx, char *ydata, char *udata, char *vdata, long *vpts, long *apts, long *vduration, long *aduration);
typedef int(*BSPDGetDecWH)(void *ctx, int *w, int *h);
typedef int(*BSPDGetAudioCfg)(void *ctx, int *sr, int *ch);
typedef int(*BSPDGetAudioCfgp)(void *ctx, int *sr, int *ch,int *nb_sample, int *bytespersec);
typedef int(*BSPDGetRaw) (void *bspdctx, char *ydata, char *udata, char *vdata, int64_t *pts, int64_t *duration);
typedef int(*BSPDSeek)(void *ctx, int64_t t);
typedef int(*BSPDGetMediaInfo)(void *ctx, unsigned char *t);

#endif





/*############################################
#
#           logic start
#
##############################################*/

bqueue* aqueue;
bqueue* vqueue;
SDL_Renderer *render = NULL;
SDL_Texture *tex = NULL;
SDL_Texture *btex = NULL;
SDL_Rect rect;
SDL_Rect wrect;

bclock rclock;
bclock vclock;
bclock aclock;



void initclock(bclock *c) {
    c->start = av_gettime_relative() / 1000;
}

void initmediaclock(bclock *c, long pts) {
    c->start = pts;
}

void setclock(bclock *c, int dur, int pts) {
    c->dur = dur;
    c->pts = pts;
}

void updateclock(bclock *c, int time) {
    c->lastupdate = av_gettime_relative() / 1000;
}


int vneedupdate() {
   
    if (vclock.pts +vclock.dur<aclock.pts-aclock.dur)
    {
        //drop frame;
        return -1;
    }
    if (aclock.pts>=vclock.pts+vclock.dur)
    {
        return 1;
    }
    return 0;
}



void aneedupdate() {

}

void updateyuv(void *data) {

    while (1)
    {
        if (render == NULL || tex == NULL)
        {
            Sleep(3);
            continue;
        }

        bdata *data = rpeek_bqueue(vqueue);
        if (data == NULL)
        {
            Sleep(3);
            continue;
        }

        setclock(&vclock, data->dur, data->pts);

        int f = vneedupdate();

        if (f == 0)
        {
            goto nup;
        }
        if (f == -1)
        {
            //drop frame
            pop_bqueue(vqueue);
            printf("drop:%ld\n", data->pts);
            continue;
        }
        pop_bqueue(vqueue);

        //show buffer
        SDL_Rect vrect;
        vrect.x = 0;
        vrect.y = 0;
        vrect.w = ((double)(vqueue->nb) / vqueue->size) * wrect.w/3;
        vrect.h = 20;
        
        SDL_Rect arect;
        arect.x = 0;
        arect.y = 30;
        arect.w = ((double)(aqueue->nb) / aqueue->size) * wrect.w/3;
        arect.h = 20;


        SDL_SetRenderDrawColor(render, 0, 255, 100, 255);

        SDL_RenderDrawRect(render, &vrect);


        SDL_UpdateYUVTexture(tex, &rect, data->ydata, rect.w, data->udata, rect.w / 2, data->vdata, rect.w / 2);

        SDL_RenderClear(render);
        //printf("UPYUV:%ld\n", data->pts);

        wrect.h = rect.h / 2;
        wrect.w = rect.w / 2;
        SDL_RenderCopy(render, tex, NULL, &wrect);
        SDL_RenderCopy(render, btex, NULL, &vrect);
        SDL_RenderCopy(render, btex, NULL, &arect);
        SDL_RenderPresent(render);

        //

    nup:
        updateclock(&rclock, av_gettime_relative() / 1000);
        Sleep(13);

    }
}


int audio_r = 0;
int _ch, _sr, bytespersec; 
long  audiobytes;
long audio_call_time = 0;
void audio_call(void *udata, Uint8 *stream, int len) {
    int temp = len;
    while (len > 0)
    {
    retry:
        bdata *data = rpeek_bqueue(aqueue);
        int len1;
        if (data == NULL)
        {
            memset(stream, 0, len);
            byte *silenc = (byte*)malloc(len);
            memset(silenc, 0, len);
            SDL_MixAudio(stream, silenc, len, SDL_MIX_MAXVOLUME);
            free(silenc);
            goto retry;
        }


        int alen = data->asize - audio_r;
        len1 = len > alen ? alen : len;
        SDL_memset(stream, 0, len1);
        data->ydata += audio_r;
        SDL_MixAudio(stream, data->ydata, len1, SDL_MIX_MAXVOLUME);

        stream += len1;
        audio_r += len1;

        if (data->asize - audio_r <= 0)
        {
            audio_r = 0;
            pop_bqueue(aqueue);
        }
        //printf("UPaudio:%ld\n", data->pts);
        len -= len1;

    }
    audiobytes += temp;
    audio_call_time = ((double)audiobytes/bytespersec)*1000;

   setclock(&aclock, ((double)temp/bytespersec)*1000, audio_call_time);


}





void audio_main() {

    initclock(&rclock);
    initclock(&aclock);
    initclock(&vclock);


    HMODULE hdll = NULL;
    hdll = LoadLibraryA("BSPD.dll");

    BSPDCreateCtx createfunc = (BSPDCreateCtx)GetProcAddress(hdll, "BSPDCreateCtx");
    BSPDOpen openfunc = (BSPDOpen)GetProcAddress(hdll, "BSPDOpen");
    BSPDGetYUV getyuvfunc = (BSPDGetYUV)GetProcAddress(hdll, "BSPDGetYUVWithTime");
    BSPDGetDecWH getdecwh = (BSPDGetDecWH)GetProcAddress(hdll, "BSPDGetDecWH");
    BSPDGetAudioCfg getaudiocfg = (BSPDGetAudioCfg)GetProcAddress(hdll, "BSPDGetAudioCfg");
    BSPDGetAudioCfgp getaudiocfgp = (BSPDGetAudioCfgp)GetProcAddress(hdll, "BSPDGetAudioCfgPlus");
    BSPDGetRaw getraw = (BSPDGetRaw)GetProcAddress(hdll, "BSPDGetRawDataWithTime");
    BSPDSeek bseek = (BSPDSeek)GetProcAddress(hdll, "BSPDSeek");
    BSPDGetMediaInfo bgetinfo = (BSPDGetMediaInfo)GetProcAddress(hdll, "BSPDGetMediaInfo");


    void *ctx = createfunc();

    //sdl normal use 2 channels
   // char *input = "http://182.150.11.188/live-tx-hdl.huomaotv.cn/live/6fvPQT.flv";
    char *input = "http://live-dl-hdl.huomaotv.cn/hmtv/5fILSX?t=1523440252&r=470080342435&stream=5fILSX&rid=oubvc2y3v&token=61ae3eb6f82c5e30bf7026f20bc64d37";
//    char *input = "f:/sp.mkv";
    openfunc(ctx, input, "-d -ha   ");

    byte test = 0;
    bgetinfo(ctx, &test);

    int h, w;
    getdecwh(ctx, &w, &h);

    h = h <= 0 ? 600 : h;
    w = w <= 0 ? 800 : w;
    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    window = SDL_CreateWindow("BSPD_AUDIO_TEST", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w / 2, h / 2, SDL_WINDOW_OPENGL);

    render = SDL_CreateRenderer(window, -1, 0);
    tex = SDL_CreateTexture(render, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, w, h);

    btex = SDL_CreateTexture(render, SDL_PIXELFORMAT_ABGR32, SDL_TEXTUREACCESS_STREAMING, 20, 20);
    byte *data = (byte*)malloc(4 * 20 * 20);
    memset(data, 0, 4 * 20 * 20);
    for (size_t i = 0; i < 4 * 20 * 20; i += 4)
    {
        data[i] = 255;
        data[i + 2] = 255;
    }
    SDL_Rect brect = { 0,0,20,20 };
    SDL_UpdateTexture(btex, &brect, data, 20);

    int size = w * h;
    byte *ydata = malloc(size);
    byte *udata = malloc(size / 4);
    byte *vdata = malloc(size / 4);

    int flag;
    int64_t pts;
    int64_t dur;

    rect.w = w;
    rect.h = h;
    rect.x = 0;
    rect.y = 0;

    int ch, sr, nb_sample;
    getaudiocfgp(ctx, &sr, &ch, &nb_sample,&bytespersec);

    SDL_AudioSpec wanted_spec;
    wanted_spec.channels = ch;
    wanted_spec.format = AUDIO_F32;
    wanted_spec.freq = sr;
    wanted_spec.silence = 0;
    wanted_spec.samples = nb_sample;
    wanted_spec.callback = audio_call;

    _ch = ch;
    _sr = sr;


    SDL_AudioSpec out_spec;

    SDL_OpenAudio(&wanted_spec, &out_spec);
    SDL_PauseAudio(0);

    SDL_Event ev;

    aqueue = (bqueue*)malloc(sizeof(bqueue));
    vqueue = (bqueue*)malloc(sizeof(bqueue));

    init_bqueue(vqueue, video, w*h);
    init_bqueue(aqueue, audio, size);

    SDL_CreateThread(updateyuv, "", NULL);
    vclock.start = -1;
    aclock.start = -1;
    while (1)
    {
        while (SDL_PollEvent(&ev)>0)
        {
            switch (ev.type)
            {
            case SDL_QUIT:
                return;
                break;
            default:
                break;
            }
        }


        flag = getraw(ctx, ydata, udata, vdata, &pts, &dur);


        //video 
        if (flag == 1)
        {
                printf("vpts:%d\n", pts);
            //updateyuv(tex,render, ydata, udata, vdata,&rect);
            continue;

        rey:
            bdata *data = wpeek_bqueue(vqueue);
            if (data!=NULL)
            {
                memcpy(data->ydata, ydata, size);
                memcpy(data->udata, udata, size/4);
                memcpy(data->vdata, vdata, size/4);
                data->dur = dur;
                data->pts = pts;
                if (vclock.start == -1 )
                {
                    initmediaclock(&vclock, data->pts);
                }
                printf("vpts:%d\n", pts);
                push_bqueue(vqueue);
            }
            else
            {
                Sleep(3);
                goto rey;
            }
        }
        //audio 
        else if (flag == 2) {

                printf("Apts:%d\n", pts);
            continue;

        reya:
            bdata * data = wpeek_bqueue(aqueue);
            if (data!=NULL)
            {
                data->asize = getpcmlen(udata);
                memcpy(data->ydata, ydata, data->asize);
                data->dur = dur;
                data->pts = pts;
                if (aclock.start == -1)
                {
                    initmediaclock(&aclock, data->pts);
                }
                printf("Apts:%d\n", pts);
                push_bqueue(aqueue);
            }
            else
            {
                Sleep(3);
                goto reya;
            }

        }

    }


}