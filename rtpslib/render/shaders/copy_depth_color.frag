#version 330 core


uniform sampler2D scalarTex;
uniform sampler2D depthTex;
uniform float near;
uniform float far;
const float maxDepth = 0.9999999;

in vec2 texCoord;
out vec4 colorOut;

vec3 linearizeDepth(float z)
{
    float ret = 2.0 * z - 1.0;
    ret=(2.0 * near * far) / (far + near - ret * (far - near));
    return vec3(ret,ret,ret);
}

void main(void)
{
    float fldepth = texture2D(depthTex, texCoord.xy).x;
    if(fldepth > maxDepth)
    {
        discard;
    }
    colorOut = vec4(linearizeDepth(texture2D(scalarTex, texCoord.xy).x),1.0f);
    gl_FragDepth = fldepth;
}
