#include "Camera.h"

namespace rtps
{
void Camera::move(float delX, float delY, float delZ)
{
    /*if(delX!=0.0f)
        pos += rotation * float3(delX*moveSpeed, 0.0f, 0.0f);
    if(delY!=0.0f)
        pos += rotation * float3(0.0f,delY*moveSpeed, 0.0f);
        //pos.y-=delY*moveSpeed;
    if(delZ!=0.0f)
        pos += rotation * float3(0.0f, 0.0f, delZ*moveSpeed);*/
    //updateViewMatrix();
    movement=movement+float3(delX,delY,delZ);
}

void Camera::rotateX(float radiansX)
{
    float3 v1 = up*(-(float)sin(radiansX));   //-sinθ.Y
    float3 v2 = forward* (float)cos(radiansX);   // cosθ.Z
    forward = (v1+v2);//.normalize();
    forward.normalize();

    up = cross(forward, left);
    up.normalize();
}

void Camera::rotateY(float radiansY)
{
    //Projection in the x/z plane of the fixed coordinate system
    /*
    float3 forwardProjected = forward;
    forwardProjected.y=0.0f;
    double mag = magnitude(forwardProjected);
    forwardProjected.normalize();
    float3 leftProjected = cross(float3(0.0f,1.0f,0.0f), forwardProjected);
    leftProjected.normalize();

    //Z' = sinθ.X+cosθ.Z
    float3 v1 = leftProjected*(float)sin(radiansY);
    float3 v2 = forwardProjected*(float)cos(radiansY);
    float3 v = v1+v2;
    //v = magnitude(v, magnitude);
    v = v*mag;
    v.y = forward.y;
    forward=v;
    forward.normalize();

    //X' = cosθ.X-sinθ.Z
    v1 = leftProjected* (float)cos(radiansY);
    v2 = forwardProjected* -(float)sin(radiansY);
    //setLeft(normalize(add(v1, v2)));
    left=v1+v2;
    left.normalize();

    //Y' = Z ^ X
    //setUp(normalize(crossProduct(getForward(), getLeft())));
    up=cross(forward,left);
    up.normalize();
    */
    float3 v1 = (left * (float)sin(radiansY));   //sinθ.X
    float3 v2 = (forward* (float)cos(radiansY));   //cosθ.Z
    forward = (v1+v2);
    forward.normalize();

    left = cross(up, forward);
    left.normalize();
}

void Camera::move(float3 delta)
{
    pos.x+=delta.x*left.x + delta.y*up.x + delta.z*forward.x;
    pos.y+=delta.x*left.y + delta.y*up.y + delta.z*forward.y;
    pos.z+=delta.x*left.z + delta.y*up.z + delta.z*forward.z;
}

/*float3 Camera::toFixedCoordinates(float3 delta)
{
    float3 retval;
    retval.x += (float)( delta.x*cos(rotationAngles.y) + delta.y*sin(rotationAngles.x)*sin(rotationAngles.y) - delta.z*cos(rotationAngles.x)*sin(rotationAngles.y) );
    retval.y += (float)(              + delta.y*cos(rotationAngles.x)           + delta.z*sin(rotationAngles.x)           );
    retval.z += (float)( delta.x*sin(rotationAngles.y) - delta.y*sin(rotationAngles.x)*cos(rotationAngles.y) + delta.z*cos(rotationAngles.x)*cos(rotationAngles.y) );
    return retval;
}*/

void Camera::rotate(float rotateDelX, float rotateDelY)
{

    rotationAngles.x+=rotateDelX*PIOVER180;
    rotationAngles.y+=rotateDelY*PIOVER180;
    //float normalizeAngle
    //if(rotationAngles.x>2.f*M_PI)
}

void Camera::setProjectionMatrixPerspective(double l, double r, double b, double t, double n, double f)
{
    projectionMatrix.loadIdentity();
    projectionMatrix[0]  = (2. * n) / (r - l);
    projectionMatrix[2]  = (r + l) / (r - l);
    projectionMatrix[5]  = (2. * n) / (t - b);
    projectionMatrix[6]  = (t + b) / (t - b);
    projectionMatrix[10] = -(f + n) / (f - n);
    projectionMatrix[11] = -(2. * f * n) / (f - n);
    projectionMatrix[14] = -1.;
    projectionMatrix[15] = 0;
    projectionMatrix.transpose();
}

void Camera::setProjectionMatrixOrthographic(double l, double r, double b, double t, double n,
                              double f)
{
    projectionMatrix.loadIdentity();
    projectionMatrix[0]  = 2. / (r - l);
    projectionMatrix[3]  = -(r + l) / (r - l);
    projectionMatrix[5]  = 2. / (t - b);
    projectionMatrix[7]  = -(t + b) / (t - b);
    projectionMatrix[10] = -2. / (f - n);
    projectionMatrix[11] = -(f + n) / (f - n);
    projectionMatrix.transpose();
}

bool Camera::tick(float seconds)
{
    bool wasUpdated=false;
    //up = toFixedCoordinates(float3(0.f,1.f,0.f));
    //forward = toFixedCoordinates(float3(0.f,0.f,1.f));
    //dout<<"Seconds elapsed = "<<seconds<<std::endl;
    //rotationAngles.print("rotation in radians");
    //movement.print("movement");
    if(rotationAngles.x!=0.0f)
    {
        wasUpdated=true;
        float delta = seconds*rotateSpeed*rotationAngles.x;
        if(delta>rotationAngles.x)
            delta=rotationAngles.x;
        rotateX(delta);
        rotationAngles.x-=delta;
    }
    if(rotationAngles.y!=0.0f)
    {
        wasUpdated=true;
        float delta = seconds*rotateSpeed*rotationAngles.y;
        if(delta>rotationAngles.y)
            delta=rotationAngles.y;
        rotateY(delta);
        rotationAngles.y-=delta;
    }
    if(movement.x!=0.0f||movement.y!=0.0f||movement.z!=0.0f)
    {
        wasUpdated=true;
        float3 delta=movement*seconds*moveSpeed;
        if(magnitude(delta)>magnitude(movement))
            delta=movement;
        move(delta);
        movement-=delta;
    }
        //pos=pos+toFixedCoordinates(movement*seconds*moveSpeed);
    if(wasUpdated)
    {
        updateViewMatrix();
        return true;
    }
    return false;
}

const float16& Camera::getProjectionMatrix()
{
    return projectionMatrix;
}
const float16& Camera::getInverseProjectionMatrix()
{
    return invViewMatrix;
}

void Camera::updateProjectionMatrix()
{
    if(currentProjection==PERSPECTIVE_PROJECTION)
    {
        double tangent = tanf(fov *0.5 * PIOVER180); // tangent of half fovY
        double h = nearClip * tangent;         // half height of near plane
        double w = h * aspectRatio;          // half width of near plane

        setProjectionMatrixPerspective(-w, w, -h, h, nearClip, farClip);
    }
    else
    {
        setProjectionMatrixOrthographic(-width,width,-height,height,nearClip,farClip);
    }
    invProjectionMatrix = projectionMatrix;
    invProjectionMatrix.inverse();
}

const float16& Camera::getViewMatrix()
{
    return viewMatrix;
}

const float16& Camera::getInverseViewMatrix()
{
    return invViewMatrix;
}

void Camera::updateViewMatrix()
{
    //rotation.normalize();
    //viewMatrix = rotation.getMatrix();
    //FIXME: might not have to transpose.
    //viewMatrix.transpose();
    viewMatrix.loadIdentity();
    viewMatrix[0]=left.x;
    viewMatrix[4]=left.y;
    viewMatrix[8]=left.z;

    viewMatrix[1]=up.x;
    viewMatrix[5]=up.y;
    viewMatrix[9]=up.z;

    viewMatrix[2]=-forward.x;
    viewMatrix[6]=-forward.y;
    viewMatrix[10]=-forward.z;

    //viewMatrix[12]=-left.x*pos.x-up.x*pos.y+forward.x*pos.z;
    //viewMatrix[13]=-left.y*pos.x-up.y*pos.y+forward.y*pos.z;
    //viewMatrix[14]=-left.z*pos.x-up.z*pos.y+forward.z*pos.z;
    viewMatrix[12]=-viewMatrix[0]*pos.x-viewMatrix[4]*pos.y-viewMatrix[8]*pos.z;
    viewMatrix[13]=-viewMatrix[1]*pos.x-viewMatrix[5]*pos.y-viewMatrix[9]*pos.z;
    viewMatrix[14]=-viewMatrix[2]*pos.x-viewMatrix[6]*pos.y-viewMatrix[10]*pos.z;
    //viewMatrix[12]=-pos.x;
    //viewMatrix[13]=-pos.y;
    //viewMatrix[14]=pos.z;
    //float16 tmpPos;
    //tmpPos[12]=-pos.x;
    //tmpPos[13]=-pos.y;
    //tmpPos[14]=-pos.z;
    //viewMatrix=viewMatrix*tmpPos;
    //viewMatrix[3]=pos.x;
    //viewMatrix[7]=pos.y;
    //viewMatrix[11]=pos.z;
    invViewMatrix=viewMatrix;
    invViewMatrix.inverse();
    //invViewMatrix.print("inverseViewMatrix");
    //viewMatrix.inverse();
    //viewMatrix.print("viewMatrix");
}
}
