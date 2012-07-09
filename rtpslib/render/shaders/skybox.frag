#version 150
uniform samplerCube skyboxCubeSampler;
smooth in vec4 position;
smooth in vec3 texCoord;
out vec4 color;

void main()
{
    //gl_FragColor = vec4(texture(skyboxCubeSampler,gl_TexCoord[0].xyz).rgb,1.0f);
    color = vec4(texture(skyboxCubeSampler,texCoord).rgb,1.0f);
}
