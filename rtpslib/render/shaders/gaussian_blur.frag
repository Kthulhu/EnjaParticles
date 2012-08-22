#version 330 core
const float pi = 3.141592654;

uniform sampler2D depthTex;
uniform float del_x;
uniform float del_y;
//uniform float falloff;
uniform float sig;
const float maxDepth = 0.9999999f;
in vec2 texCoord;
out vec4 colorOut;

void main(void)
{
    float depth=texture2D(depthTex, texCoord.st).x;
    if(depth>maxDepth)
    {
            discard;
    }
   float sum = 0.0;
   float denom = 1./(2.*sig*sig);
   float gauss = (1./(2.*pi*sig*sig));
   int width = int(6.*sig)+1;
   for(int i=-width; i<width; i++ )
   {
           for(int j=-width; j<width; j++ )
           {
                float d = texture2D(depthTex,texCoord.st+vec2(float(i)*del_x,float(j)*del_y)).x;
                //if(abs(depth-d)>falloff)
                //    d =depth;
                sum += d *exp(-float(i*i+j*j)*denom);
            }
   }
   sum*= gauss;
   colorOut = vec4(sum,sum,sum,1.0);
   gl_FragDepth = sum;
}
