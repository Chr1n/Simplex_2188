#include "AppClass.h"
#include <math.h>
#include <random>
using namespace Simplex;
void Application::InitVariables(void)
{
	m_sProgrammer = "Christopher Lawrence - cml3051@g.rit.edu";

	//Set the position and target of the camera
	m_pCameraMngr->SetPositionTargetAndUpward(
		vector3(0.0f, 9.0f, 20.0f), //Position
		vector3(0.0f, 0.0f, 0.0f),	//Target
		AXIS_Y);					//Up

	m_pLightMngr->SetPosition(vector3(0.0f, 3.0f, 13.0f), 1); //set the position of first light (0 is reserved for ambient light)
	m_pEntityMngr = MyEntityManager::GetInstance(); //Initialize the entity manager

	// For loop to create the maze with minecraft cube blocks
	for (int i = 0; i < 20; i++)
	{
		for (int j = 0; j < 20; j++)
		{
			int x = i - 10;
			int z = j - 10;
			if (minecraftMaze[i][j] > 0)
			{
				m_pEntityMngr->AddEntity("Minecraft\\Cube.obj", "Cube_" + to_string(j * 20 + i));
				vector3 v3pos = vector3(x, 0, z);
				matrix4 m4Pos = glm::translate(v3pos);
				m_pEntityMngr->SetModelMatrix(m4Pos);
			}
		}
	}

	// Creating the start and end positions
	startPos = vector3(4, 1, 18);
	endPos = vector3(18, 1, 2);

	// Create Steve, the victim who walks along the cubes
	m_pEntityMngr->AddEntity("Minecraft\\Steve.obj", "Steve");
	targetPos = vector3(endPos.x - 10 + 0.5, 1, endPos.z - 10 + 0.5);
	m_pEntityMngr->SetModelMatrix(glm::translate(targetPos));

	// Initialize the Zombie, zombie will be the path follower
	// Put the zombie at the start position
	m_pEntityMngr->AddEntity("Minecraft\\Zombie.obj", "Zombie");
	vector3 zombiePos = vector3(startPos.x - 10 + 0.5, 1, startPos.z - 10 + 0.5);

	// Create the graph class by entering the maze, start and end vertex parameters
	mazeGraph.SetStart(startPos.x, startPos.z);
	mazeGraph.SetEnd(endPos.x, endPos.z);

	// Width and height of the maze, according to the matrix provided
	mazeGraph.mazeWidth = 20;
	mazeGraph.mazeHeight = 20;

	// Temporary pointer to hold data
	int* tempMaze[20];

	// Set the original maze to the temporary maze pointer
	for (int i = 0; i < 20; i++)
	{
		tempMaze[i] = minecraftMaze[i];
	}

	// Set the maze using the temporary maze, and perform A Star algorithm
	mazeGraph.SetMaze(tempMaze);
	mazeGraph.FindAStarPath(aStarPath);

	// Set path searched bool value to true if the path is found
	bPathSearched = false;
	if (aStarPath.size() > 0)
	{
		bPathSearched = true;
	}

	// Create the clock
	uClock = m_pSystem->GenClock();

	// Create the octree to reduce lag because that's annoying in minecraft of course
	m_pRoot = new MyOctant(m_uOctantLevels, 3);

}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the arcball active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();

	//Update Entity Manager
	m_pEntityMngr->Update();

	// Consider steve and zombie so that it doesn't lag while doing octree and A Star
	static uint clock = m_pSystem->GenClock();
	static bool bStart = false;
	if (m_pSystem->IsTimerDone(clock) || !bStart)
	{
		bStart = true;
		m_pSystem->StartTimerOnClock(0.5, clock);
		SafeDelete(m_pRoot);
		m_pRoot = new MyOctant(m_uOctantLevels, 4);
	}

	// Recalculate the A Star path if the user has moved steve
	if (bMoved == true)
	{
		endPos = targetPos;
		CalculatePath();
		bMoved = false;
	}

	// Add objects to render list
	m_pEntityMngr->AddEntityToRenderList(-1, true);
}

void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	// Display the Octree if it is true
	if (m_bOctVisRep)
	{
		m_pRoot->Display(C_YELLOW);
	}

	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();

	// This block of code is for drawing the green tiles and red tiles
	// Red tiles indicate the path, and the green tiles mean regular ground
	// Will run if there is a path found
	if (bPathSearched)
	{
		for (int i = 0; i < 20; i++)
		{
			for (int j = 0; j < 20; j++)
			{
				if (minecraftMaze[i][j] > 0)
				{
					// Variables and data needed for laying out the materials
					int x = i - 10;
					int z = j - 10;
					Vertex vertex(i, j);
					vector3 v3CubePos = vector3(x + 0.5, 1.07, z + 0.5);
					matrix4 m4Model = glm::translate(v3CubePos)*glm::scale(vector3(0.9, 0.02, 0.9));

					// Green means regular ground, red means the A Star path that the zombie is following
					if (find(aStarPath.begin(), aStarPath.end(), vertex) == aStarPath.end())
					{
						m_pMeshMngr->AddCubeToRenderList(m4Model, C_GREEN);
					}
					else
					{
						m_pMeshMngr->AddCubeToRenderList(m4Model, C_RED);
					}
				}

			}
		}
	}

	// All this is needed for movement through the use of lerping
	// Current position
	int currPos = aStarPath.size() - 1;
	vector3 v3CurrentPos = vector3(0.0f, 0.0f, 0.0f);
	if (currPos - 1 >= 0)
	{
		// Percentage from 0 to 1
		fTime += m_pSystem->GetDeltaTime(uClock); // delta time
		float fPercentage = MapValue(fTime, 0.0f, 0.6f, 0.0f, 1.f);

		// Start and end position
		vector3 v3startPos = vector3(aStarPath[currPos].xPos - 10 + 0.5, 1, aStarPath[currPos].yPos - 10 + 0.5);
		vector3 v3endPos = vector3(aStarPath[currPos - 1].xPos - 10 + 0.5, 1, aStarPath[currPos - 1].yPos - 10 + 0.5);

		// Reset the clock for every second that passes by
		if (fPercentage >= 1.0f)
		{
			fPercentage = 1;
			currentPos--;
			fTime = m_pSystem->GetDeltaTime(uClock);
			aStarPath.pop_back();
		}

		// LERP between the start and end nodes
		v3CurrentPos = glm::lerp(v3startPos, v3endPos, fPercentage);
		currentPos = v3CurrentPos;
	}

	// Model Matrix for Steve and Zombie
	m_pEntityMngr->SetModelMatrix(glm::translate(currentPos), "Zombie");
	m_pEntityMngr->SetModelMatrix(glm::translate(targetPos), "Steve");

	// Render calls
	int calls = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();

	//draw gui
	DrawGUI();

	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}

void Application::Release(void)
{
	//release GUI
	ShutdownGUI();
}

bool Application::CalculatePath()
{
	// Create clock for timer
	uClock = m_pSystem->GenClock(); 
	fTime = 0;

	// Reset and make a new graph
	mazeGraph.ResetGraph();
	mazeGraph = Graph();

	//setting the width and height
	mazeGraph.mazeWidth = 20;
	mazeGraph.mazeHeight = 20;

	// Set the start and end vertices
	mazeGraph.SetStart(ApproxHalf(currentPos.x + 9.5), ApproxHalf(currentPos.z + 9.5));
	mazeGraph.SetEnd(ApproxHalf(targetPos.x + 9.5), ApproxHalf(targetPos.z + 9.5));

	// Making a temporary pointer to hold the data
	int* tempMaze[20];

	// Assigning original maze data values into the temporary pointer
	for (int i = 0; i < 20; i++)
	{
		tempMaze[i] = minecraftMaze[i];
	}

	// Setting the maze using the temporary maze
	mazeGraph.SetMaze(tempMaze);

	// Clearing the path, and perform the A Star search algorithm again
	aStarPath.clear();
	mazeGraph.FindAStarPath(aStarPath);

	return true;
}

/// Need this method because its annoying to keep typing out the same formula
float Simplex::Application::ApproxHalf(float num)
{
	return floor((num * 2) + 0.5f) / 2;
}
