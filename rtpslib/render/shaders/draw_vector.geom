#version 150
//#geometry shader

layout(points) in;
layout(line_strip, max_vertices=2) out;
in vec4 vector[1];
in vec4 color[1];
uniform float scale;

void main() 
{
    vec4 p = gl_in[0].gl_Position;
    gl_Position = p; 
    gl_FrontColor = color[0];
    EmitVertex();

    gl_Position = vec4(p.rgb+(scale*vector[0].rgb),p.a);
    gl_FrontColor = color[0];
    EmitVertex();
    EndPrimitive();
}

