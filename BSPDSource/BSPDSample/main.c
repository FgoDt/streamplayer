#pragma comment (lib, "glfw3dll.lib")
#define _CRT_SECURE_NO_WARNINGS 1
#include<Windows.h>
#include<stdlib.h>
#include<stdio.h>
#include<glad/glad.h>
#include<glfw3.h>
#include<sys/stat.h>


typedef void* (*BSPDCreateCtx)();
typedef int(*BSPDOpen)(void *ctx, char* input, char *options);
typedef int(*BSPDGetYUV)(void *ctx, char *ydata, char *udata, char *vdata, long *vpts, long *apts, long *vduration, long *aduration);
typedef int(*BSPDGetDecWH)(void *ctx, int *w, int *h);
typedef int(*BSPDGetAudioCfg)(void *ctx, int *sr, int *ch);
typedef int(*BSPDGetRaw) (void *bspdctx, char *ydata, char *udata, char *vdata, int64_t *pts, int64_t *duration);


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

static const char* vertex_shader_text =
"attribute vec4 vCol;\n"
"attribute vec4 vPos;\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = vPos;\n"
"     color = vCol;\n"
"}\n";

static const char* fragment_shader_text =
"precision lowp float; \n"
"varying vec3 color;\n"
"uniform sampler2D tex_y; \n"
"uniform sampler2D tex_u; \n"
"uniform sampler2D tex_v; \n"
"void main()\n"
"{\n"
" vec3 yuv; vec3 rgb; \n"
" yuv.x = texture2D(tex_y,color).r; \n"
" yuv.y = texture2D(tex_u,color).r - 0.5; \n"
" yuv.z = texture2D(tex_v,color).r - 0.5; \n"
" rgb = mat3( 1,     1,        1,       \n"
"             0,     -0.39465, 2.03211, \n"
"           1.13983, -0.58060, 0) * yuv; \n"
"    gl_FragColor = vec4(rgb , 1.0);\n"
"}\n";

static float va[8] = { -.8f, -.8f,.8f,-.8f,-.8f,.8f,.8f,.8f };
static float vt[9] = {
    1.0f,1.f,1.f ,
    1.f,0.7f,0.f ,
    0.f,0.f,0.8f
};

static float vt2[] = { 0.0f,1.0f,1.0f,1.0f,0.f,0.f,1.f,0.f };


char *read_shader_file(char * path) {

    FILE *p = fopen(path, "r");
    unsigned long fileSize = -1;
    if (p == NULL)
    {
        return NULL;
    }
    struct _stat stat;
    if (_stat(path,&stat)!=0)
    {
        fclose(p);
        return NULL;
    }
    fileSize = stat.st_size;
    char * ret = (char*)malloc(fileSize + 1);
    ret[fileSize] = '\0';

    fread(ret, 1, fileSize, p);
    fclose(p);

    return ret;

}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int getpcmlen(char *udata) {
    int len = 0;
    len += (uint8_t)(udata[0]) << 24;
    len += (uint8_t)(udata[1]) << 16;
    len += (uint8_t)(udata[2]) << 8;
    len +=(uint8_t)udata[3];
    return len;
}

float i = -1.5f;
int main(void)
{
    LPCSTR dllname = "BSPD.dll";

    HMODULE hdll = LoadLibraryA(dllname);

    if (hdll == NULL)
    {
        return 1;
    }
        BSPDCreateCtx createfunc = (BSPDCreateCtx)GetProcAddress(hdll, "BSPDCreateCtx");
        BSPDOpen openfunc = (BSPDOpen)GetProcAddress(hdll, "BSPDOpen");
        BSPDGetYUV getyuvfunc = (BSPDGetYUV)GetProcAddress(hdll, "BSPDGetYUVWithTime");
        BSPDGetDecWH getdecwh = (BSPDGetDecWH)GetProcAddress(hdll, "BSPDGetDecWH");
        BSPDGetAudioCfg getaudiocfg = (BSPDGetAudioCfg)GetProcAddress(hdll, "BSPDGetAudioCfg");
        BSPDGetRaw getraw = (BSPDGetRaw)GetProcAddress(hdll, "BSPDGetRawDataWithTime");


        void* ctx = createfunc();
        int f = openfunc(ctx, (char*)"f:\sp.mkv", (char*)"-d -ha");
        FILE *testpcm = fopen("F:\\test.pcm", "wb");
        int w = 120;
        int h = 180;

        getdecwh(ctx, &w, &h);
  

        char* ydata = (char*)malloc(w * h);
        char* udata = (char*)malloc(w * h/4 );
        char* vdata = (char*)malloc(w * h/4);
  
    GLFWwindow* window;
    GLuint vertex_shader, fragment_shader, program;
    GLint  vpos_location, vcol_location;

    GLint tex_y, tex_u, tex_v;//uniform
    GLuint id_y, id_u, id_v;//texture

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);


    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);


    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    char *vsstr = read_shader_file("./vs.glsl");
    glShaderSource(vertex_shader, 1, &vsstr, NULL);
    glCompileShader(vertex_shader);
    free(vsstr);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    char* fsstr = read_shader_file("./fs.glsl");
    glShaderSource(fragment_shader, 1, &fsstr, NULL);
    glCompileShader(fragment_shader);
    free(fsstr);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");

    tex_y = glGetUniformLocation(program, "tex_y");
    tex_u = glGetUniformLocation(program, "tex_u");
    tex_v = glGetUniformLocation(program, "tex_v");

    glEnableVertexAttribArray(vpos_location);

    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, 0, va);
    glEnableVertexAttribArray(vcol_location);

    glVertexAttribPointer(vcol_location, 2, GL_FLOAT, GL_FALSE, 0, vt2);


    //create texture
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



    long t;
    while (!glfwWindowShouldClose(window))
    {
      //  f = getyuvfunc(ctx, ydata, udata, vdata,&t,&t,&t,&t);
        f = getraw(ctx, ydata, udata, vdata, &t, &t);
        if (f == 2)
        {
            fwrite(ydata, 1, getpcmlen(udata), testpcm);
        }
        if (f != 1)
        {
            continue;
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, id_y);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, ydata);
        glUniform1i(tex_y, 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, id_u);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, w/2, h/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, udata);
        glUniform1i(tex_u, 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, id_v);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, w/2, h/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, vdata);
        glUniform1i(tex_v,2) ;

        float ratio;
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float)height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        // mat4x4_rotate_Z(m, m, (float) glfwGetTime());

        glUseProgram(program);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);
        Sleep(2);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}