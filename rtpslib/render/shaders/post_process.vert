uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

//Fullscreen quad pass through. Assumes vertices are the for corners of the screen.
void main(void)
{
    gl_Position = gl_Vertex;
    gl_TexCoord[0] = gl_MultiTexCoord0;
}
