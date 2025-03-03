#pragma once
#ifndef SMOOTHING_KERNELS_H
#define SMOOTHING_KERNELS_H

#include "GlmVec.h"
#include <string>
#include <memory>

enum KernelType
{
	POLY6,
	SPIKY,
	VISCOSITY,
	DEFAULT
};

enum SPHKernelUse
{
	BASE_KERNEL,		// Used in density update and surface tension forces
	PRESSURE_KERNEL,	// Used in pressure forces calculations
	VISCOSITY_KERNEL	// Used in viscosity forces calculation
};

class iKernel
{
protected:
	float baseFactor;
	float gradientFactor;
	float laplacianFactor;
	float h;

public:
	typedef std::unique_ptr<iKernel> unique;

	virtual float base( float r )=0;
	virtual glm::vec2 gradient( glm::vec2 r )=0;
	virtual glm::vec3 gradient( glm::vec3 r )=0;
	virtual float laplacian( float r )=0;
	virtual void  adjustSmoothingLength( float h )=0;
};

class KernelPoly6 : public iKernel
{	
	float hSquared;

public:
	KernelPoly6( float smoothingLength );

	float base( float r );
	glm::vec2 gradient( glm::vec2 r );
	glm::vec3 gradient( glm::vec3 r );
	float laplacian( float r );
	void  adjustSmoothingLength( float h );
};

class KernelSpiky : public iKernel
{
public:
	KernelSpiky( float smoothingLength );

	float base( float r );
	glm::vec2 gradient( glm::vec2 r );
	glm::vec3 gradient( glm::vec3 r );
	float laplacian( float r );
	void  adjustSmoothingLength( float h );
};

class KernelViscosity : public iKernel
{
public:
	KernelViscosity( float smoothingLength );

	float base( float r );
	glm::vec2 gradient( glm::vec2 r );
	glm::vec3 gradient( glm::vec3 r );
	float laplacian( float r );
	void  adjustSmoothingLength( float h );
};

class KernelBuilder
{
public:
	static iKernel::unique getKernel( std::string name, float smoothingLength );
	static iKernel::unique getKernel( KernelType type, float smoothingLength );
};

#endif