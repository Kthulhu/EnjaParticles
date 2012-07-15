#version 330

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

uniform sampler2D normalTex;
uniform sampler2D depthTex;
smooth in vec2 texCoord;

void main(void)
{
    float fldepth = texture2D(depthTex, texCoord.xy).x;
    if(fldepth > .999)
    {
        discard;
    }    
    gl_FragColor = texture2D(normalTex, texCoord.xy);
    gl_FragDepth = fldepth;

}
