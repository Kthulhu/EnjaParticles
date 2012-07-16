#version 330 core
uniform samplerCube skyboxCubeSampler;
in vec3 texCoord;
out vec4 colorOut;

void main()
{
    //gl_FragColor = vec4(texture(skyboxCubeSampler,gl_TexCoord[0].xyz).rgb,1.0f);
    colorOut = vec4(texture(skyboxCubeSampler,texCoord).rgb,1.0f);
}
