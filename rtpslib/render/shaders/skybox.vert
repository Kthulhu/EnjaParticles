#version 330
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 inTexCoord;

smooth out vec3 texCoord;
void main(void)
{
    texCoord = inTexCoord;
    gl_Position = projectionMatrix * viewMatrix * vec4(vertex,1.0f);
}
