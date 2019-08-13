/// Graph.h is our custom class for keeping track of the entire grid
#pragma once
#include "Vertex.h"
#include <vector>
#include <stack>
#include <queue>
#include <map>
using namespace std;
class Graph
{
public:

	Graph(); // Default constructor
	~Graph(); // Destructor

	// A bunch of data regarding the maze world
	int mazeWidth;
	int mazeHeight;
	Vertex startV;
	Vertex endV;
	Vertex currV;

	//Methods to set up the start vertex and end vertex
	void SetStart(const int& _xPos, const int& _yPos);
	void SetEnd(const int& _xPos, const int& _yPos);

	// Method to set up the open and closed lists for the vertices in the maze world
	void SetOpenList();
	void SetClosedList();
	
	// Method will be used for cleanup
	void ResetGraph();

	// Method for setting up the maze world
	void SetMaze(int** mazeData);

	// The method that holds and handles the A* algorithm
	void FindAStarPath(vector<Vertex>& path);

private:

	// Method to find neighboring vertices
	void SearchNeighbors(vector<Vertex>& _neighbors);

	// The stack that will be used to hold our determined path
	stack<Vertex> path;
	vector<Vertex> neighbors;
	Vertex* searched;

	// Open and closed lists
	map<Vertex, bool> closedList; //closed
	bool** bClosed; //inClosed
	float** openList; //inOpen
	float** hCost; //heuristicSSSS
	int** adjacencyMatrix; // adj matrix

	// Struct needed to compare two vertices and evaluate them depending on their fCost
	struct PriorityV
	{
		bool operator()(const Vertex& v1, const Vertex& v2) const
		{
			return v1.fCost > v2.fCost;
		}
	};

	// Open and closed list
	priority_queue<Vertex, vector<Vertex>, PriorityV> openclose;
};