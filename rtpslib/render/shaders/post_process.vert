#version 330
layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 texCoordIn;

out vec2 texCoord;
//uniform mat4 projectionMatrix;
//uniform mat4 viewMatrix;

//Fullscreen quad pass through. Assumes vertices are the for corners of the screen.
void main(void)
{
    //position = vec4(pos);
    gl_Position=vec4(pos,0.0f,1.0f);
    texCoord = texCoordIn;
    //gl_TexCoord[0] = gl_MultiTexCoord0;
}
