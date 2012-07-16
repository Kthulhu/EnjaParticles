#version 330
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

uniform float scale;

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 col;

smooth out vec4 vector;
smooth out vec4 color;

void main() 
{
    gl_Position = projectionMatrix * viewMatrix * vec4(pos.xyz,1.0f);
    vector = projectionMatrix * viewMatrix * vec4(col.xyz,1.0f);
    color = vec4(abs(normalize(col.rgb)),1.0);
}


