#ifndef QUATERNION_H
#define QUATERNION_H

//Original code taken from http://content.gpwiki.org/index.php/OpenGL:Tutorials:Using_Quaternions_to_represent_rotation

#include "../structs.h"
#include "../rtps_common.h"


class RTPS_EXPORT Quaternion
{

public:
    Quaternion(){x=y=z=0.0f; w=1.0f;}

    // Convert to Axis/Angles
    void getAxisAngle(float3 *axis, float *angle);

    // Convert to Matrix
    float16 getMatrix() const;

    // Convert from Euler Angles
    void FromEuler(float pitch, float yaw, float roll);

    // Convert from Axis Angle
    void FromAxis(const float3 &v, float angle);

    // Multiplying a quaternion q with a vector v applies the q-rotation to v
    float3 operator* (const float3 &vec) const;

    // Multiplying q1 with q2 applies the rotation q2 to q1
    Quaternion operator* (const Quaternion &rq) const;

    // We need to get the inverse of a quaternion to properly apply a quaternion-rotation to a vector
    // The conjugate of a quaternion is the same as the inverse, as long as the quaternion is unit-length
    Quaternion getConjugate();

    // normalising a quaternion works similar to a vector. This method will not do anything
    // if the quaternion is close enough to being unit-length. define TOLERANCE as something
    // small like 0.00001f to get accurate results
    void normalize();
private:
    float x,y,z,w;
};

#endif // QUATERNION_H
