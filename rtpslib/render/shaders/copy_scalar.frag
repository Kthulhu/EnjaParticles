#version 330 core


uniform sampler2D scalarTex;
uniform sampler2D depthTex;
uniform float near;
uniform float far;
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
    colorOut = vec4(vec3(texture2D(scalarTex, texCoord.xy).x),1.0f);
    gl_FragDepth = fldepth;
}
