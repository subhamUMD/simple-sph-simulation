#pragma once
#ifndef SPH_PLANE_INTERACTOR_3D_H
#define SPH_PLANE_INTERACTOR_3D_H

#include "SPHInteractor3d.h"
#include "GlmVec.h"

struct SPHParticle3d;

class SPHPlaneInteractor3d : public SPHInteractor3d
{
	glm::vec3 up;
	glm::vec3 start;
	float distanceSquared;
	float distance;

public:
	SPHPlaneInteractor3d( glm::vec3 startOffset, glm::vec3 upNormal, float distance = 0.1 );

	void applyDensity( SPHParticle3d& other, glm::vec3 rvec );
	void applyForce( SPHParticle3d& other, glm::vec3 rvec );
	void enforceInteractor( SPHParticle3d& other, glm::vec3 rvec );
	glm::vec3 directionTo( SPHParticle3d& other );
	void draw();
};

#endif
