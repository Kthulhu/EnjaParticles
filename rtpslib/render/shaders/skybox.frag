#version 330
uniform samplerCube skyboxCubeSampler;
smooth in vec3 texCoord;

void main()
{
    //gl_FragColor = vec4(texture(skyboxCubeSampler,gl_TexCoord[0].xyz).rgb,1.0f);
    gl_FragColor = vec4(texture(skyboxCubeSampler,texCoord).rgb,1.0f);
}
