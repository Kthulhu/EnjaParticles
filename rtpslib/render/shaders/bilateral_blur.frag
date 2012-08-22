#version 330 core
uniform sampler2D depthTex; // the texture with the scene you want to blur
uniform vec2 blurDir;
uniform float sig_range;
uniform float sig;
const float pi = 3.141592654f;
const float maxDepth = 0.9999999f;

in vec2 texCoord;
out vec4 colorOut;

void main(void)
{
    float depth = texture2D(depthTex,texCoord.xy).x;
    if(depth>maxDepth)
    {
            discard;
    }
    float wsum = 0.0f;
    float sum = 0.0f;
    float denom1 = (1.f/(2.f*sig*sig));
    float denom2 = (1.f/(2.f*sig_range*sig_range));
    float gauss_s =(1./(sqrt(2.*pi)*sig));
    float gauss_r =(1./(sqrt(2.*pi)*sig_range));
    int width = int(6.f*sig)+1;

    for(int x=-width; x<=width; x++)
    {
            float samp = texture2D(depthTex,texCoord.xy+x*blurDir).x;

            float w = exp(-float(x*x)*denom1);

            float r2 = (samp - depth);
            float g = exp(-(r2*r2)*denom2);
            sum += samp * w * g;
            wsum += w * g;
    }
    if(wsum > 0.0f)
    {
            sum /= wsum;
    }
    sum*=gauss_r*gauss_s;

    colorOut = vec4(sum,sum,sum,1.0f);
    gl_FragDepth=sum;
}
