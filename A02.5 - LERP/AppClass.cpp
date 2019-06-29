#include "AppClass.h"
#include <iostream>
using namespace std;

void Application::InitVariables(void)
{
	//Change this to your name and email
	m_sProgrammer = "Christopher Lawrence - cml3051@rit.edu";
	
	//Set the position and target of the camera
	//(I'm at [0,0,10], looking at [0,0,0] and up is the positive Y axis)
	m_pCameraMngr->SetPositionTargetAndUpward(AXIS_Z * 20.0f, ZERO_V3, AXIS_Y);

	//if the light position is zero move it
	if (m_pLightMngr->GetPosition(1) == ZERO_V3)
		m_pLightMngr->SetPosition(vector3(0.0f, 0.0f, 3.0f));

	//if the background is cornflowerblue change it to black (its easier to see)
	if (vector3(m_v4ClearColor) == C_BLUE_CORNFLOWER)
	{
		m_v4ClearColor = vector4(ZERO_V3, 1.0f);
	}
	
	//if there are no segments create 7
	if (m_uOrbits < 1)
	{
		m_uOrbits = 7;
	}
	
	float fSize = 1.0f; //initial size of orbits

	//creating a color using the spectrum 
	uint uColor = 650; //650 is Red
	//prevent division by 0
	float decrements = 250.0f / (m_uOrbits > 1? static_cast<float>(m_uOrbits - 1) : 1.0f); //decrement until you get to 400 (which is violet)
	/*
		This part will create the orbits, it start at 3 because that is the minimum subdivisions a torus can have
	*/
	uint uSides = 3; //start with the minimal 3 sides
	for (uint i = uSides; i < m_uOrbits + uSides; i++)
	{
		vector3 v3Color = WaveLengthToRGB(uColor); //calculate color based on wavelength
		m_shapeList.push_back(m_pMeshMngr->GenerateTorus(fSize, fSize - 0.1f, 3, i, v3Color)); //generate a custom torus and add it to the meshmanager
		fSize += 0.5f; //increment the size for the next orbit
		uColor -= static_cast<uint>(decrements); //decrease the wavelength
	}
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the arcball active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	matrix4 m4View = m_pCameraMngr->GetViewMatrix(); //view Matrix
	matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix(); //Projection Matrix
	matrix4 m4Offset = IDENTITY_M4; //offset of the orbits, starts as the global coordinate system
	/*
		The following offset will orient the orbits as in the demo, start without it to make your life easier.
	*/
	m4Offset = glm::rotate(IDENTITY_M4, 1.5708f, AXIS_Z); // Used to rotate the entire thing, so the shapes point upwards

	// Calculate the list of Stops and create timers needed
	static vector<vector<vector3>> v3Stops;      // List of lists of stops
	static vector<float> fTimers;                // List of timers for each shape
	static vector<uint> pathRoutes;              // List for the routes for each shape
	static vector<uint> uClocks;                 // List of clocks
	static bool bInitOnce = false;               // Bool to be initialized once

	// Entire block of code of calculating the stops will occur when the bool is false
	if (!bInitOnce)
	{
		// Loop through the orbits to calculate the appropriate stops
		for (uint i = 0; i < m_uOrbits; i++)
		{
			vector<vector3> stops; // list of stops

			// Calculate stops in terms of the number of sides
			float segAngle = 2 * PI / (float)(i + 3);   // Angle of each segment of the current shape
			float radius = 1.0f + i * 0.5f - 0.05f;

			// Push back each appropriate stop onto the list using a loop
			for (uint j = 0; j < i + 3; j++)
			{
				stops.push_back(vector3(cosf(segAngle * j) * radius, sinf(segAngle * j) * radius, 0));
			}

			// Add them to the main list of stops
			v3Stops.push_back(stops);

			// Initialize timers and routes
			fTimers.push_back(0);                      // Store the new timer in the list of timers for each shape
			uClocks.push_back(m_pSystem->GenClock());  // Generate a new clock and push it into the list of clocks
			pathRoutes.push_back(0);
		}

		// Set true when all calculations are done
		bInitOnce = true;
	}

	// Draw shapes
	// Loop through all the orbits to search for the immediate stop
	for (uint i = 0; i < m_uOrbits; ++i)
	{
		m_pMeshMngr->AddMeshToRenderList(m_shapeList[i], glm::rotate(m4Offset, 1.5708f, AXIS_X));

		// Get a timer from the list of timers
		fTimers[i] += m_pSystem->GetDeltaTime(uClocks[i]);              // Get the delta time for that timer
		vector3 v3Start;												// Start point
		vector3 v3End;												    // End point
		v3Start = v3Stops[i][pathRoutes[i]];							// Start at the current route
		v3End = v3Stops[i][(pathRoutes[i] + 1) % v3Stops[i].size()];    // End at route + 1, and if its over the limit, it will restart from 0

		// Get the percentage / Time between stops
		float fTimeBetwStops = 0.8f;

		// Map the value to be between 0.0 and 1.0
		float fPercent = MapValue(fTimers[i], 0.0f, fTimeBetwStops, 0.0f, 1.0f);

		//calculate the current position by using linear interpolation
		vector3 v3CurrentPos = glm::lerp(v3Start, v3End, fPercent);
		matrix4 m4Model = glm::translate(m4Offset, v3CurrentPos);

		// If we are done with this route,
		// Go the next route, restart the clock, and make sure we are within the boundaries
		if (fPercent >= 1.0f)
		{
			pathRoutes[i]++;									// To next route
			fTimers[i] = m_pSystem->GetDeltaTime(uClocks[i]);   // Restart the clock
			pathRoutes[i] %= v3Stops[i].size();					// Make sure that we are within boundaries
		}

		//draw spheres
		m_pMeshMngr->AddSphereToRenderList(m4Model * glm::scale(vector3(0.2)), C_WHITE);
	}

	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

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