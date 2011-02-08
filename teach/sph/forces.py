import pygame
from pygame.locals import *

from kernels import *
from vector import Vec



def density_update(sphp, particles):
    #brute force
    for pi in particles:
        pi.dens = 0.
        for pj in particles:
            r = pi.pos - pj.pos
            #print r
            pi.dens += pj.mass*Wpoly6(pi.h, r)

def force_update(sphp, particles):
    #brute force
    rho0 = sphp.rho0
    K = sphp.K

    for pi in particles:
        pi.force = Vec([0.,0.])
        pi.xsph = Vec([0.,0.])
        di = pi.dens
        #print "di", di
        Pi = K*(di - rho0)
        #print "Pi", Pi
        #print "pi.h", pi.h
        for pj in particles:
            if pj == pi:
                continue
            r = pi.pos - pj.pos
            #print "r", r

            dj = pj.dens
            Pj = K*(dj - rho0)

            #print "dWspiky", dWspiky(pi.h, r)
            kern = .5 * (Pi + Pj) * dWspiky(pi.h, r)
            f = r*kern
            #does not account for variable mass
            f *= pi.mass / (di + dj)

            #print "force", f
            pi.force += f

            #XSPH
            #float4 xsph = (2.f * sphp->mass * Wijpol6 * (velj-veli)/(di.x+dj.x));
            xsph = pj.veleval - pi.veleval
            xsph *= 2. * pi.mass * Wpoly6(pi.h, r) / (di + dj) 
            pi.xsph += xsph


        #print "pi.force", pi.force
  


def calcRepulsionForce(normal, vel, sphp, distance):
    repulsion_force = (sphp.boundary_stiffness * distance - sphp.boundary_dampening * np.dot(normal, vel))*normal;
    return repulsion_force;

def calcFrictionForce(v, f, normal, friction_kinetic, friction_static_limit):
    pass


def collision_wall(sphp, domain, particles):
    
    dmin = domain.dmin * sphp.sim_scale
    dmax = domain.dmax * sphp.sim_scale
    bnd_dist = sphp.boundary_distance
    #float diff = params->boundary_distance - (p.z - gp->bnd_min.z);
    #if (diff > params->EPSILON)
    #r_f += calculateRepulsionForce(normal, v, params->boundary_stiffness, params->boundary_dampening, diff);
    #f_f += calculateFrictionForce(v, f, normal, friction_kinetic, friction_static_limit);
   

    for pi in particles:
        f = Vec([0.,0.])
        p = pi.pos
        #X Min
        diff = bnd_dist - (p.x - dmin.x)
        if diff > 0:
            normal = Vec([1.,0.])
            f += calcRepulsionForce(normal, pi.vel, sphp, diff)
            #calcFrictionForce(pi.v, pi.f, normal, friction_kinetic, friction_static_limit)
        #X Max
        diff = bnd_dist - (dmax.x - p.x)
        if diff > 0:
            normal = Vec([-1.,0.])
            f += calcRepulsionForce(normal, pi.vel, sphp, diff)
            #calcFrictionForce(pi.v, pi.f, normal, friction_kinetic, friction_static_limit)
        #Y Min
        diff = bnd_dist - (p.y - dmin.y)
        if diff > 0:
            normal = Vec([0.,1.])
            f += calcRepulsionForce(normal, pi.vel, sphp, diff)
            #calcFrictionForce(pi.v, pi.f, normal, friction_kinetic, friction_static_limit)
        #Y Max
        diff = bnd_dist - (dmax.y - p.y)
        if diff > 0:
            normal = Vec([0.,-1.])
            f += calcRepulsionForce(normal, pi.vel, sphp, diff)
            #calcFrictionForce(pi.v, pi.f, normal, friction_kinetic, friction_static_limit)

        #print "collision force", f
        pi.force += f




def euler_update(sphp, particles):
    dt = .001

    for pi in particles:
        f = pi.force

        f.y += -9.8

        speed = mag(f);
        #print "speed", speed
        if speed > sphp.velocity_limit:
            f *= sphp.velocity_limit/speed;

        
    
        #print "force", f
        pi.vel += f * dt
        pi.pos += pi.vel * dt


def leapfrog_update(sphp, particles):
    dt = .001

    for pi in particles:
        f = pi.force

        f.y += -9.8

        speed = mag(f);
        #print "speed", speed
        if speed > sphp.velocity_limit:
            f *= sphp.velocity_limit/speed;
    
        #print "force", f
        vnext = pi.vel + f * dt
        vnext += sphp.xsph_factor * pi.xsph
        pi.pos += pi.vel * dt

        veval = .5 * (pi.vel + vnext)
        pi.vel = vnext
        pi.veleval = veval


