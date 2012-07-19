#version 330 core

uniform sampler2D normalTex;
uniform sampler2D depthTex;
const float maxDepth = 0.9999999;

in vec2 texCoord;
out vec4 colorOut;

void main(void)
{
    float fldepth = texture2D(depthTex, texCoord.xy).x;
    if(fldepth > maxDepth)
    {
        discard;
    }
    colorOut = texture2D(normalTex, texCoord.xy);
    gl_FragDepth = fldepth;
}
