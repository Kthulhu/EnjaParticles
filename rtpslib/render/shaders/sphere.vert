#version 120
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

uniform float pointRadius;
uniform float pointScale;   // scale to calculate size in pixels

varying out vec3 posEye;        // position of center in eye space

void main()
{

    posEye = vec3(viewMatrix * vec4(gl_Vertex.xyz, 1.0));
    float dist = length(posEye);
    gl_PointSize = pointRadius * (pointScale / dist);

    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position = projectionMatrix * viewMatrix * vec4(gl_Vertex.xyz, 1.0);

    gl_FrontColor = gl_Color;
}
