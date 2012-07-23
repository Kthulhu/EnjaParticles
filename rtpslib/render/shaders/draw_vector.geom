#version 330 core

layout(points) in;
layout(line_strip, max_vertices=2) out;
in vec4 vector[1];
in vec4 color[1];

smooth out vec4 col;

void main() 
{
    vec4 p = gl_in[0].gl_Position;
    gl_Position = p; 
    col = color[0];
    EmitVertex();

    gl_Position = vector[0];
    col = color[0];
    EmitVertex();
    EndPrimitive();
}

