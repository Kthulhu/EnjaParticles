#version 330 core
uniform mat4 projectionMatrix;
//uniform mat4 inverseProjectionMatrix;

const float maxDepth = 0.9999999f;
uniform sampler2D depthTex; 
uniform float del_x;
uniform float del_y;
uniform float dt;
uniform float depthThreshold;
in vec2 texCoord;
out vec4 colorOut;
/*vec3 uvToEye(vec2 texCoordinate,float z)
{
        // convert texture coordinate to homogeneous space
        vec2 xyPos = (texCoordinate*2. -1.);
        // construct clip-space position
        vec4 clipPos = vec4( xyPos, z, 1.0 );
        vec4 viewPos =  (inverseProjectionMatrix * clipPos);
        return(viewPos.xyz/viewPos.w);
}*/
//consider using textureGrad() for derivative information.

float secondOrderCenterDifference(vec2 texCoord,float depth, vec2 dir, float h)
{
    //if(abs(texture2D(depthTex, texCoord+dir).x- 2.0*depth +texture2D(depthTex, texCoord-dir).x)>distance_threshold)
    //    return 0.0;
    return (texture2D(depthTex, texCoord+dir).x- 2.0f*depth +texture2D(depthTex, texCoord-dir).x);//(h*h);
}

float secondOrderCenterDifferenceMixed(vec2 texCoord)//, float h, float k)
{
    //if(abs(texture2D(depthTex, texCoord+dir).x- 2.0*depth +texture2D(depthTex, texCoord-dir).x)>distance_threshold)
    //    return 0.0;
    return (texture2D(depthTex, texCoord+vec2(del_x,del_y)).x- texture2D(depthTex, texCoord+vec2(-del_x,del_y)).x
            -texture2D(depthTex, texCoord+vec2(del_x,-del_y)).x +texture2D(depthTex, texCoord+vec2(-del_x,-del_y) ).x)/4.0f;//(h*h);
}

float centerDifference(vec2 texCoord, vec2 dir, float h)
{
    //if(abs(texture2D(depthTex, texCoord+dir).x-texture2D(depthTex, texCoord-dir).x)>distance_threshold)
    //    return 0.0;
    return (texture2D(depthTex, texCoord+dir).x-texture2D(depthTex, texCoord-dir).x)/2.0f;///(2.0*h);
}

void main()
{

    float depth=texture2D(depthTex, texCoord.st).x;
    if(depth>maxDepth)
        discard;
    float Cx = del_x*(-2.f/projectionMatrix[0][0]);//2.0/(width*focal_x);
    float Cy = del_y*(-2.f/projectionMatrix[1][1]);//2.0/(height*focal_y);
    float Cx2 = Cx*Cx;
    float Cy2 = Cy*Cy;
    float dx = centerDifference(texCoord.st,vec2(del_x,0.0f),1.0f);//,h_x);
    float ddx = secondOrderCenterDifference(texCoord.st,depth,vec2(del_x,0.0),1.0f);//,h_x);
    float dy = centerDifference(texCoord.st,vec2(0.0f,del_y),1.0f);//,h_y);
    float ddy = secondOrderCenterDifference(texCoord.st,depth,vec2(0.0,del_y),1.0f);//,h_y);
    float D = (Cy2*dx*dx)+(Cx2*dy*dy)+(Cy2*Cx2*depth*depth);
    //float depthX = texture2D(depthTex, dx_texCoord).x;
    //float dxdx = centerDifference(dx_texCoord,vec2(del_x,0.0),1.0f);//,h_x);
    float dydx = secondOrderCenterDifferenceMixed(texCoord);//centerDifference(dx_texCoord,vec2(0.0,del_y),1.0f);//,h_y);
    //float depthY = texture2D(depthTex, dy_texCoord).x;
    //float dxdy = centerDifference(dy_texCoord,vec2(del_x,0.0),1.0f);//,h_x);
    //float dydy = centerDifference(dy_texCoord,vec2(0.0,del_y),1.0f);//,h_y);
    float Dx=2.f*((Cy2*dx*ddx)+(Cx2*dy*dydx)+(Cy2*Cx2*depth*dx));
    float Dy=2.f*((Cy2*dx*dydx)+(Cx2*dy*ddy)+(Cy2*Cx2*depth*dy));
    //float dDx = forwardDifference(Dx,D,1.0f);//,h_x);
    //float dDy = forwardDifference(Dy,D,1.0f);//,h_y);
    float Ex = 0.5f*dx*Dx-ddx*D;
    float Ey = 0.5f*dy*Dy-ddy*D;
    float H = (Cy*Ex+Cx*Ey)/(2.f*D*sqrt(D));
    float newDepth = depth + dt*H;
    colorOut = vec4(newDepth,newDepth,newDepth,1.0f);
    gl_FragDepth = newDepth;
}
