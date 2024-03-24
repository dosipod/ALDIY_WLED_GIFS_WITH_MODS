/*
  FXparticleSystem.cpp

  Particle system with functions for particle generation, particle movement and particle rendering to RGB matrix.
  by DedeHai (Damian Schneider) 2013-2024
  Rendering is based on algorithm by giladaya, https://github.com/giladaya/arduino-particle-sys

  LICENSE
  The MIT License (MIT)
  Copyright (c) 2024  Damian Schneider 
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.

*/


#include <stdint.h>

//particle dimensions (subpixel division)
#define PS_P_RADIUS 64 //subpixel size, each pixel is divided by this for particle movement, if this value is changed, also change the shift defines (next two lines)
#define PS_P_HALFRADIUS 32  
#define PS_P_RADIUS_SHIFT 6 // shift for RADIUS
#define PS_P_SURFACE 12 // shift: 2^PS_P_SURFACE = (PS_P_RADIUS)^2
#define PS_P_HARDRADIUS 80 //hard surface radius of a particle, used for collision detection proximity

//struct for a single particle
typedef struct {
    int16_t x;   //x position in particle system
    int16_t y;   //y position in particle system    
    int8_t vx;  //horizontal velocity
    int8_t vy;  //vertical velocity
    uint16_t ttl; // time to live
    uint8_t hue;  // color hue
    uint8_t sat;  // color saturation
    //add a one byte bit field:
    bool outofbounds : 1; //out of bounds flag, set to true if particle is outside of display area
    bool collide : 1; //if flag is set, particle will take part in collisions
    bool flag2 : 1; // unused flags... could use one for collisions to make those selective.
    bool flag3 : 1;
    uint8_t counter : 4; //a 4 bit counter for particle control
} PSparticle;

//struct for a particle source
typedef struct {
	uint16_t minLife; //minimum ttl of emittet particles
	uint16_t maxLife; //maximum ttl of emitted particles
    PSparticle source; //use a particle as the emitter source (speed, position, color)
    uint8_t var; //variation of emitted speed
    int8_t vx; //emitting speed
    int8_t vy; //emitting speed
} PSpointsource;

#define GRAVITYCOUNTER 2 //the higher the value the lower the gravity (speed is increased every n'th particle update call), values of 1 to 4 give good results
#define MAXGRAVITYSPEED 40 //particle terminal velocity

void Emitter_Flame_emit(PSpointsource *emitter, PSparticle *part);
void Emitter_Fountain_emit(PSpointsource *emitter, PSparticle *part);
void Emitter_Angle_emit(PSpointsource *emitter, PSparticle *part, uint8_t angle, uint8_t speed);
void Particle_attractor(PSparticle *particle, PSparticle *attractor, uint8_t *counter, uint8_t strength, bool swallow);
void Particle_Move_update(PSparticle *part, bool killoutofbounds = false, bool wrapX = false, bool wrapY = false);
void Particle_Bounce_update(PSparticle *part, const uint8_t hardness);
void Particle_Gravity_update(PSparticle *part, bool wrapX, bool bounceX, bool bounceY, const uint8_t hardness);
void ParticleSys_render(PSparticle *particles, uint32_t numParticles, bool wrapX, bool wrapY);
void FireParticle_update(PSparticle *part, uint32_t numparticles, bool wrapX = false);
void ParticleSys_renderParticleFire(PSparticle *particles, uint32_t numParticles, bool wrapX);
void PartMatrix_addHeat(uint8_t col, uint8_t row, uint32_t heat, uint32_t rows);
void detectCollisions(PSparticle *particles, uint32_t numparticles, uint8_t hardness);
void handleCollision(PSparticle *particle1, PSparticle *particle2, const uint32_t hardness);
void applyFriction(PSparticle *particle, int32_t coefficient);
