#version 120

uniform float timer;

varying vec4 vector;

void main() 
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    vector = gl_ModelViewProjectionMatrix * gl_Color;
    gl_FrontColor = vec4(normalize(gl_Color.rgb),1.0);
}


