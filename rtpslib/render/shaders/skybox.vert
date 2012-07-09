#version 150
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

in vec3 vertex;
in vec3 inTexCoord;

smooth out vec4 position;
smooth out vec3 texCoord;
void main(void)
{
    texCoord = inTexCoord;
    position = projectionMatrix * viewMatrix * vec4(vertex,1.0f);
}
