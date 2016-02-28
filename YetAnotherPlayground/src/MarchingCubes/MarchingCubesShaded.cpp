
#include "MarchingCubesShaded.h"
#include "MappedData.h"
#include "ShaderProgram.h"
#include "MarchingCubesFactory.h"
#include "Camera.h"
#include <glm\geometric.hpp>
#include <glm\common.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

using namespace std;

MarchingCubesShaded::MarchingCubesShaded( const char* filePath )
{
	MappedData paramFile( filePath );

	vPosition = paramFile.getData("base","position").getVec3f();	
	vSpan = paramFile.getData("base","span").getVec3f();	
		
	dataWidth = paramFile.getData("base","dataWidth").getInt();
	dataHeight = paramFile.getData("base","dataHeight").getInt();
	dataDepth = paramFile.getData("base","dataDepth").getInt();
	dataWidth = dataWidth<DATA_MIN ? DATA_MIN : dataWidth;
	dataHeight = dataHeight<DATA_MIN ? DATA_MIN : dataHeight;
	dataDepth = dataDepth<DATA_MIN ? DATA_MIN : dataDepth;
	
	dataMax = paramFile.getData("base","maxValue").getInt();
	treshold = paramFile.getData("base","treshold").getFloat( 0.5f );

	string vertex = paramFile.getData("shaders", "v").getStringData();
	string geometry = paramFile.getData("shaders", "g").getStringData();
	string fragment = paramFile.getData("shaders", "f").getStringData();

	mcShader = ShaderProgram::CreateShader( vertex, geometry, fragment );

	GLuint programHandle = mcShader->getProgramID();

	initDataField( );	
		
	initGridBuffer( );
		
	mcShader->turnOn();
		mcShader->setUniformI( "Data", 0 );
		mcShader->setUniformI( "TriTable", 1 );
		
		mcShader->setUniformV3( "VerticeOffsets[0]", 0.0f,			0.0f,		0.0f		); 
		mcShader->setUniformV3( "VerticeOffsets[1]", gridStep.x,	0.0f,		0.0f		); 
		mcShader->setUniformV3( "VerticeOffsets[2]", gridStep.x,	gridStep.y, 0.0f		); 
		mcShader->setUniformV3( "VerticeOffsets[3]", 0.0f,			gridStep.y, 0.0f		); 
		mcShader->setUniformV3( "VerticeOffsets[4]", 0.0f,			0.0f,		gridStep.z	); 
		mcShader->setUniformV3( "VerticeOffsets[5]", gridStep.x,	0.0f,		gridStep.z	); 
		mcShader->setUniformV3( "VerticeOffsets[6]", gridStep.x,	gridStep.y, gridStep.z	); 
		mcShader->setUniformV3( "VerticeOffsets[7]", 0.0f,			gridStep.y, gridStep.z	);

		mcShader->setUniformV3("Color", 0.0f, 0.3f, 1.0f);
		mcShader->setUniformV3("Diffuse", 0.4f, 0.9f, 0.0f);
		mcShader->setUniformV3("Specular", 1.0f, 1.0f, 1.0f);
		mcShader->setUniformV3("Ambient", 0.3f, 0.8f, 0.6f);
		mcShader->setUniformV3("LightPosition", 20.f, -15.0f, 5.0f);

		mcShader->setUniformV3( "DataStep", 1.0f/dataWidth, 1.0f/dataHeight, 1.0f/dataDepth );
	mcShader->turnOff();

	clear();
}

MarchingCubesShaded::~MarchingCubesShaded( )
{
	delete [] dataField;
	// delete buffer and vao
	glDeleteBuffers(1,&gridHandle);
	glDeleteVertexArrays(1,&gridVao);
}

// Create a 3D grid of points in the span od 0,0,0 - 1,1,1
// This will be bound to the VAO and sent to the shader as vertices
// Actual data that generates the marching cube mesh is sent as a 3D texture
void MarchingCubesShaded::initGridBuffer( )
{
	gridStep = vec3f(1,1,1) / (vec3f(dataWidth, dataHeight, dataDepth)-vec3f(1,1,1));
	gridElementCount = (dataWidth + 1)*(dataHeight + 1)*(dataDepth + 1);

	int gridTotalSize =  gridElementCount*3;
	float* grid = new float[gridTotalSize];
	int index = 0;
	for( float x = -gridStep.x; x <= 1.0f; x+=gridStep.x )
	{
		for( float y = -gridStep.y; y <= 1.0f; y+=gridStep.y )
		{
			for( float z = -gridStep.z; z <= 1.0f; z+=gridStep.z )
			{
				grid[ index   ] = x;
				grid[ index+1 ] = y;
				grid[ index+2 ] = z;
				index += 3;				
			}
		}
	}
	
	glGenBuffers(1, &gridHandle);
	glBindBuffer( GL_ARRAY_BUFFER, gridHandle );
	glBufferData( GL_ARRAY_BUFFER, gridTotalSize * sizeof(float), grid, GL_STATIC_DRAW );
	delete[] grid;
	
	glGenVertexArrays( 1, &gridVao );
	glBindVertexArray( gridVao );
	glEnableVertexAttribArray(0);
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL );

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(0);
}

void MarchingCubesShaded::initDataField( )
{
	// MC DATA
	glGenTextures(1, &dataTexID);

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_3D);
	glBindTexture(GL_TEXTURE_3D, dataTexID);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glDisable(GL_TEXTURE_3D);

	dataSize = dataWidth * dataHeight * dataDepth;
	deltaSpan = vSpan / vec3f( dataWidth, dataHeight, dataDepth );
	dataField = new float[ dataSize ]();
	dataChanged = true;
	// data is updated when changed
//	glTexImage3D( GL_TEXTURE_3D, 0, GL_ALPHA32F_ARB, dataWidth, dataHeight, dataDepth, 0, GL_ALPHA, GL_FLOAT, dataField);
	
}

void MarchingCubesShaded::setUnchecked( int x, int y, int z, float value )
{
	float &data = dataField[ x*dataHeight*dataDepth + y*dataDepth + z ];
	data = contain(value+data, 0.0f, dataMax);
	dataChanged = true;
}

void MarchingCubesShaded::set( int x, int y, int z, float value )
{
	if( x<1 || y<1  || z<1 || x>=dataWidth-1 || y>=dataHeight-1 || z>=dataDepth-1 ) return;	
	float &data = dataField[ x*dataHeight*dataDepth + y*dataDepth + z ];
	data = contain(value+data, 0.0f, dataMax);
	dataChanged = true;
}

void MarchingCubesShaded::set( int position, float value )
{
	if( position<0 || position>dataSize ) return;	
	dataField[ position ] = contain(value, 0.0f, dataMax);
	dataChanged = true;
}

void MarchingCubesShaded::clear()
{
	memset( dataField, 0, dataSize*sizeof(float) );
	dataChanged = true;
}

vec3f MarchingCubesShaded::getScale()
{
	return vSpan;
}

vec3f MarchingCubesShaded::getPosition()
{
	return vPosition;
}

float MarchingCubesShaded::getTreshold()
{
	return this->treshold;
}

void MarchingCubesShaded::setTreshold( float t )
{
	this->treshold = t;
}

void MarchingCubesShaded::putSphere( float x, float y, float z, float r )
{
	float value;
	vec3f start = vec3f( x-r, y-r, z-r );
	//start = glm::clamp( start, vPosition, vPosition+vSpan );
	start -= vPosition;
	start /= deltaSpan;

	vec3f end = vec3f( x+r, y+r, z+r );
	//glm::clamp( end, vPosition, vPosition+vSpan );
	end -= vPosition;
	end /= deltaSpan;

	vec3f center = (vec3f( x,y,z ) - vPosition) / deltaSpan;
	r /= glm::length( deltaSpan );

	for(int i=start.x; i<end.x; i++)
	{
		for(int j=start.y; j<end.y; j++)
		{
			for(int k=start.z; k<end.z; k++)
			{
				value = r-glm::length(center-vec3f(i,j,k))+1;
				if (value > 0)
				{
					set(i, j, k, value);
				}
			}
		}
	}
}

void MarchingCubesShaded::draw(const Camera& camera)
{
	glm::mat4 mvp = camera.getViewProjection() * transform.getTransformMatrix();
	glm::vec3 eye = camera.getPosition();
		
	glDisable( GL_CULL_FACE );
	mcShader->turnOn();
		mcShader->setUniformF( "Treshold", this->treshold );		
		mcShader->setUniformV3( "Eye", eye.x, eye.y, eye.z );
		mcShader->setUniformM4( "MVP", glm::value_ptr(mvp) );
	
		MarchingCubesFactory::setTexture( GL_TEXTURE1 );
	
		glEnable( GL_TEXTURE_3D );
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_3D, dataTexID );
		if (dataChanged)
		{
			glTexImage3D(GL_TEXTURE_3D, 0, GL_ALPHA32F_ARB, dataWidth, dataHeight, dataDepth, 0, GL_ALPHA, GL_FLOAT, dataField);
			dataChanged = false;
		}

		// !
		GLboolean blendEnabled = glIsEnabled( GL_BLEND );
		glEnable( GL_BLEND );
		glBlendFunc(GL_ONE, GL_ONE);
		glDepthMask(GL_FALSE);
			glBindVertexArray(gridVao);
			glDrawArrays(GL_POINTS, 0, gridElementCount );
			glBindVertexArray(0);
		glDepthMask(GL_TRUE);
		if( !blendEnabled ) glDisable( GL_BLEND );	

		glDisable( GL_TEXTURE_3D);
	mcShader->turnOff();
	glEnable( GL_CULL_FACE );
}