#ifndef CAMERA_H
#define CAMERA_H
#include "../structs.h"
#include "../rtps_common.h"
#include "Quaternion.h"

// modified from http://content.gpwiki.org/index.php/OpenGL:Tutorials:Using_Quaternions_to_represent_rotation

#define _USE_MATH_DEFINES
#include <math.h>
#define PIOVER180 M_PI/180.0f

enum Projection
{
    PERSPECTIVE_PROJECTION = 0,
    ORTHOGRAPHIC_PROJECTION
};

class RTPS_EXPORT Camera
{
    Camera(float3 pos = float3(0.0f, 0.0f, 0.0f), float fov=65.0f,
           float near = 0.3f, float far = 1000.0f, unsigned int width=600,
           unsigned int height=800)
    {
        this->pos=pos;
        this->fov=fov;
        this->nearClip=near;
        this->farClip=far;
        this->width=width;
        this->height=height;
        updateProjectionMatrix();
        updateModelviewMatrix();
    }

    void moveX(float delX);
    void moveY(float delY);
    void moveZ(float delZ);

    void rotateX(float rotateDelX);
    void rotateY(float rotateDelY);

    //void tick(float seconds);

    void setFOV(double fov){this->fov=fov; updateProjectionMatrix();}
    double getFOV(){return fov;}

    void setNearClip(double near){this->nearClip=near; updateProjectionMatrix();}
    double getNearClip(){return near;}

    void setFarClip(double far){this->far=farClip; updateProjectionMatrix();}
    double getFarClip(){return far;}

    void setWidth(unsigned int width){this->width = width; aspectRatio=width/((double)height); updateProjectionMatrix();}
    unsigned int getWidth(){return width;}

    void setHeight(unsigned int height){this->height=height; aspectRatio=width/((double)height); updateProjectionMatrix();}
    unsigned int getHeight(){return height;}

    void setMoveSpeed(float moveSpeed){this->moveSpeed=moveSpeed;}
    float getMoveSpeed(){return moveSpeed;}

    void setRotateSpeed(float rotateSpeed){this->rotateSpeed=rotateSpeed;}
    float getRotateSpeed(){return rotateSpeed;}

    void setProjectionType(Projection projection)
    {
        if(projection!=currentProjection)
        {
            this->currentProjection=projection;
            updateProjectionMatrix();
        }
    }
    Projection getProjectionType(){return currentProjection;}

    const float16& getProjectionMatrix();
    const float16& getViewMatrix();
    const float16& getInverseViewMatrix();

protected:
    void updateProjectionMatrix();
    void updateViewMatrix();
    void setProjectionMartrixPerspective(float l, float r, float b, float t, float n,
                                         float f);
    void setProjectionMatrixOrtho(float l, float r, float b, float t, float n,
                                  float f);

    float3 pos;
    Quaternion rotation;
    float16 projectionMatrix;
    float16 viewMatrix;
    float16 invViewMatrix;


    double nearClip;
    double farClip;
    double fov,aspectRatio;
    float moveSpeed, rotateSpeed;

    Projection currentProjection;

    unsigned int width;
    unsigned int height;
}

#endif
