uniform mat4 inverseProjectionMatrix;
struct Material
{
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    float shininess;
    float opacity;
};
struct Light
{
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    vec3 position;
};


uniform Material material;
uniform Light light;

uniform sampler2D depthTex;
uniform float del_x;
uniform float del_y;
const float maxDepth = 0.999999;

vec3 uvToEye(vec2 texCoord,float z)
{
	// convert texture coordinate to homogeneous space
	vec2 xyPos = (texCoord*2. -1.);
	// construct clip-space position
	vec4 clipPos = vec4( xyPos, z, 1.0 );
        vec4 viewPos =  (inverseProjectionMatrix * clipPos);
	return(viewPos.xyz/viewPos.w);
}

void main()
{
	float depth = texture2D(depthTex,gl_TexCoord[0].xy).x;
	if(depth>maxDepth)
	{
		discard;
		//return;
	}

	vec3 posEye = uvToEye(gl_TexCoord[0].xy,depth);
	vec2 texCoord1 = vec2(gl_TexCoord[0].x+del_x,gl_TexCoord[0].y);
	vec2 texCoord2 = vec2(gl_TexCoord[0].x-del_x,gl_TexCoord[0].y);

	vec3 ddx = uvToEye(texCoord1, texture2D(depthTex,texCoord1.xy).x)-posEye;
	vec3 ddx2 = posEye-uvToEye(texCoord2, texture2D(depthTex,texCoord2.xy).x);
	if(abs(ddx.z)>abs(ddx2.z))
	{
		ddx = ddx2;
	}

	texCoord1 = vec2(gl_TexCoord[0].x,gl_TexCoord[0].y+del_y);
	texCoord2 = vec2(gl_TexCoord[0].x,gl_TexCoord[0].y-del_y);

	vec3 ddy = uvToEye(texCoord1, texture2D(depthTex,texCoord1.xy).x)-posEye;
	vec3 ddy2 = posEye-uvToEye(texCoord2, texture2D(depthTex,texCoord2.xy).x);
	if(abs(ddy.z)>abs(ddy2.z))
	{
		ddy = ddy2;
	}

	vec3 n = cross(ddx,ddy);
        n = normalize(n);
        vec3 ambientColor=material.ambient*light.ambient;
        //vec3 specularColor=material.specular*light.specular*pow(max(dot(nnorm, halfLightDir), 0.0) , material.shininess);
        vec3 lightDir = normalize(light.position-posEye);
        vec3 ref = normalize(reflect(-lightDir,n));
        float spec = max(0.0,dot(n,ref));
        vec3 specularColor=material.specular*pow(spec,material.shininess);
        vec3 diffuseColor=material.diffuse*light.diffuse*max(dot(n,lightDir), 0.0);
        gl_FragColor = vec4(ambientColor+specularColor+diffuseColor,material.opacity);
        //gl_FragColor = vec4(n,1.0f);
}
