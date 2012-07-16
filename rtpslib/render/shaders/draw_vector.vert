#version 330
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

uniform float scale;

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 col;

out vec4 vector;
out vec4 color;

void main() 
{
    mat4 viewProj = projectionMatrix * viewMatrix;
    gl_Position = viewProj * vec4(pos.xyz,1.0f);
    vector = viewProj * vec4(col.xyz,1.0f);
    color = vec4(abs(normalize(col.xyz)),1.0);
}


