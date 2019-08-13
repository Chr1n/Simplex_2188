/// Custom class for keeping track of the entire grid
#include "Graph.h"
#include "Vertex.h"

// Set the pointers to null at the start
Graph::Graph()
{
	openList = nullptr;
	bClosed = nullptr;
	hCost = nullptr;
	searched = nullptr;
}

// Destructor to clean up the open, closed, hCost, and searched vertex pointers
Graph::~Graph()
{
	if (openList != nullptr)
	{
		for (int i = 0; i < mazeWidth; i++)
		{
			delete[] openList[i];
		}

		delete[] openList;
		openList = nullptr;
	}

	if (bClosed != nullptr)
	{
		for (int i = 0; i < mazeWidth; i++)
		{
			delete[] bClosed[i];
		}

		delete[] bClosed;
		bClosed = nullptr;
	}

	if (hCost != nullptr)
	{
		for (int i = 0; i < mazeWidth; i++)
		{
			delete[] hCost[i];
		}

		delete[] hCost;
		hCost = nullptr;
	}

	if (searched != nullptr)
	{
		delete searched;
		searched = nullptr;
	}
}

/// Method for setting up the start vertex position
void Graph::SetStart(const int & _xPos, const int & _yPos)
{
	startV.xPos = _xPos;
	startV.yPos = _yPos;
}

/// Method for setting up the end vertex position
void Graph::SetEnd(const int & _xPos, const int & _yPos)
{
	endV.xPos = _xPos;
	endV.yPos = _yPos;
}

/// Method for setting up the open list
void Graph::SetOpenList()
{
	// First clean up the open list
	if (openList != nullptr)
	{
		for (int i = 0; i < mazeWidth; i++)
		{
			delete[] openList[i];
		}

		delete[] openList;
		openList = nullptr;
	}

	// Create the new open list
	openList = new float*[mazeWidth];

	// Arrange heights
	for (int i = 0; i < mazeWidth; i++)
	{
		openList[i] = new float[mazeHeight];
	}

	// Rearrange the open list
	for (int i = 0; i < mazeWidth; i++)
	{
		for (int j = 0; j < mazeHeight; j++)
		{
			if (adjacencyMatrix[i][j] > 0)
			{
				openList[i][j] = INT_MAX;
			}
		}
	}
}

/// Method for setting up the closed list
void Graph::SetClosedList()
{
	// First clean up the closed list
	if (bClosed != nullptr)
	{
		for (int i = 0; i < mazeWidth; i++)
		{
			delete[] bClosed[i];
		}

		delete[] bClosed;
		bClosed = nullptr;
	}

	// Create the new closed list
	bClosed = new bool*[mazeWidth];

	// Arrange heights
	for (int i = 0; i < mazeWidth; i++)
	{
		bClosed[i] = new bool[mazeHeight];
	}

	// Rearrange the closed list
	for (int i = 0; i < mazeWidth; i++)
	{
		for (int j = 0; j < mazeHeight; j++)
		{
			if (adjacencyMatrix[i][j] > 0)
			{
				bClosed[i][j] = false;
			}
		}
	}
}

/// Method for cleaning up the maze data
/// Pretty much the destructor part all over again
void Graph::ResetGraph()
{
	if (openList != nullptr)
	{
		for (int i = 0; i < mazeWidth; i++)
		{
			delete[] openList[i];
		}

		delete[] openList;
		openList = nullptr;
	}

	if (bClosed != nullptr)
	{
		for (int i = 0; i < mazeWidth; i++)
		{
			delete[] bClosed[i];
		}

		delete[] bClosed;
		bClosed = nullptr;
	}

	if (hCost != nullptr)
	{
		for (int i = 0; i < mazeWidth; i++)
		{
			delete[] hCost[i];
		}

		delete[] hCost;
		hCost = nullptr;
	}

	if (searched != nullptr)
	{
		delete searched;
		searched = nullptr;
	}
}

/// Method to set up the maze
void Graph::SetMaze(int ** mazeData)
{
	// Create the maze
	adjacencyMatrix = mazeData;

	// H Cost calculations
	hCost = new float*[mazeWidth];
	for (int i = 0; i < mazeWidth; i++)
	{
		hCost[i] = new float[mazeHeight];
	}

	// Nested for loop across the whole maze needed to find the hCost to each vertex
	for (int i = 0; i < mazeWidth; i++)
	{
		for (int j = 0; j < mazeHeight; j++)
		{
			if (adjacencyMatrix[i][j] > 0)
			{
				float hCostSq = pow((i - endV.xPos), 2) + pow((j - endV.yPos), 2);
				float heuristic = sqrt(hCostSq);
				hCost[i][j] = heuristic;
			}
		}
	}

	// Proceed to set up the open and closed lists
	SetOpenList();
	SetClosedList();
}

/// The A Star Algorithm
void Graph::FindAStarPath(vector<Vertex>& path)
{
	// Current and Start Vertex
	currV = startV;
	currV.gCost = 0;
	currV.hCost = hCost[currV.xPos][currV.yPos];
	currV.fCost = currV.gCost + currV.hCost;
	openclose.push(currV);
	openList[currV.xPos][currV.yPos] = 0;

	// While loop to continue searching until we find the path, or determine if the path exists
	while (!openclose.empty())
	{
		// Manage the current vertex from the queue and manage neighbors too
		currV = openclose.top();
		openclose.pop();
		bClosed[currV.xPos][currV.yPos] = true;
		neighbors.clear();

		// Determine if the x and y location is within the area of the maze
		// Then we check if it's a wall or not
		SearchNeighbors(neighbors);

		// Loop for calculating costs and handling checking the vertices in open list and closed list
		int nSize = neighbors.size();
		for (int i = 0; i < nSize; i++)
		{
			// Calulating the cost depending on the neighbor nodes
			float cost = currV.gCost + adjacencyMatrix[neighbors[i].xPos][neighbors[i].yPos];

			// See if the vertex is in the closed list or not
			if (bClosed[neighbors[i].xPos][neighbors[i].yPos] == true)
			{
				continue;
			}

			// Need to check if the neighbor vertex is in the open list
			if (cost < openList[neighbors[i].xPos][neighbors[i].yPos] || openList[neighbors[i].xPos][neighbors[i].yPos] == INT_MAX)
			{
				// gCost, hCost, and fCost
				neighbors[i].gCost = cost;
				openList[neighbors[i].xPos][neighbors[i].yPos] = cost;
				neighbors[i].hCost = hCost[neighbors[i].xPos][neighbors[i].yPos];
				neighbors[i].fCost = neighbors[i].hCost*1.2 + neighbors[i].gCost;

				// Assign the parent vertex to the current vertex
				neighbors[i].prevV = make_shared<Vertex>(currV);
				openclose.push(neighbors[i]);

				// Create the path again if the neighbour is the goal
				if (neighbors[i] == endV)
				{
					searched = &neighbors[i];
					while (searched != nullptr)
					{
						path.push_back(*searched);
						searched = searched->prevV.get();

					};
					return;
				}
			}
		}
	}

	// Back to start if no path is found
	if (path.size() == 0)
	{
		path.push_back(Vertex(startV.xPos, startV.yPos));
	}
}

/// Method for searching neighbor vertices
void Graph::SearchNeighbors(vector<Vertex>& _neighbors)
{
	Vertex topN(currV.xPos, currV.yPos + 1);
	Vertex bottomN(currV.xPos, currV.yPos - 1);
	Vertex rightN(currV.xPos + 1, currV.yPos);
	Vertex leftN(currV.xPos - 1, currV.yPos);

	// Top neighbor
	if (topN.xPos >= 0 && topN.yPos >= 0 && topN.xPos < mazeWidth&&topN.yPos < mazeHeight
		&&adjacencyMatrix[topN.xPos][topN.yPos] > 0)
	{
		neighbors.push_back(topN);
	}

	// Bottom neighbor
	if (bottomN.xPos >= 0 && bottomN.yPos >= 0 && bottomN.xPos < mazeWidth&&bottomN.yPos < mazeHeight
		&&adjacencyMatrix[bottomN.xPos][bottomN.yPos] > 0)
	{
		neighbors.push_back(bottomN);
	}

	// Right neighbor
	if (rightN.xPos >= 0 && rightN.yPos >= 0 && rightN.xPos < mazeWidth&&rightN.yPos < mazeHeight
		&&adjacencyMatrix[rightN.xPos][rightN.yPos] > 0)
	{
		neighbors.push_back(rightN);
	}

	// Left neighbor
	if (leftN.xPos >= 0 && leftN.yPos >= 0 && leftN.xPos < mazeWidth&&leftN.yPos < mazeHeight
		&&adjacencyMatrix[leftN.xPos][leftN.yPos] > 0)
	{
		neighbors.push_back(leftN);
	}
}

