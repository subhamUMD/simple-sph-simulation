#pragma once
#include "Scene.h"
#include "IntervalAverageTimer.h"
#include "Timer.h"
#include <gl\glew.h>

class SPHSystem3d;
class PointDataVisualiser;
class MarchingCubesShaded;
class LineGrid;
class Interactor;

class SPHScene :
	public Scene
{
public:
	SPHScene(void);
	virtual ~SPHScene(void);
	
	void draw(const Camera& camera);
	void drawSFML(sf::RenderWindow& window);
	void update(float dt);
	virtual void eventKeyboardUp(sf::Keyboard::Key keyPressed);


private:
	SPHSystem3d* sph3;
	PointDataVisualiser* pointVisualizer;
	MarchingCubesShaded *marchingCubes;
	LineGrid* grid;
	LineGrid* coords;

	//Costumize
	bool interactored;
	bool interactorFlag;
	Interactor* interactor;

	IntervalAverageTimer sphTimer;
	IntervalAverageTimer marchingTimer;
	FPSTimer fpsTimer;
	Timer clock;
	bool drawWithMC;
	float treshold;

	bool paused;

	// GUI
	bool fontLoaded;
	sf::Font anonPro;
	sf::Text status;
};

