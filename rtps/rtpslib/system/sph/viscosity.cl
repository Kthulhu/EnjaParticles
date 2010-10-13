#define STRINGIFY(A) #A

//do the SPH pressure calculations and update the force
std::string viscosity_program_source = STRINGIFY(

typedef struct SPHParams
{
    float4 grid_min;            //float3s are really float4 in opencl 1.0 & 1.1
    float4 grid_max;            //so we have padding in C++ definition
    float mass;
    float rest_distance;
    float smoothing_distance;
    float simulation_scale;
    float boundary_stiffness;
    float boundary_dampening;
    float boundary_distance;
    float EPSILON;
    float PI;       //delicious
    float K;        //speed of sound
 
} SPHParams;


float magnitude(float4 vec)
{
    return sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
}
float dist_squared(float4 vec)
{
    return vec.x*vec.x + vec.y*vec.y + vec.z*vec.z;
}

       
__kernel void viscosity(__global float4* pos, __global float* vel, __global float* veleval, __global float4* density, __global float4* force, __constant struct SPHParams* params)
{
    unsigned int i = get_global_id(0);

    float4 p = pos[i] * params->simulation_scale;
    float4 v = veleval[i] * params->simulation_scale;
    float4 f = (float4)(0.0f, 0.0f, 0.0f, 0.0f);

    //obviously this is going to be passed in as a parameter (rather we will use neighbor search)
    int num = get_global_size(0);
    float h = params->smoothing_distance;

    //stuff from Tim's code (need to match #s to papers)
    //float alpha = 315.f/208.f/params->PI/h/h/h;
    float h6 = h*h*h * h*h*h;
    float alpha = 45.f/params->PI/h6;

    float4 di = density[i];

    //super slow way, we need to use grid + sort method to get nearest neighbors
    //this code should never see the light of day on a GPU... just sayin
    for(int j = 0; j < num; j++)
    {
        if(j == i) continue;
        float4 dj = density[j];
        float4 pj = pos[j] * params->simulation_scale;
        float4 vj = veleval[j] * params->simulation_scale;
        float4 r = p - pj;
        float rlen = magnitude(r);
        if(rlen < h)
        {
            float r2 = rlen*rlen;
            float re2 = h*h;
            if(r2/re2 <= 4.f)
            {
                //float R = sqrt(r2/re2);
                //float Wij = alpha*(-2.25f + 2.375f*R - .625f*R*R);
                float Wij = alpha * (h - rlen);
                //from tim's code
                //form simple SPH in Krog's thesis
                f += params->mass * Wij * (vj - v) / (di * dj);
            }

        }
    }
    force[i] += f;

}
);

