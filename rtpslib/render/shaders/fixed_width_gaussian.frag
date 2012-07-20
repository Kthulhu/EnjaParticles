#version 330 core
uniform sampler2D imgTex; // the texture with the scene you want to blur
uniform vec2 dTex;
in vec2 texCoord;
out vec4 colorOut;

void main(void)
{
   float sum = 0.0;
   float val = texture2D(imgTex, texCoord).x;
   //discard if value is 0.0
   //if(val==0.0f)
   //    discard;

   sum += texture2D(imgTex, texCoord-dTex*4.0f).x * 0.05;
   sum += texture2D(imgTex, texCoord-dTex*3.0f).x * 0.09;
   sum += texture2D(imgTex, texCoord-dTex*2.0f).x * 0.12;
   sum += texture2D(imgTex, texCoord-dTex).x * 0.15;
   sum += val * 0.16;
   sum += texture2D(imgTex, texCoord+dTex).x * 0.15;
   sum += texture2D(imgTex, texCoord+dTex*2.0f).x * 0.12;
   sum += texture2D(imgTex, texCoord+dTex*3.0f).x * 0.09;
   sum += texture2D(imgTex, texCoord+dTex*4.0f).x * 0.05;

   colorOut = vec4(sum,sum,sum,sum);
}
