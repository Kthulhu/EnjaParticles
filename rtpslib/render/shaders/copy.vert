uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

// remember that you should draw a screen aligned quad
void main(void)
{
   gl_TexCoord[0] = gl_MultiTexCoord0;
   gl_Position = projectionMatrix*viewMatrix * vec4(gl_Vertex.xyz, 1.0);
}
