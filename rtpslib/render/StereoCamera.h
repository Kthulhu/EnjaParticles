#ifndef STEREO_CAMERA_H
#define STEREO_CAMERA_H
#include "../rtps_common.h"
#include "Camera.h"

namespace rtps
{

class RTPS_EXPORT StereoCamera : public Camera
{
public:
    StereoCamera(float3 pos = float3(0.0f, 0.0f, 0.0f), float fov=65.0f,
           float n = 0.3f, float f = 1000.0f, unsigned int width=600,
                 unsigned int height=800, double screenDepth = 10.0,double eyeSeperation=0.5):Camera(pos,fov,n,f,width,height)
      ,screenZ(screenDepth),IOD(eyeSeperation)
    {

    }

    const float16& getProjectionMatrixRight();
    const float16& getInverseProjectionMatrixRight();
    const float16& getViewMatrixRight();
    const float16& getInverseViewMatrixRight();


protected:
    void updateProjectionMatrix();
    void updateViewMatrix();

    double screenZ;                                     //screen projection plane
    double IOD;                                          //intraocular distance

    float16 projectionMatrixRight;
    float16 invProjectionMatrixRight;
    float16 viewMatrixRight;
    float16 invViewMatrixRight;
};
};

#endif
