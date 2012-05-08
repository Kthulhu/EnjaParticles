#version 330
struct Material
{
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    float shininess;
    float opacity;
};
struct Light
{
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    vec3 pos;
};

uniform mat4 modelview;
uniform mat4 project;
uniform mat4 viewinvmat;
uniform Material material;
uniform Light light;
layout (location = 0) in vec3 pos;
//layout (location = 1) in vec4 col;
layout (location = 2) in vec3 normal;
//layout (location = 3) in vec2 texcoords;

//smooth out vec4 color;
smooth out vec3 normalVec;
smooth out vec3 eyeVec;
smooth out vec3 lightVec;
//smooth out vec3 halfLightDir;
void main()
{
    vec4 viewPos;

    viewPos = modelview*vec4(pos,1.0);

    //color=col;
    //lightDir = normalize(light.pos-eyePos);
    //lightDir = normalize(light.pos);
    //halfLightDir=normalize(light.pos+eyePos);
	normalVec = normalize(modelview*vec4(normal,0.0f)).xyz;
	eyeVec = -viewPos.xyz/viewPos.w;
	lightVec = light.pos - viewPos.xyz;
    gl_Position= project*viewPos;
}
