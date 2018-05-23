# BSPD

a sample lib, decode use [FFmpeg](http://ffmpeg.org/)

理论上来讲只要FFmpeg支持的媒体文件这个库都能使用

### 这个库怎么工作的
container->sample->h264->yuv420p

### 如何使用这个库
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

所有的编译前提是 你需要完成FFmpeg在当前平台的编译，库的解码器支持也是对应你的FFmpeg编译时的选项

### windows build
默认使用VS2017来进行编译

### *nix 
在类nix系统下，我们需要Cmake 来编译

修改 CMakeLists.txt 里的 include和link路径，或者拷贝FFmpeg的include和libs文件到项目下的ThirdPartyLibs/FFmpeg/路径

```shell
$ mkdir build
$ cd build
$ cmake ..
$ make
```
### android
在项目的Android目录下有AndroidStudio的项目例子

### ios
拷贝FFmpeg的include和libs文件到项目下的ThirdPartyLibs/FFmpeg/路径

使用xcode打开IOS/BSPDSample 编译可以得到一个简单应用


