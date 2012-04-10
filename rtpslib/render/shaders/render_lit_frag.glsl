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
//smooth in vec4 color;
smooth in vec3 norm;
smooth in vec3 lightDir;
smooth in vec3 halfLightDir;
//in vec3 norm;
//in vec3 lightDir;

void main(void) {
    vec3 ambientColor=material.ambient*light.ambient;
    //vec3 specularColor=material.specular*light.specular*pow(max(dot(nnorm, halfLightDir), 0.0) , material.shininess);
    vec3 ref = normalize(reflect(-normalize(lightDir),normalize(norm)));
    float spec = max(0.0,dot(normalize(norm),ref));
    vec3 specularColor=material.specular*pow(spec,material.shininess);
    vec3 diffuseColor=material.diffuse*light.diffuse*max(dot(normalize(norm),normalize(lightDir)), 0.0);
    gl_FragColor = vec4(ambientColor+specularColor+diffuseColor,material.opacity);
    //gl_FragColor = color;
    //gl_FragColor = vec4(material.ambient+material.diffuse,material.opacity);
}
