#version 330 core
//#geometry shader

uniform float scale;

layout(points) in;
layout(line_strip, max_vertices=2) out;
in vec4 vector[1];
in vec4 color[1];

smooth out vec4 col[2];

void main() 
{
    vec4 p = gl_in[0].gl_Position;
    gl_Position = p; 
    col[0] = color[0];
    EmitVertex();

    gl_Position = vec4(p.xyz+(scale*vector[0].xyz),p.w);
    col[1] = color[0];
    EmitVertex();
    EndPrimitive();
}

