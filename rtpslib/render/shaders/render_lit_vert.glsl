#version 330
uniform mat4 modelview;
uniform mat4 project;
uniform mat3 normalMat;
struct Material
{
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    float shininess;
    float opacity;
};
uniform Material material;
struct Light
{
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    vec3 position;
};
uniform Light light;
layout (location = 0) in vec3 pos;
//layout (location = 1) in vec4 col;
layout (location = 2) in vec3 normal;
//layout (location = 3) in vec2 texcoords;

out vec4 color;
//out vec3 norm;
//out vec3 lightDir;
void main()
{
    vec3 norm = normalize(modelview*vec4(normal,0.0f)).xyz;
//    vec3 lightDir = normalize(gl_LightPosition[0].position.xyz-pos.xyz);

//    float cosang = clamp(dot(lightDir,norm),0.0,1.0);
//    color=intensity*col*cosang;
//    color = col;
    color = gl_FrontColor;
    gl_Position= project*modelview*vec4(pos,1.0);
}
