//#define KERNEL_SIZE 10000
#define KERNEL_DIAMETER 60 
const float sigmasq = 64. ;//0.84089642*0.84089642;
const float pi = 3.141592654;

//float kernel[KERNEL_SIZE];

uniform sampler2D depthTex;
uniform float del_x;
uniform float del_y;

float linearizeDepth(float depth, float f, float n)
{
    return (2*n)/(f+n-depth*(f-n));
}
float nonlinearDepth(float depth, float f, float n)
{
    return ((-2*n)/depth+f+n)/(f-n);
}
//vec2 offset[KERNEL_SIZE];

void main(void)
{
	float depth=texture2D(depthTex, gl_TexCoord[0].st).x;
//	float depth=linearizeDepth(texture2D(depthTex, gl_TexCoord[0].st).x,1000.0,0.3);
    float maxDepth=0.999999;
    //float maxDepth=1.0;
	//float threshold=0.01;
	if(depth>maxDepth)
	{
		discard;
		//return;
	}
   float sum = 0.0;	
   for(int i=0; i<KERNEL_DIAMETER; i++ )
   {
	   for(int j=0; j<KERNEL_DIAMETER; j++ )
	   {
			float tmp = texture2D(depthTex,gl_TexCoord[0].st+vec2(float(i-(KERNEL_DIAMETER/2))*del_x,float(j-(KERNEL_DIAMETER/2))*del_y)).x;//texture2D(depthTex, gl_TexCoord[0].st + offset[(i*KERNEL_DIAMETER)+j]).x;
			//float tmp = linearizeDepth(texture2D(depthTex,gl_TexCoord[0].st+vec2(float(i-(KERNEL_DIAMETER/2))*del_x,float(j-(KERNEL_DIAMETER/2))*del_y)).x,1000.0,0.3);
			//if(tmp-depth>threshold)
			//	tmp=depth;//+(sign(tmp-depth))*threshold;//continue;
			sum += tmp * (1./(2.*pi*sigmasq))*exp(-(pow(float(i-(KERNEL_DIAMETER/2)),2.)+pow(float(j-(KERNEL_DIAMETER/2)),2.))/(2.*sigmasq));
		}
   }
   //if(sum<0.05)
	//	sum=1.0;
   gl_FragData[0] = vec4(sum,sum,sum,1.0);
   //sum = nonlinearDepth(sum,1000.0, 0.3);
   gl_FragDepth = sum;
}
/*uniform sampler2D depthTex; // the texture with the scene you want to blur
uniform vec2 blurDir;
uniform float blurScale;
uniform float blurDepthFallOff;

float filterwidth(vec2 v)
{
  vec2 fw = max(abs(dFdx(v)), abs(dFdy(v)));
  return max(fw.x, fw.y);
}

void main(void)
{
	float depth = texture2D(depthTex,gl_TexCoord[0].xy).x;
	float sum = 0.0;
	float wsum = 0.0;
	float filterRadius = 3.0;
	float blurDir = vec2(1.0,0.0)*1/800;//*dFdy(gl_TexCoord[0].xy);
	float blurDepthFallOff = 1.0;
	float blurScale = 1./5.;
	for(float x=-filterRadius; x<=filterRadius; x+=1.0)
	{
		float sample = texture2D(depthTex,gl_TexCoord[0].xy+x*blurDir).x;
		
		float r = x * blurScale;
		float w = exp(-r*r);

		float r2 = (sample - depth) * blurDepthFallOff;
		float g = exp(-r2*r2);
		sum += sample * w * g;
		wsum += w * g;
	}
	if(wsum > 0.0)
	{
		sum /= wsum;
	}

	gl_FragData[0] = vec4(sum,sum,sum,1.0);
	gl_FragDepth=sum;
}*/
