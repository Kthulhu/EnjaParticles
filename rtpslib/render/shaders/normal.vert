#version 330 core
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 inverseProjectionMatrix;

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 texCoordIn;

out vec2 texCoord;

void main()
{
        texCoord = texCoordIn;
        gl_Position = pos;
}
