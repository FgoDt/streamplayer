//
//  ViewController.m
//  BSPDSample
//
//  Created by rt-zl on 2018/2/27.
//  Copyright © 2018年 fgodt. All rights reserved.
//

#import "ViewController.h"
#import "../../../BSPDSource/BSPD/BSPD.h"
#import <OpenGLES/ES2/gl.h>


#import <AVFoundation/AVFoundation.h>
#import <VideoToolbox/VideoToolbox.h>
#import <AudioToolbox/AudioToolbox.h>

#define ATTRIB_VERTEX 3
#define ATTRIB_TEXTURE 4

@interface ViewController (){
    EAGLContext *_EAGLContext;
    CAEAGLLayer *_CAEAGLLayer;
    GLuint  _colorRenderBuffer;
    GLuint  _frameBuffer;
    CADisplayLink *displayLink;
    BSPDContext *ctx;
    
    
    //opengles
    GLuint p;
    GLuint id_v, id_u, id_y;
    GLuint textureUniformY, textureUniformV, textureUniformU;
    Byte* plane[3];
}


@end


@implementation ViewController

- (void)viewDidLoad {
    
    textureUniformY= textureUniformV = textureUniformU = 255;
    
    plane[0]= (Byte*)malloc(32*32);
    
    plane[1]= (Byte*)malloc(32*32/4);
    plane[2]= (Byte*)malloc(32*32/4);
    
    ctx = BSPDCreateCtx();
    
    BSPDOpen(ctx,"rtmp://live.hkstv.hk.lxdns.com/live/hks"," -d");
    
    int ysize = ctx->ysize;
    plane[0] = (Byte*)malloc(ysize);
    plane[1] = (Byte*)malloc(ysize/4);
    plane[2] = (Byte*)malloc(ysize/4);
    
    
    [super viewDidLoad];
    [self test];
    
}
-(void)test{
    
    [self setupEAGL];
    [self setuprenderbuffer];
    [self setupframebuffer];
    
    // [_EaglContext presentRenderbuffer:GL_RENDERBUFFER];
    
    displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(mUpdate)];
    displayLink.frameInterval = 2;
    [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    
    [self CompShader];
}

int blue = 0;

const char vShaderStr[] ="\
attribute vec4 vertexIn; \n \
attribute vec2 textureIn; \n \
varying vec2 tc; \n \
void main()   \n \
{ gl_Position = vertexIn; tc = textureIn; } \0";

const char fShaderStr[] = "\
precision lowp float; \n \
uniform sampler2D tex_y; \n \
uniform sampler2D tex_u; \n \
uniform sampler2D tex_v; \n \
varying vec2 tc; \n \
void main() \n \
{ vec3 yuv; \
vec3 rgb; \
yuv.x = texture2D(tex_y, tc).r; \
yuv.y = texture2D(tex_u, tc).r-0.5;\
yuv.z = texture2D(tex_v, tc).r-0.5; \
rgb = mat3( 1,     1,        1, \
0,     -0.39465, 2.03211, \
1.13983, -0.58060, 0) *yuv; \
gl_FragColor = vec4(rgb, 1); \
} \0";

-(void)mUpdate{
    glViewport(0, 260, 320, 240);
    long t;
    int flag =  BSPDGetRawDataWithTime(ctx, plane[0], plane[1], plane[2], &t, &t);
    if (flag!=1) {
        return;
    }

    glClearColor(1.0f,1.0f,1.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id_y);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, ctx->width, ctx->height, 0,GL_LUMINANCE, GL_UNSIGNED_BYTE, plane[0]);
    glUniform1i(textureUniformY, 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, id_u);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, ctx->width/2, ctx->height/2, 0,GL_LUMINANCE, GL_UNSIGNED_BYTE, plane[1]);
    glUniform1i(textureUniformU, 1);
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, id_v);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, ctx->width/2, ctx->height/2, 0,GL_LUMINANCE, GL_UNSIGNED_BYTE, plane[2]);
    glUniform1i(textureUniformV, 2);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    
    glFlush();
    [_EAGLContext presentRenderbuffer:GL_RENDERBUFFER];
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}



-(void)setupEAGL{
    _CAEAGLLayer = [CAEAGLLayer layer];
    _CAEAGLLayer.frame = self.view.frame;
    _CAEAGLLayer.opaque = YES;
    
    _EAGLContext = [[EAGLContext alloc]initWithAPI:kEAGLRenderingAPIOpenGLES2];
    [EAGLContext setCurrentContext:_EAGLContext];
    
    _CAEAGLLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:NO],kEAGLDrawablePropertyRetainedBacking,kEAGLColorFormatRGBA8,kEAGLDrawablePropertyColorFormat, nil];
    [self.view.layer addSublayer:_CAEAGLLayer];
    // [self.view addSubview:_CaeglLayer];
}

-(void)setuprenderbuffer{
    if (_colorRenderBuffer) {
        
        glDeleteRenderbuffers(1,&_colorRenderBuffer);
        _colorRenderBuffer = 0;
    }
    // glGenRenderBuffers(1, &_colorRenderBuffer);
    glGenRenderbuffers(1, &_colorRenderBuffer );
    glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderBuffer);
    [_EAGLContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:_CAEAGLLayer];
}
-(void )setupframebuffer{
    if (_frameBuffer) {
        glDeleteFramebuffers(1, &_frameBuffer);
        _frameBuffer = 0;
    }
    glGenFramebuffers(1, &_frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _colorRenderBuffer);
}

/**
 * 编译shader
 */
-(void)CompShader{
    GLint vc, fc, link;
    GLint v, f;
    v = glCreateShader(GL_VERTEX_SHADER);
    f = glCreateShader(GL_FRAGMENT_SHADER);
    const char* temp = (char*)malloc(1024);
    memset(temp,0,1024);
    memcpy(temp, vShaderStr, strlen(vShaderStr));
    glShaderSource(v, 1, &temp, NULL);
    memset(temp,0,1024);
    memcpy(temp,fShaderStr, strlen(fShaderStr));
    glShaderSource(f, 1, &temp, NULL);
    free(temp);
    glCompileShader(v);
    glGetShaderiv(v, GL_COMPILE_STATUS, &vc);
    
    glCompileShader(f);
    glGetShaderiv(f, GL_COMPILE_STATUS, &fc);
    
    p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f );
    glBindAttribLocation(p,ATTRIB_VERTEX, "vertexIn");
    glBindAttribLocation(p, ATTRIB_TEXTURE, "textureIn");
    glLinkProgram(p);
    glGetProgramiv(p, GL_LINK_STATUS, &link);
    glUseProgram(p);
    
    textureUniformY = glGetUniformLocation(p, "tex_y");
    textureUniformU = glGetUniformLocation(p, "tex_u");
    textureUniformV = glGetUniformLocation(p, "tex_v");
    
    
    static const GLfloat vertexVertices[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f, 1.0f,
        1.0f, 1.0f,
    };
    static const GLfloat textureVerticces[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
    };
    glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, vertexVertices);
    
    glEnableVertexAttribArray(ATTRIB_VERTEX);
    glVertexAttribPointer(ATTRIB_TEXTURE,2,GL_FLOAT,0,0,textureVerticces);
    glEnableVertexAttribArray(ATTRIB_TEXTURE);
    
    glGenTextures(1, &id_y);
    glBindTexture(GL_TEXTURE_2D, id_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glGenTextures(1, &id_u);
    glBindTexture(GL_TEXTURE_2D, id_u);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glGenTextures(1, &id_v);
    glBindTexture(GL_TEXTURE_2D, id_v);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    
}



@end

