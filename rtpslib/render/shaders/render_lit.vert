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
    vec3 position;
};

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 meshMatrix;
//uniform mat4 normalMatrix;

uniform Material material;
uniform Light light;
layout (location = 0) in vec3 pos;
//layout (location = 1) in vec4 col;
layout (location = 2) in vec3 normal;
//layout (location = 3) in vec2 texcoords;

//smooth out vec4 color;
smooth out vec3 norm;
smooth out vec3 lightDir;
smooth out vec3 halfLightDir;

void main()
{
    vec4 viewPos;
    vec3 eyePos;
    mat4 modelview =viewMatrix*meshMatrix;
    //vec4 temp =(mat3(inverse(modelview))*vec4(normal,0.0f));
    norm = normalize(mat3(inverse(modelview))*normal);
    //norm = normalize(normal);
    viewPos = modelview*vec4(pos,1.0f);
    eyePos = viewPos.xyz/viewPos.w;
    
    //color=col;
    lightDir = normalize(light.position-eyePos);

    gl_Position= projectionMatrix*viewPos;
    //gl_Position=vec4(pos,1.0f);
}
