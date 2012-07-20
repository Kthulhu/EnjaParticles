#version 330 core
uniform mat4 inverseProjectionMatrix;

uniform sampler2D depthTex;
uniform float del_x;
uniform float del_y;
const float maxDepth = 0.9999999f;

in vec2 texCoord;
out vec4 colorOut;

vec3 uvToEye(vec2 texCoordinate,float z)
{
	// convert texture coordinate to homogeneous space
        vec2 xyPos = (texCoordinate*2. -1.);
	// construct clip-space position
	vec4 clipPos = vec4( xyPos, z, 1.0 );
        vec4 viewPos =  (inverseProjectionMatrix * clipPos);
	return(viewPos.xyz/viewPos.w);
}

void main()
{
        float depth = texture2D(depthTex,texCoord).x;
	if(depth>maxDepth)
	{
		discard;
		//return;
	}

        vec3 posEye = uvToEye(texCoord,depth);
        vec2 texCoord1 = vec2(texCoord.x+del_x,texCoord.y);
        vec2 texCoord2 = vec2(texCoord.x-del_x,texCoord.y);

	vec3 ddx = uvToEye(texCoord1, texture2D(depthTex,texCoord1.xy).x)-posEye;
	vec3 ddx2 = posEye-uvToEye(texCoord2, texture2D(depthTex,texCoord2.xy).x);
	if(abs(ddx.z)>abs(ddx2.z))
	{
		ddx = ddx2;
	}

        texCoord1 = vec2(texCoord.x,texCoord.y+del_y);
        texCoord2 = vec2(texCoord.x,texCoord.y-del_y);

	vec3 ddy = uvToEye(texCoord1, texture2D(depthTex,texCoord1.xy).x)-posEye;
	vec3 ddy2 = posEye-uvToEye(texCoord2, texture2D(depthTex,texCoord2.xy).x);
	if(abs(ddy.z)>abs(ddy2.z))
	{
		ddy = ddy2;
	}

	vec3 n = cross(ddx,ddy);
        n = normalize(n);
        n = 0.5f*n+0.5f;
        //n = normalize(n);


        colorOut = vec4(n,1.0f);
}
