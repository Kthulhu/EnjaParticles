#version 330

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

uniform sampler2D normalTex;
uniform sampler2D depthTex;
in vec2 texCoord;
out vec4 colorOut;

void main(void)
{
    float fldepth = texture2D(depthTex, texCoord.xy).x;
    if(fldepth > .9999999f)
    {
        discard;
    }    
    colorOut = texture2D(normalTex, texCoord.xy);
    gl_FragDepth = fldepth;

}
