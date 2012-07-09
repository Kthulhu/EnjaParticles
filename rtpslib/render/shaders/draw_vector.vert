#version 120
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

varying out vec4 vector;
varying out vec4 color;

void main() 
{
    gl_Position = projectionMatrix * viewMatrix * gl_Vertex;
    vector = projectionMatrix * viewMatrix * gl_Color;
    color = vec4(abs(normalize(gl_Color.rgb)),1.0);
}


