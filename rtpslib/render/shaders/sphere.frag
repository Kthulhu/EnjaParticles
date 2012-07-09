#version 120
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

uniform float pointRadius;  // point size in world space
//uniform float near;
//uniform float far;
varying vec3 posEye;        // position of center in eye space

void main()
{
    //const vec3 lightDir = vec3(0.577, 0.577, 0.577);
    //const float shininess = 50.0;

    // calculate normal from texture coordinates
    vec3 n;
    //we should find a better way of doing this...
    n.xy = gl_PointCoord.st*vec2(2.0, -2.0) + vec2(-1.0, 1.0);
    float mag = dot(n.xy, n.xy);
    if (mag > 1.0) discard;   // kill pixels outside circle
    n.z = sqrt(1.0-mag);

    // point on surface of sphere in eye space
    vec4 spherePosEye =vec4(posEye+n*pointRadius,1.0);

    vec4 clipSpacePos = projectionMatrix*spherePosEye;
    float normDepth = clipSpacePos.z/clipSpacePos.w;

    // Transform into window coordinates coordinates
    //(((far-near)/2.)*normDepth)+((far+near)/2.);
    gl_FragDepth = normDepth;

    gl_FragData[0] = gl_Color;
}
