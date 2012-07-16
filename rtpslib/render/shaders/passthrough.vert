#version 330 core
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 col;

smooth out vec4 color;

void main(void)
{
    gl_Position = projectionMatrix * viewMatrix * pos;
    color = col;
}
