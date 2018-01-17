# BSPD

a sample lib, decode use [FFmpeg](http://ffmpeg.org/)

❗ now only support decode video 

video container can be any

### how it work
container->sample->h264->yuv420p

### how to use
```c
    BSPDContext *ctx = BSPDCreateCtx();

    BSPDOpen(ctx,"http:192.168.3.69/vod/cs.mkv","");

    char *y,*u,*v;
    int64_t t = 0;
    y = (char*)malloc(ctx->ysize);
    u = (char*)malloc(ctx->ysize/4);
    v = (char*)malloc(ctx->ysize/4);

    while(BSPDGetYUVWithTime(ctx,y,u,v,&t,&t,&t,&t) == 0{
        //do something
    }
    BSPDClose(ctx);

```
## build

### windows build
you need visual studio 2017 

### *nix 
you need cmake, ffmpeg lib

change CMakeLists.txt include/link path

```shell
$ mkdir build
$ cd build
$ cmake ..
$ make
```
### android

### ios




