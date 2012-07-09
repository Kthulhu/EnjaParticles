uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 inverseProjectionMatrix;

void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
        gl_Position = gl_Vertex;
}
