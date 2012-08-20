#include "StereoCamera.h"

namespace rtps
{

const float16& StereoCamera::getProjectionMatrixRight()
{
    return projectionMatrixRight;
}
const float16& StereoCamera::getInverseProjectionMatrixRight()
{
    return invViewMatrixRight;
}

void StereoCamera::updateProjectionMatrix()
{
    if(currentProjection==PERSPECTIVE_PROJECTION)
    {
        double tangent = tanf(fov *0.5 * PIOVER180); // tangent of half fovY
        double h = nearClip * tangent;         // half height of near plane
        double w = h * aspectRatio;          // half width of near plane
        double frustumshift = (IOD/2.0)*nearClip/screenZ;

        //create right eye
        setProjectionMatrixPerspective(-w-frustumshift, w-frustumshift, -h, h, nearClip, farClip);
        projectionMatrixRight=projectionMatrix;
        //create left eye
        setProjectionMatrixPerspective(-w+frustumshift, w+frustumshift, -h, h, nearClip, farClip);
    }
    else
    {
        setProjectionMatrixOrthographic(-width,width,-height,height,nearClip,farClip);
    }
    invProjectionMatrix = projectionMatrix;
    invProjectionMatrix.inverse();
    invProjectionMatrixRight = projectionMatrixRight;
    invProjectionMatrixRight.inverse();
}

const float16& StereoCamera::getViewMatrixRight()
{
    return viewMatrixRight;
}

const float16& StereoCamera::getInverseViewMatrixRight()
{
    return invViewMatrixRight;
}

void StereoCamera::updateViewMatrix()
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

    viewMatrixRight=viewMatrix;

    //viewMatrix[12]=-left.x*pos.x-up.x*pos.y+forward.x*pos.z;
    //viewMatrix[13]=-left.y*pos.x-up.y*pos.y+forward.y*pos.z;
    //viewMatrix[14]=-left.z*pos.x-up.z*pos.y+forward.z*pos.z;
    float3 posLeft = pos;
    posLeft= posLeft-IOD/2.;
    viewMatrix[12]=-viewMatrix[0]*posLeft.x-viewMatrix[4]*posLeft.y-viewMatrix[8]*posLeft.z;
    viewMatrix[13]=-viewMatrix[1]*posLeft.x-viewMatrix[5]*posLeft.y-viewMatrix[9]*posLeft.z;
    viewMatrix[14]=-viewMatrix[2]*posLeft.x-viewMatrix[6]*posLeft.y-viewMatrix[10]*posLeft.z;

    float3 posRight = pos;
    posRight= posRight + IOD/2.;
    viewMatrixRight[12]=-viewMatrixRight[0]*posRight.x-viewMatrixRight[4]*posRight.y-viewMatrixRight[8]*posRight.z;
    viewMatrixRight[13]=-viewMatrixRight[1]*posRight.x-viewMatrixRight[5]*posRight.y-viewMatrixRight[9]*posRight.z;
    viewMatrixRight[14]=-viewMatrixRight[2]*posRight.x-viewMatrixRight[6]*posRight.y-viewMatrixRight[10]*posRight.z;
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
    invViewMatrixRight=viewMatrixRight;
    invViewMatrixRight.inverse();
    //invViewMatrix.print("inverseViewMatrix");
    //viewMatrix.inverse();
    //viewMatrix.print("viewMatrix");
}
}
