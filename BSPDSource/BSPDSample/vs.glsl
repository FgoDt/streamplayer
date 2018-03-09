attribute vec4 vCol;
attribute vec4 vPos;
varying vec3 color;
void main()
{
    gl_Position = vPos;
     color = vCol;
};