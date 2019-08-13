/// Vertex.h is our custom class for keeping track of a single cell in the grid
#pragma once
#include <memory>
using namespace std;

class Vertex
{
public:
	// Constructors
	Vertex(); // Default
	~Vertex(); // Destructor
	Vertex(int _xPos, int _yPos); // Parameterized
	Vertex(const Vertex& vertex); // Copy
	Vertex& operator=(const Vertex& vertex); // Copy Assignment operator
	bool operator==(const Vertex& vertex) const; // equal operator

	shared_ptr<Vertex> prevV; // previous vertex pointer
	shared_ptr<Vertex> nextV; // next vertex pointer
	int xPos; //xPos of vertex
	int yPos; //yPos of vertex
	float hCost; //heuristic cost
	float gCost; //gCost
	float fCost; //fCost
	bool open = false;
	bool closed = false;
	bool visited;
};
