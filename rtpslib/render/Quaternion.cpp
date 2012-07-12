#include "Quaternion.h"
#define TOLERANCE 0.00001f
namespace rtps
{
// Convert to Axis/Angles
void Quaternion::getAxisAngle(float3 *axis, float *angle)
{
    //float scale = sqrt(x * x + y * y + z * z);
    axis->x = x;//x / scale;
    axis->y = y;///y / scale;
    axis->z = z;//z / scale;
    axis->normalize();
    *angle = acos(w) * 2.0f;
}

// Convert to Matrix
float16 Quaternion::getMatrix() const
{
    //normalize();
    float x2 = x * x;
    float y2 = y * y;
    float z2 = z * z;
    float xy = x * y;
    float xz = x * z;
    float yz = y * z;
    float wx = w * x;
    float wy = w * y;
    float wz = w * z;


    // This calculation would be a lot more complicated for non-unit length quaternions
    // Note: The constructor of Matrix4 expects the Matrix in column-major format like expected by
    //   OpenGL
    return float16( 1.0f - 2.0f * (y2 + z2), 2.0f * (xy - wz), 2.0f * (xz + wy), 0.0f,
            2.0f * (xy + wz), 1.0f - 2.0f * (x2 + z2), 2.0f * (yz - wx), 0.0f,
            2.0f * (xz - wy), 2.0f * (yz + wx), 1.0f - 2.0f * (x2 + y2), 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);
}

// Convert from Euler Angles
void Quaternion::FromEuler(float pitch, float yaw, float roll)
{
    // Basically we create 3 Quaternions, one for pitch, one for yaw, one for roll
    // and multiply those together.
    // the calculation below does the same, just shorter

    float p = pitch * PIOVER180 / 2.0;
    float y = yaw * PIOVER180 / 2.0;
    float r = roll * PIOVER180 / 2.0;

    float sinp = sin(p);
    float siny = sin(y);
    float sinr = sin(r);
    float cosp = cos(p);
    float cosy = cos(y);
    float cosr = cos(r);

    this->x = sinr * cosp * cosy - cosr * sinp * siny;
    this->y = cosr * sinp * cosy + sinr * cosp * siny;
    this->z = cosr * cosp * siny - sinr * sinp * cosy;
    this->w = cosr * cosp * cosy + sinr * sinp * siny;

    normalize();
}

// Convert from Axis Angle
void Quaternion::FromAxis(const float3 &v, float angle)
{
    float sinAngle;
    angle *= 0.5f;
    float3 vn(v);
    vn.normalize();
    //vn.print("vn");

    sinAngle = sin(angle);

    x = (vn.x * sinAngle);
    y = (vn.y * sinAngle);
    z = (vn.z * sinAngle);
    w = cos(angle);
}

// Multiplying a quaternion q with a vector v applies the q-rotation to v
float3 Quaternion::operator* (const float3 &vec) const
{
    float3 vn(vec);
    vn.normalize();

    Quaternion vecQuat, resQuat;
    vecQuat.x = vn.x;
    vecQuat.y = vn.y;
    vecQuat.z = vn.z;
    vecQuat.w = 0.0f;

    resQuat = vecQuat * getConjugate();
    resQuat = *this * resQuat;
    //resQuat= *this*vecQuat;
    //resQuat= resQuat*getConjugate();

    return (float3(resQuat.x, resQuat.y, resQuat.z));
}

// Multiplying q1 with q2 applies the rotation q2 to q1
Quaternion Quaternion::operator* (const Quaternion &rq) const
{
    //float3 a(x,y,z);
    //float3 b(rq.x,rq.y,rq.z);
    //float3 ans = cross(a,b);
    //ans+= w*b+rq.w*a;
    //float angle = w*rq.w - (a.x*rq.x+a.y*rq.y+a.z*rq.z);

    //return Quaternion(ans,angle);
    // the constructor takes its arguments as (x, y, z, w)
    return Quaternion(w * rq.x + x * rq.w + y * rq.z - z * rq.y,
                      w * rq.y + y * rq.w + z * rq.x - x * rq.z,
                      w * rq.z + z * rq.w + x * rq.y - y * rq.x,
                      w * rq.w - x * rq.x - y * rq.y - z * rq.z);
}

// We need to get the inverse of a quaternion to properly apply a quaternion-rotation to a vector
// The conjugate of a quaternion is the same as the inverse, as long as the quaternion is unit-length
Quaternion Quaternion::getConjugate() const
{
    return Quaternion(-x, -y, -z, w);
}

// normalising a quaternion works similar to a vector. This method will not do anything
// if the quaternion is close enough to being unit-length. define TOLERANCE as something
// small like 0.00001f to get accurate results
void Quaternion::normalize()
{
    // Don't normalize if we don't have to
    float mag2 = w * w + x * x + y * y + z * z;
    if (fabs(mag2) > TOLERANCE && fabs(mag2 - 1.0f) > TOLERANCE) {
        float mag = sqrt(mag2);
        w /= mag;
        x /= mag;
        y /= mag;
        z /= mag;
    }
}
}
