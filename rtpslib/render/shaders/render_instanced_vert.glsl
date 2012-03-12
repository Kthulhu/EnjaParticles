#version 330
uniform mat4 modelview;
uniform mat4 project;
layout(location = 0) in vec3 pos;
layout(location = 1) in vec4 col;
layout(location = 2) in vec4 com_pos;
layout(location = 3) in vec4 com_rot;
layout(location = 4) in vec3 normal;
layout(location = 5) in vec2 texcoords;

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

out vec4 color;
out vec3 norm;
//out vec3 lightDir;
void main()
{
    norm = rotate(vec4( normal, 0.0),com_rot).xyz;
    norm = (modelview*vec4(norm,0.0)).xyz;
//    lightDir = normalize(gl_LightPosition[0].position.xyz-com_pos.xyz);
    vec4 localcoord=rotate(vec4(pos,0.0),com_rot);
    color = col;
    gl_Position=project*modelview*(localcoord+com_pos);
}
