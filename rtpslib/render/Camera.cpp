#include "Camera.h"


void Camera::moveX(float delX)
{
    pos += rotation * float3(delX*moveSpeed, 0.0f, 0.0f);
}

void Camera::moveY(float delY)
{
    pos.y -= delY*moveSpeed;
}

void Camera::moveZ(float delZ)
{
    pos += rotation * float3(0.0f, 0.0f, -delZ*moveSpeed);
}

void Camera::rotateX(float rotateDelX)
{
    Quaternion nrot(float3(1.0f, 0.0f, 0.0f), rotateDelX * rotateSpeed * PIOVER180);
    rotation = rotation * nrot;
}

void Camera::rotateY(float rotateDelY)
{
    Quaternion nrot(float3(0.0f, 1.0f, 0.0f), rotateDelX * rotateSpeed * PIOVER180);
    rotation = nrot * rotation;
}

/*void Camera::tick(float seconds)
{
    if (rot.x != 0.0f) rotateX(rot.x * seconds * rotspeed);
    if (rot.y != 0.0f) rotateY(rot.y * seconds * rotspeed);

    if (move.x != 0.0f) moveX(move.x * seconds * movespeed);
    if (move.y != 0.0f) moveY(move.y * seconds * movespeed);
    if (move.z != 0.0f) moveZ(move.z * seconds * movespeed);
    if((rot.x != 0.0f)||(rot.y != 0.0f)||(move.x != 0.0f)||(move.y != 0.0f)||(move.z != 0.0f))
            updateModelviewMatrix();
}*/
void Camera::setProjectionMatrixPerspective(float l, float r, float b, float t, float n, float f)
{
    projectionMatrix.loadIdentity();
    projectionMatrix[0]  = 2 * n / (r - l);
    projectionMatrix[2]  = (r + l) / (r - l);
    projectionMatrix[5]  = 2 * n / (t - b);
    projectionMatrix[6]  = (t + b) / (t - b);
    projectionMatrix[10] = -(f + n) / (f - n);
    projectionMatrix[11] = -(2 * f * n) / (f - n);
    projectionMatrix[14] = -1;
    projectionMatrix[15] = 0;
}

void Camera::setProjectionMatrixOrtho(float l, float r, float b, float t, float n,
                              float f)
{
    projectionMatrix.loadIdentity();
    projectionMatrix[0]  = 2 / (r - l);
    projectionMatrix[3]  = -(r + l) / (r - l);
    projectionMatrix[5]  = 2 / (t - b);
    projectionMatrix[7]  = -(t + b) / (t - b);
    projectionMatrix[10] = -2 / (f - n);
    projectionMatrix[11] = -(f + n) / (f - n);
}

const float16& Camera::getProjectionMatrix()
{
    return projectionMatrix;
}

void Camera::updateProjectionMatrix()
{
    if(currentProjection==PERSPECTIVE_PROJECTION)
    {
        double tangent = tanf(fov/2 * PIOVER180); // tangent of half fovY
        double h = nearClip * tangent;         // half height of near plane
        double w = h * aspectRatio;          // half width of near plane

        setPerspectiveFrustum(-w, w, -h, h, nearClip, farClip);
    }
    else
    {
        setProjectionMatrixOrthographic(-width,width,height,height,nearClip,farClip)
    }
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
    viewMatrix = rotation.getMatrix();
    viewMatrix[3]=pos.x;
    viewMatrix[7]=pos.y;
    viewMatrix[11]=pos.z;
    invViewMatrix=viewMatrix;
    invViewMatrix.inverse();
}
