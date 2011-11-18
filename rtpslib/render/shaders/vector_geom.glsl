#version 150
//#geometry shader

layout(points) in;
layout(line_strip, max_vertices=2) out;
in vec4 vector[1];

void main() 
{
    vec4 p = gl_in[0].gl_Position;
    gl_Position = p; 
    EmitVertex();

    gl_Position = vec4(p.rgb+vector[0].rgb,p.a);
    EmitVertex();
    EndPrimitive();
}

