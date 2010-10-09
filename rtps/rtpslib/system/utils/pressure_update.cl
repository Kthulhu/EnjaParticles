#ifndef _PRESSURE_UPDATE_CL_
#define _PRESSURE_UPDATE_CL_

	// gradient
	float dWijdr = Wspiky_dr(rlen, sphp->smoothing_distance, sphp);

	float4 di = density(index_i);  // should not repeat di=
	float4 dj = density(index_j);

	//form simple SPH in Krog's thesis
	float fact = 1.; // 5.812
	// rest density does not appear to be correct. 
	//float Pi = sphp->K*(di.x - fact * sphp->rest_density); 
	//float Pj = sphp->K*(dj.x - fact * sphp->rest_density);
	float Pi = sphp->K*(di.x - 1000.f);
	float Pj = sphp->K*(dj.x - 1000.f);

	float kern = -dWijdr * (Pi + Pj)*0.5;
	float4 stress = kern*r;

	#if 1
	// Add viscous forces

	float4 veli = vel(index_i);
	float4 velj = vel(index_j);

	//float vvisc = 0.001f; // SHOULD BE SET IN GE_SPH.cpp
	float vvisc = 1.000f; // SHOULD BE SET IN GE_SPH.cpp
	float dWijlapl = Wvisc_lapl(rlen, sphp->smoothing_distance, sphp);
	stress += vvisc * (velj-veli) * dWijlapl;
	stress *=  sphp->mass/(di.x*dj.x);  // original
	#endif


	#if 1
	// Add XSPH stabilization term
	float Wijpol6 = Wpoly6(rlen, sphp->smoothing_distance, sphp);
	stress +=  (2.f * sphp->mass * (velj-veli)/(di.x+dj.x) 
	    * Wijpol6);
	#endif

	pt->force += stress;

	//return stress;

#endif
