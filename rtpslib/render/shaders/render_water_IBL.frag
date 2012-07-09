#version 330
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

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 inverseViewMatrix; //ViewMatrixInverse or ViewMatrixTranspose
smooth in vec3 normalVec, eyeVec, lightVec;

//uniform sampler2D reflectionEquiSampler;
uniform samplerCube reflectionCubeSampler;

#define M_PI 3.14159265358979323846

bool cubemap = true; //cubemap or equirectangular environment map
vec3 waterColor = vec3(0.2,0.65,0.9); //color of the water
float waterDensity = 0.5; //water density (0.0-1.0)

float fresnel_dielectric(vec3 incoming, vec3 normal, float eta)
{
	float c = abs(dot(incoming, normal));
	float g = eta * eta - 1.0 + c * c;
	float result;

	if(g > 0.0) {
		g = sqrt(g);
		float A =(g - c)/(g + c);
		float B =(c *(g + c)- 1.0)/(c *(g - c)+ 1.0);
		result = 0.5 * A * A *(1.0 + B * B);
	}
	else
		result = 1.0;
	return result;
}

vec3 equirectangular(vec3 vec)
{
	float phi = atan(vec.x, vec.y);
	float theta = asin(vec.z);
	float x = -0.5*(phi / M_PI - 1.0);
	float y = 0.5 + theta/M_PI;
	return vec3(x, y, 0.0);
}

vec3 to_world(vec3 vec)
{
        vec3 wvec = (inverseViewMatrix*vec4(vec, 1.0)-inverseViewMatrix[3]).xyz;
	return normalize(wvec);
}

void main(void)
{
	//normalizing vecs
	vec3 lVec = normalize(lightVec);
	vec3 nVec = normalize(normalVec);
	vec3 vVec = normalize(eyeVec);

	//fresnel term
	float IOR = 1.333; //refractive indice for water = 1.333
	float eta = max(IOR, 0.00001);
	float fresnel = fresnel_dielectric(vVec,nVec,eta);

	//reflection coordinates for environment map
	vec3 reflCoords = -reflect(vVec,nVec);
	reflCoords = to_world(reflCoords); //getting to world space
	vec3 equiCoords = equirectangular(reflCoords); //for latitude-longitude texture

	//reflection
	vec3 reflection = vec3(0.0);

	//if(cubemap)
	//{
		reflection = texture(reflectionCubeSampler,reflCoords.xyz).rgb; //samplerCube
	//}
	//else
	//{
	//	reflection = texture2DLod(reflectionEquiSampler,equiCoords.xy,0.0).rgb;
	//}

	vec3 luminosity = vec3(0.299, 0.587, 0.114);
	float reflectivity = pow(dot(luminosity, reflection.rgb),4.0);

	//refraction
	vec3 rIOR = vec3(-0.33109,-0.33605,-0.33957); //(air IOR - water IOR).RGB

	vec3 refrCoordsR = to_world(refract(vVec,nVec, rIOR.r));
	vec3 refrCoordsG = to_world(refract(vVec,nVec, rIOR.g));
	vec3 refrCoordsB = to_world(refract(vVec,nVec, rIOR.b));

	vec3 refraction = vec3(0.0);

	//if(cubemap)
	//{
		refraction.r = texture(reflectionCubeSampler,refrCoordsR).r;
		refraction.g = texture(reflectionCubeSampler,refrCoordsG).g;
		refraction.b = texture(reflectionCubeSampler,refrCoordsB).b;
	/*}
	else
	{
		refraction.r = texture2DLod(reflectionEquiSampler,equirectangular(refrCoordsR).xy,0.0).r;
		refraction.g = texture2DLod(reflectionEquiSampler,equirectangular(refrCoordsG).xy,0.0).g;
		refraction.b = texture2DLod(reflectionEquiSampler,equirectangular(refrCoordsB).xy,0.0).b;
	}*/

	vec3 transmittance = mix(refraction,refraction*material.diffuse,waterDensity);

	//specularity
	vec3 hVec = normalize(lVec + vVec);
	float rslt = max(dot(hVec, nVec), 0.0);
	float specfac = pow(rslt, 300.0);
	vec3 specular = vec3(1.0)*specfac;

	//final color
	vec3 final = mix(transmittance, reflection, fresnel+(reflectivity*(fresnel*0.1+0.9)));
	gl_FragColor = vec4(final,material.opacity);
}
