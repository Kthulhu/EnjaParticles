jversion 120

layout (location = 0) in vec3 pos;
layout (location = 1) in vec4 col;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 texcoords;

out vec4 color;
//out vec3 norm;
//out vec3 lightDir;
void main()
{
    vec3 norm = normalize(gl_ModelViewMatrix*vec4(normal,0.0)).xyz;
    vec3 lightDir = normalize(gl_LightPosition[0].position.xyz-pos.xyz);

    float cosang = clamp(dot(lightDir,norm),0,1);
    color=intensity*col*cosang;
    gl_Position= gl_ModelViewProjectionMatrix*pos;
}
