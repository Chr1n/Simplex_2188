///Custom class for keeping track of a single cell in the grid
#include "Vertex.h"

/// Constructor to initialize values
Vertex::Vertex()
{
	hCost = INT_MAX;
	gCost = INT_MAX;
	open = false;
	closed = false;
	visited = false;
	prevV = nullptr;
	this->xPos = 0;
	this->yPos = 0;
}

/// Destructor
Vertex::~Vertex()
{
}

/// Parameterized Constructor that takes in x and y position
Vertex::Vertex(int _xPos, int _yPos)
{
	this->xPos = _xPos;
	this->yPos = _yPos;
	gCost = INT_MAX;
	hCost = INT_MAX;
	open = false;
	closed = false;
	visited = false;
	prevV = nullptr;

}

/// Copy Constructor to copy in the values for the new vertex
Vertex::Vertex(const Vertex& vertex)
{
	this->xPos = vertex.xPos;
	this->yPos = vertex.yPos;
	this->gCost = vertex.gCost;
	this->hCost = vertex.hCost;
	this->fCost = vertex.fCost;
	this->open = vertex.open;
	this->closed = vertex.closed;
	this->visited = vertex.visited;
	this->prevV = vertex.prevV;
}

/// Equals operator which is pretty much the same as the copy constructor but returns the vertex
Vertex& Vertex::operator=(const Vertex& vertex)
{
	this->xPos = vertex.xPos;
	this->yPos = vertex.yPos;
	this->gCost = vertex.gCost;
	this->hCost = vertex.hCost;
	this->fCost = vertex.fCost;
	this->open = vertex.open;
	this->closed = vertex.closed;
	this->visited = vertex.visited;
	this->prevV = vertex.prevV;

	return *this;
}

/// Equal operator will determine if vertices are equal or not
/// Returns true if they are equal
bool Vertex::operator==(const Vertex& vertex) const
{
	if (this->xPos == vertex.xPos && this->yPos == vertex.yPos)
	{
		return true;
	}

	return false;
}


