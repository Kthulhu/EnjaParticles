#version 330 core
uniform mat4 projectionMatrix;

uniform float pointRadius;  // point size in world space
//uniform float near;
//uniform float far;
smooth in vec3 posEye;        // position of center in eye space
smooth in vec4 color;
uniform sampler2D sceneDepth;

out vec4 colorOut;

void main()
{
    //const vec3 lightDir = vec3(0.577, 0.577, 0.577);
    //const float shininess = 50.0;

    // calculate normal from texture coordinates
    vec3 n;
    //we should find a better way of doing this...
    //n.xy = gl_PointCoord.st*vec2(2.0, -2.0) + vec2(-1.0, 1.0);
    n.xy= gl_PointCoord.st*2.0f-1.0f;
    float mag = dot(n.xy, n.xy);
    if (mag > 1.0) discard;   // kill pixels outside circle
    n.z = sqrt(1.0-mag);

    // point on surface of sphere in eye space
    vec4 spherePosEye =vec4(posEye+n*pointRadius,1.0);

    vec4 clipSpacePos = projectionMatrix*spherePosEye;
    float normDepth = clipSpacePos.z/clipSpacePos.w;

    // Transform into window coordinates coordinates
    normDepth=0.5*normDepth+.5;

    colorOut = color;
    //compare against depth of scene geometry.
    if(normDepth>=texture2D(sceneDepth,gl_FragCoord.xy*0.5+0.5).x)
        discard;
    else
        gl_FragDepth = normDepth;
}
