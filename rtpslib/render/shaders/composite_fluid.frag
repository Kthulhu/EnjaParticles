#version 330 core
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

uniform float gamma;
uniform sampler2D depthTex;
uniform sampler2D normalTex;
uniform sampler2D thicknessTex;
uniform sampler2D sceneTex;

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

float IOR = 1.333; //refractive indice for water = 1.333
float eta = max(IOR, 0.00001);
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

void main(void)
{
    float depth = texture2D(depthTex,texCoord).x;
    if(depth>maxDepth)
    {
            discard;
            //return;
    }
    vec3 posEye = uvToEye(texCoord,depth);
    vec3 n = normalize(2.0f*(texture2D(normalTex,texCoord).xyz-0.5f));
    float thickness = texture2D(thicknessTex,texCoord).x;
    vec3 ambientColor=material.ambient*light.ambient;

    vec3 lightDir = normalize(light.position-posEye);
    //vec3 halfLightDir = normalize(lightDir+posEye);//normalize((lightDir+posEye)*0.5f);
    //float fresnel = fresnel_dielectric(normalize(posEye),n,eta);


    //thickness=300.f*gamma;
    //thickness+=0.1f;
    //thickness=0.75f;
    float beta = thickness*0.1f;
    float lerpFact =exp(-thickness);
    //vec3 a = mix(material.diffuse,texture2D(sceneTex,texCoord.xy).xyz,lerpFact);
    vec3 a = mix(material.diffuse,texture2D(sceneTex,texCoord.xy+vec2(n.x*beta,n.y*beta)).xyz,lerpFact);
    //vec3 specularColor=material.specular*light.specular*pow(max(dot(n, halfLightDir), 0.0) , material.shininess);
    vec3 ref = normalize(reflect(-lightDir,n));
    float spec = max(0.0,dot(n,ref));
    vec3 specularColor=material.specular*pow(spec,material.shininess);
    //vec3 diffuseColor=material.diffuse*light.diffuse*max(dot(n,lightDir), 0.0);

    //colorOut = vec4(ambientColor+specularColor+diffuseColor,material.opacity);
    colorOut = vec4(a+specularColor,1.0f);
}
