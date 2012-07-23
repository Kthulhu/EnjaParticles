#version 330 core
uniform mat4 projectionMatrix;

uniform float pointRadius;  // point size in world space
//uniform float near;
//uniform float far;
smooth in vec3 posEye;        // position of center in eye space
smooth in vec4 color;

out vec4 colorOut;

void main()
{
    //const vec3 lightDir = vec3(0.577, 0.577, 0.577);
    //const float shininess = 50.0;

    // calculate normal from texture coordinates
    vec3 n;
    //we should find a better way of doing this...
    //n.xy = gl_PointCoord.st*vec2(2.0, -2.0) + vec2(-1.0, 1.0);
    n.xy = gl_PointCoord.st*2. - 1.;
    float mag = dot(n.xy, n.xy);
    if (mag > 1.0) discard;   // kill pixels outside circle
    n.z = sqrt(1.0-mag);
    float thickness =0.005f+ n.z*pointRadius*0.05f;

    // point on surface of sphere in eye space
    //vec4 spherePosEye =vec4(posEye+n*pointRadius,1.0);

    //vec4 clipSpacePos = projectionMatrix*spherePosEye;
    //float normDepth = clipSpacePos.z/clipSpacePos.w;

    // Transform into window coordinates coordinates
    //normDepth=0.5*normDepth+.5;

    colorOut = vec4(thickness,thickness,thickness,1.0f);
    //colorOut = vec4(1.0f,1.0f,1.0f,.6f);
    //colorOut=vec4(1.0f,0.0f,0.0f,1.0f);
    //gl_FragDepth = normDepth;
}
