#version 330 core
uniform sampler2D imgTex; // the texture with the scene you want to blur
uniform vec2 dTex;
in vec2 inTexCoord;
out vec4 colorOut;

void main(void)
{
   float sum = 0.0;

   sum += texture2D(imgTex, inTexCoord-dTex*4.0f).x * 0.05;
   sum += texture2D(imgTex, inTexCoord-dTex*3.0f).x * 0.09;
   sum += texture2D(imgTex, inTexCoord-dTex*2.0f).x * 0.12;
   sum += texture2D(imgTex, inTexCoord-dTex).x * 0.15;
   sum += texture2D(imgTex, inTexCoord).x * 0.16;
   sum += texture2D(imgTex, inTexCoord+dTex).x * 0.15;
   sum += texture2D(imgTex, inTexCoord+dTex*2.0f).x * 0.12;
   sum += texture2D(imgTex, inTexCoord+dTex*3.0f).x * 0.09;
   sum += texture2D(imgTex, inTexCoord+dTex*4.0f).x * 0.05;

   colorOut = vec4(sum,0.0f,0.0f,1.0f);
}
