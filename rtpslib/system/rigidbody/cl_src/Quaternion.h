/*
 * File:   Quaternion.h
 * Taken from Bullet experiments. Copyright belongs to 2011 Takahiro Harada.
 *
 * Created on November 14, 2011, 11:54 AM
 */

#ifndef QUATERNION_H
#define	QUATERNION_H

typedef float4 Quaternion;

__inline
float dot3F4(const float4 a, const float4 b)
{
	return a.x*b.x+a.y*b.y+a.z*b.z;
}

__inline
float4 normalize3(const float4 a)
{
	float length = sqrt(dot3F4(a, a));
	return length==0.?(float4)(0.,0.,0.,0.):1.f/length * a;
}

__inline
Quaternion qtSet(const float4 axis, float angle)
{
	float4 nAxis = normalize3( axis );
    //float4 nAxis = fast_normalize(axis);

	Quaternion q;
	q.x = nAxis.x*sin(angle/2);
	q.y = nAxis.y*sin(angle/2);
	q.z = nAxis.z*sin(angle/2);
	q.w = cos(angle/2);
	return q;
}
__inline float4 cross3F4( float4 a, float4 b)
{
	float4 retval;
	retval.x=a.y*b.z-a.z*b.y;
	retval.y=a.z*b.x-a.x*b.z;
	retval.z=a.x*b.y-a.y*b.x;
	retval.w=0.0f;
	return retval;
}

__inline
Quaternion qtMul(const Quaternion a, const Quaternion b)
{
	Quaternion ans;
	ans = cross3F4( a, b );
	ans += a.w*b + b.w*a;
	ans.w = a.w*b.w - (a.x*b.x+a.y*b.y+a.z*b.z);
	return ans;
}

__inline
float16 qtGetRotationMatrix(const Quaternion quat)
{
	float4 quat2 = (float4)(quat.x*quat.x, quat.y*quat.y, quat.z*quat.z, 0.f);
	float16 out;

	out.s0=1-2*quat2.y-2*quat2.z;
	out.s1=2*quat.x*quat.y-2*quat.w*quat.z;
	out.s2=2*quat.x*quat.z+2*quat.w*quat.y;
	out.s3 = 0.f;

	out.s4=2*quat.x*quat.y+2*quat.w*quat.z;
	out.s5=1-2*quat2.x-2*quat2.z;
	out.s6=2*quat.y*quat.z-2*quat.w*quat.x;
	out.s7 = 0.f;

	out.s8=2*quat.x*quat.z-2*quat.w*quat.y;
	out.s9=2*quat.y*quat.z+2*quat.w*quat.x;
	out.sa=1-2*quat2.x-2*quat2.y;
	out.sb = 0.f;

    out.sc = 0.f;
    out.sd = 0.f;
    out.se = 0.f;
    out.sf = 0.f;

	return out;
}



#endif	/* QUATERNION_H */

