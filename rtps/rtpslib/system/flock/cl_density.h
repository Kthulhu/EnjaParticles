#ifndef _DENSITY_UPDATE_CL_
#define _DENSITY_UPDATE_CL_

//if(index_i != index_j){
	
	// positions
	float4 pi = pos(index_i);
	float4 pj = pos(index_j);
	clf[index_i] = position_i; 
	clf[index_i].w = -131.;
	return;

	// velocities
	float4 vj = vel(index_j);

	// number of flockmates
    pt->density.x += 1.f;

	// setup for rule 1. Separation
	// force is the separation vector
    float4 s = r;       //pi - pj;
	float  d = rlen;    //length(s);
	
    if(d < flockp->min_dist){
		s.w = 0.0f;
        s = normalize(s);
        s /= d;
	    pt->force += s * (float)iej;         // accumulate the separation vector
        pt->density.y += 1.f * (float)iej;   // count how many flockmates are with in the separation distance
	}

	// setup for rule 2. alignment
	// surf_tens is the alignment vector
	pt->surf_tens  += vj * (float)iej;   // desired velocity
	pt->surf_tens.w = 1.f;

	// setup for rule 3. cohesion
    // xflock is the cohesion vector
	pt->xflock  += pj * (float)iej; 		// center of the flock
	pt->xflock.w = 1.f;

    float4 k = pi-pj;
    pt->density = (float4)(5., 5., 5., 5.);
    pt->force= (float4)(k.xyz, 1.);
    //pt->force= (float4)(index_i, index_j, 1., 1.);
    //pt->force= (float4)(pj.xyz, 1.);
//}
#endif
