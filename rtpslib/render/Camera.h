#ifndef CAMERA_H
#define CAMERA_H
#include "../structs.h"
#include "../rtps_common.h"
//#include "Quaternion.h"
#define _USE_MATH_DEFINES
#include <math.h>
#define PIOVER180 M_PI/180.0f
#define PI2 2.f*M_PI
// modified from http://content.gpwiki.org/index.php/OpenGL:Tutorials:Using_Quaternions_to_represent_rotation

namespace rtps
{
enum Projection
{
    PERSPECTIVE_PROJECTION = 0,
    ORTHOGRAPHIC_PROJECTION
};
class RTPS_EXPORT Camera
{
public:
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
        aspectRatio=(double)width/(double)height;
        moveSpeed=1.0f;
        rotateSpeed=1.0f;
        up=float3(0.0f,1.0f,0.0f);
        forward=float3(0.0f,0.0f,-1.0f);
        left=float3(-1.0f,0.0f,0.0f);
        currentProjection=PERSPECTIVE_PROJECTION;
        updateProjectionMatrix();
        updateViewMatrix();
    }

    void move(float delX,float delY,float delZ);
    //void moveX(float delX);
    //void moveY(float delY);
    //void moveZ(float delZ);


    void rotate(float rotateDelX, float rotateDelY);

    bool tick(float seconds);

    void setFOV(double fov){this->fov=fov; updateProjectionMatrix();}
    double getFOV(){return fov;}

    void setNearClip(double near){this->nearClip=near; updateProjectionMatrix();}
    double getNearClip(){return nearClip;}

    void setFarClip(double far){this->farClip=far; updateProjectionMatrix();}
    double getFarClip(){return farClip;}

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
    const float16& getInverseProjectionMatrix();
    const float16& getViewMatrix();
    const float16& getInverseViewMatrix();

protected:
    void rotateX(float radiansX);
    void rotateY(float radiansY);
    void move(float3 delta);
    void updateProjectionMatrix();
    void updateViewMatrix();
    void setProjectionMatrixPerspective(double l, double r, double b, double t, double n,
                                         double f);
    void setProjectionMatrixOrthographic(double l, double r, double b, double t, double n,
                                  double f);

    float3 pos;
    float3 rotationAngles;
    //Quaternion rotation;
    float3 up;
    float3 forward;
    float3 left;

    float3 movement;
    float3 rotation;


    float16 projectionMatrix;
    float16 invProjectionMatrix;
    float16 viewMatrix;
    float16 invViewMatrix;

    double nearClip;
    double farClip;
    double fov,aspectRatio;
    float moveSpeed, rotateSpeed;

    Projection currentProjection;

    unsigned int width;
    unsigned int height;
};
};

#endif
