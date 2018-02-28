#include"BSPD.h"

void test(){
	printf("test in\n");
	BSPDContext *ctx = BSPDCreateCtx();

	BSPDOpen(ctx,"http:192.168.3.69/vod/css.mkv","");

	char *y,*u,*v;
	int64_t t = 0;

	int ysize = ctx->ysize;
	y = (char*)malloc(ysize);
	u = (char*)malloc(ysize/4);
	v = (char*)malloc(ysize/4);

	int i = 0;
	while(i<5){
		BSPDGetYUVWithTime(ctx,y,u,v,&t,&t,&t,&t);
		i++;
	}

	BSPDClose(ctx);
	ctx = NULL;

	BSPDGetYUVWithTime(ctx,y,u,v,&t,&t,&t,&t);
	BSPDOpen(ctx,"rtmp://live.hkstv.hk.lxdns.com/live/hks","");
	BSPDGetYUVWithTime(ctx,y,u,v,&t,&t,&t,&t);
	BSPDOpen(ctx,"rtmp://live.hkstv.hk.lxdns.com/live/hks","");
	BSPDClose(ctx);

	free(y);
	free(u);
	free(v);

	printf("test out\n");
}

int main(){
	printf("start \n");
	int i = 0;
	test();
	return 0;
}

