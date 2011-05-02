import pygame
from pygame.locals import *

from forces import *
from vector import Vec
import sph
from hash import Domain
#from domain import Domain
#from timing import Timing, timing_collector

#timings = {}
#timing_coroutine = timing_collector(timings)
#timing_coroutine.next()

#@Timing(timing_coroutine)

#@timings
def fromscreen(p, surface):
    #v.x
    p.y = surface.get_height() - p.y
    return p


def toscreen(p, surface, screen_scale):
    translate = Vec([0,0])
    p.x = translate.x + p.x*screen_scale
    p.y = surface.get_height() - (translate.y + p.y*screen_scale)
    return p



#@print_timing
#@Timing(timing_coroutine)
@timings
def draw_particles(ps):
    for p in ps:
        p.draw()


def main():
    pygame.init()
    screen = pygame.display.set_mode((800, 800))
    pygame.display.set_caption('SPH Forces')

    background = pygame.Surface(screen.get_size())
    background = background.convert()
    background.fill((250, 250, 250))

    clock = pygame.time.Clock()

    max_num = 2**12 #4096
    #max_num = 2**10 #1024
    #max_num = 2**8 #256
    #max_num = 2**7 #128
    
    dmin = Vec([0,0,0])
    dmax = Vec([5,5,5])
    domain = Domain(dmin, dmax, screen)
    system = sph.SPH(max_num, domain)

    particles = sph.init_particles(50, system, domain, screen)




    mouse_down = False
    while 1:
        clock.tick(60)
        key = pygame.key.get_pressed()
        for event in pygame.event.get():
            if event.type == QUIT or key[K_ESCAPE] or key[K_q]:
                print "quit!"
                return
            elif key[K_t]:
                print timings
            elif event.type == MOUSEBUTTONDOWN:
                mouse_down = True
            elif event.type == MOUSEMOTION:
                if(mouse_down):
                    v = Vec([event.pos[0], event.pos[1]])
                    v = fromscreen(v, screen)
                    particles[0].move(v)
            elif event.type == MOUSEBUTTONUP:
                mouse_down = False

        
        screen.blit(background, (0, 0))

        density_update(system, particles)
        force_update(system, particles)
        collision_wall(system, domain, particles)
        #euler_update(system, particles)
        leapfrog_update(system, particles)

        draw_particles(particles)
        pygame.display.flip()

if __name__ == "__main__":
    main()