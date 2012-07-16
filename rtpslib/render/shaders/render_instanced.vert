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
uniform Material material;
uniform Light light;

layout(location = 0) in vec3 pos;
//layout(location = 1) in vec4 col;
layout(location = 2) in vec4 com_pos;
layout(location = 3) in vec4 com_rot;
layout(location = 4) in vec3 normal;
//layout(location = 5) in vec2 texcoords;

vec4 quaternionMultiply(vec4 q1, vec4 q2)
{
    vec4 retval = vec4(0.f,0.f,0.f,0.f);
    retval.xyz = q1.w*q2.xyz+q2.w*q1.xyz+cross(q1.xyz,q2.xyz);
    retval.w = q1.w*q2.w-dot(q1.xyz,q2.xyz);
    return retval;
}
vec4 rotate(vec4 v, vec4 q)
{
    vec4 vtemp = quaternionMultiply(q,v);
    q.xyz = -q.xyz;
    return quaternionMultiply(vtemp,q);
}

smooth out vec4 color;
smooth out vec3 norm;
smooth out vec3 lightDir;
//out vec3 lightDir;
void main()
{
    vec4 viewPos;
    vec3 eyePos;
    norm = normalize(rotate(vec4( normal, 0.0),com_rot).xyz);
    norm = normalize((viewMatrix*vec4(norm,0.0f)).xyz);
    vec4 localcoord=rotate(vec4(pos,0.0),com_rot);
    viewPos = viewMatrix*vec4(localcoord.xyz+com_pos.xyz,1.0);
    eyePos = viewPos.xyz/viewPos.w;

    lightDir = normalize(light.position-eyePos);
    //lightDir = normalize(light.pos);

    gl_Position= projectionMatrix*viewPos;
//    lightDir = normalize(gl_LightPosition[0].position.xyz-com_pos.xyz);

    //gl_Position=project*modelview*(localcoord+com_pos);
}
