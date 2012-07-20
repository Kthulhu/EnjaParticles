#version 330 core

uniform sampler2D colorTex;
uniform sampler2D depthTex;
const float maxDepth = 0.9999999f;

in vec2 texCoord;
out vec4 colorOut;

void main(void)
{
    float fldepth = texture2D(depthTex, texCoord.xy).x;
    if(fldepth > maxDepth)
    {
        discard;
    }
    colorOut = vec4(vec3(1.0f)-texture2D(colorTex, texCoord.xy).xyz,1.0f);
    gl_FragDepth = fldepth;
}
