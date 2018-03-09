const vec2 ts = vec2(1280., 720.);  
const vec2 ms = vec2(28., 28.);

varying vec3 color;
uniform sampler2D tex_y; 
uniform sampler2D tex_u; 
uniform sampler2D tex_v; 

void main()
{
 vec3 yuv; vec3 rgb; 
 yuv.x = texture2D(tex_y,color).r; 
 yuv.y = texture2D(tex_u,color).r - 0.5; 
 yuv.z = texture2D(tex_v,color).r - 0.5; 
 rgb = mat3( 1,     1,        1,
            0,     -0.39465, 2.03211,  
            1.13983, -0.58060, 0) * yuv; 

 vec2 xy = vec2(color.x* ts.x, color.y *ts.y);

 vec2 xymsic = vec2(floor(xy.x/ms.x)*ms.x,
                    floor(xy.y/ms.y)*ms.y)+.5*ms;

                    vec2 delxy = xymsic - xy;
                    float dell = length(delxy);

                    vec2 uvmisc = vec2(xymsic.x/ts.x, xymsic.y/ts.y);

 if(dell<0.5*ms.x){
 yuv.x = texture2D(tex_y,uvmisc).r; 
 yuv.y = texture2D(tex_u,uvmisc).r - 0.5; 
 yuv.z = texture2D(tex_v,uvmisc).r - 0.5; 
 rgb = mat3( 1,     1,        1,
            0,     -0.39465, 2.03211,  
            1.13983, -0.58060, 0) * yuv; 
  gl_FragColor = vec4(rgb,1.);
 }
 else{
 gl_FragColor = vec4(rgb , 1.0);
 }
};