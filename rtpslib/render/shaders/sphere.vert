#version 330
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

uniform float pointRadius;
uniform float pointScale;   // scale to calculate size in pixels

layout(location=0) in vec4 pos;
layout(location=1) in vec4 col;

smooth out vec3 posEye;        // position of center in eye space
smooth out vec4 color;

void main()
{
    posEye = vec3(viewMatrix * vec4(pos.xyz,1.0f));
    float dist = length(posEye);
    gl_PointSize = pointRadius * (pointScale / dist);

    //gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position = projectionMatrix * viewMatrix * vec4(pos.xyz, 1.0);

    color = col;
}
