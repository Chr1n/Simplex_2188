/*----------------------------------------------
Programmer: Christopher Lawrence (cml3051@rit.edu)
Date: 07/2019
----------------------------------------------*/
#include "MyOctant.h"
using namespace Simplex;

// Initializing static values
uint MyOctant::m_uOctantCount = 0;
uint MyOctant::m_uMaxLevel = 1; //0
uint MyOctant::m_uIdealEntityCount = 1; //0

// constructors
Simplex::MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
	Init();

	// Set the octant count, max level, and the ideal number of entities for each octant
	m_uOctantCount = 0;
	m_uMaxLevel = a_nMaxLevel;
	m_uIdealEntityCount = a_nIdealEntityCount;
	m_uID = m_uOctantCount;

	// Set the octant as the root
	m_pRoot = this;
	m_lChild.clear();

	std::vector<vector3> lMinMax;

	// Retrieve the number of entities on the scene, and loop through the number of entities in the scene
	uint iObjects = m_pEntityMngr->GetEntityCount();
	for (size_t i = 0; i < iObjects; i++)
	{
		// Retrieve rigid body of each entity
		MyEntity* pEntity = m_pEntityMngr->GetEntity(i);
		MyRigidBody* pRB = pEntity->GetRigidBody();

		// Store the minimum and maximum in the list
		lMinMax.push_back(pRB->GetMinGlobal());
		lMinMax.push_back(pRB->GetMaxGlobal());
	}

	// Create a rigidBody pointer with the new set of vertices from the loop
	MyRigidBody* pRigidBody = new MyRigidBody(lMinMax);

	// Retrieve the halfwidth of the rigid body with the vertices
	vector3 halfWidth = pRigidBody->GetHalfWidth();

	// Get the max value of the bounding object
	float fMax = halfWidth.x;
	for (size_t i = 0; i < 3; i++)
	{
		if (fMax < halfWidth[i])
		{
			fMax = halfWidth[i];
		}
	}

	// Retrieve center of the bounding box
	vector3 center = pRigidBody->GetCenterLocal();
	lMinMax.clear();
	SafeDelete(pRigidBody);

	// Calculating the min and max vertices of the box
	m_fSize = fMax * 2.0f;
	m_v3Center = center;
	m_v3Min = m_v3Center - (vector3(fMax));
	m_v3Max = m_v3Center + (vector3(fMax));

	m_uOctantCount++;

	// Construct the oct tree depending on the number of max level passed in
	ConstructTree(m_uMaxLevel);
}

/// This is for calculating the center, size, and minimum and maximum vectors for the child octant
Simplex::MyOctant::MyOctant(vector3 a_v3Center, float a_fSize)
{
	Init();
	m_v3Center = a_v3Center;
	m_fSize = a_fSize;

	m_v3Min = m_v3Center - (vector3(m_fSize) / 2.0f);
	m_v3Max = m_v3Center + (vector3(m_fSize) / 2.0f);

	m_uOctantCount++;
}

/// Copy constructor for the octant to copy the data for the object passed in
Simplex::MyOctant::MyOctant(MyOctant const & other)
{
	m_nData = other.m_nData;
	m_lData = other.m_lData;
}

/// Copy assignment operator for the octant
Simplex::MyOctant & Simplex::MyOctant::operator=(MyOctant const & other)
{
	// If the passed in object is not the same
	if (this != &other)
	{
		// Release allocated memory, initialize values, 
		// store it in a temporary octant, swap, and then return the new octant
		Release();
		Init();
		MyOctant temp(other);
		Swap(temp);
	}
	return *this;
}

/// Octant Destructor
Simplex::MyOctant::~MyOctant() { Release(); }

/// Method needed for swapping data from this octant to the other using the std::swap method
void Simplex::MyOctant::Swap(MyOctant & other)
{
	std::swap(m_nData, other.m_nData);
	std::swap(m_lData, other.m_lData);
}

/// Method for checking if an entity is colliding with an octant using arbb against arbb collision
bool Simplex::MyOctant::IsColliding(uint a_uRBIndex)
{
	// get the entity by using the index passed in
	int objectCount = m_pEntityMngr->GetEntityCount();
	if (a_uRBIndex >= objectCount) return false;

	// retrieve minimum and maximum values of the entity
	MyEntity* entity = m_pEntityMngr->GetEntity(a_uRBIndex);
	MyRigidBody* rb = entity->GetRigidBody();
	vector3 min = rb->GetMinGlobal();
	vector3 max = rb->GetMaxGlobal();

	// arbb checks for both x and y
	if (m_v3Max.x < min.x) return false;
	if (m_v3Min.x > max.x) return false;
	if (m_v3Max.y < min.y) return false;
	if (m_v3Min.y > max.y) return false;
	if (m_v3Max.z < min.z) return false;
	if (m_v3Min.z > max.z) return false;
	return true;
}

/// Method to assist with displaying the oct tree, takes in an index, and a color as well
void Simplex::MyOctant::Display(uint a_nIndex, vector3 a_v3Color)
{
	// Loop through the child octants and use the other display method by passing in only the color
	for (size_t i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->Display(a_nIndex);
	}

	// Adds a cube that has the location of the center of the octant and the size of the octant as well
	if (m_uID == a_nIndex)
	{
		m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
	}
}

/// Method to assist with displaying the oct tree, takes in a color
void Simplex::MyOctant::Display(vector3 a_v3Color)
{
	// Shows all children octants and their child octants within the cube
	for (size_t i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->Display(a_v3Color);
	}

	// Adds a cube that has the location of the center of the octant and the size of the octant as well
	if (IsLeaf())
	{
		m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
	}
}

/// Method needed to clear the entity list for each of the child octants
void Simplex::MyOctant::ClearEntityList(void)
{
	for (size_t i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->ClearEntityList();
	}
	m_EntityList.clear();
}

/// Method to subdivide the oct tree when the +/- keys are pressed
void Simplex::MyOctant::Subdivide(void)
{
	if (m_uLevel >= m_uMaxLevel) return; //return if its at or greater than max level
	if (m_uChildren != 0) return; //return if it already has children

	m_uChildren = 8;

	// Sizes for each of the children
	float fSize = m_fSize / 4.0f;
	float newfSize = m_fSize / 2.0f;
	vector3 center;

	// Creating centers for the new octants
	center = m_v3Center;
	center.x -= fSize;
	center.y -= fSize;
	center.z -= fSize;
	m_pChild[0] = new MyOctant(center, newfSize);

	center.x += newfSize;
	m_pChild[1] = new MyOctant(center, newfSize);

	center.z += newfSize;
	m_pChild[2] = new MyOctant(center, newfSize);

	center.x -= newfSize;
	m_pChild[3] = new MyOctant(center, newfSize);

	center.y += newfSize;
	m_pChild[4] = new MyOctant(center, newfSize);

	center.z -= newfSize;
	m_pChild[5] = new MyOctant(center, newfSize);

	center.x += newfSize;
	m_pChild[6] = new MyOctant(center, newfSize);

	center.z += newfSize;
	m_pChild[7] = new MyOctant(center, newfSize);

	// Subdividing the children until max level is reached
	for (size_t i = 0; i < 8; i++)
	{
		m_pChild[i]->m_pRoot = m_pRoot;
		m_pChild[i]->m_pParent = this;
		m_pChild[i]->m_uLevel = m_uLevel + 1;
		m_pChild[i]->m_uIdealEntityCount = m_uIdealEntityCount;
		if (m_pChild[i]->ContainsMoreThan(m_uIdealEntityCount))
		{
			m_pChild[i]->Subdivide();
		}
	}
}

/// Method to get the parent of an octant
MyOctant * Simplex::MyOctant::GetChild(uint a_nChild)
{
	return nullptr;
}

/// Method to get the parent of an octant
MyOctant * Simplex::MyOctant::GetParent(void)
{
	return nullptr;
}

/// Method to determine if the octant is a leaf or not
bool Simplex::MyOctant::IsLeaf(void)
{
	//return true if there are no children octants
	if (m_uChildren == 0)
	{
		return true;
	}

	return false;
}

/// Method that returns true or false depending
/// if the octant contains more than the correct amount of entities
bool Simplex::MyOctant::ContainsMoreThan(uint a_nEntities)
{
	int count = 0;
	int objectCount = m_pEntityMngr->GetEntityCount();
	for (size_t i = 0; i < objectCount; i++)
	{
		if (IsColliding(i))
		{
			count++;
		}
		if (count > a_nEntities)
		{
			return true;
		}
	}
	return false;
}

/// Method for killing the branches within the oct tree
/// A loop is used to go through all the octants until an octant with no
/// children has been found, and then go back to the parent and delete the child
void Simplex::MyOctant::KillBranches(void)
{
	for (size_t i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->KillBranches();
		delete m_pChild[i];
		m_pChild[i] = nullptr;
	}
	m_uChildren = 0;
}

/// Method needed to construct the oct tree depending on the number passed in
void Simplex::MyOctant::ConstructTree(uint a_nMaxLevel)
{
	if (m_uLevel != 0) return;

	m_uMaxLevel = a_nMaxLevel;
	m_uOctantCount = 1;
	m_EntityList.clear();
	KillBranches();
	m_lChild.clear();

	if (ContainsMoreThan(m_uIdealEntityCount))
	{
		Subdivide();
	}

	AssignIDtoEntity();

	ConstructList();
}

/// Method for assigning the smallest leaf child to the entities colliding with it,
/// and assigning the ID to the entities in the children in order to reach the leaf
void Simplex::MyOctant::AssignIDtoEntity(void)
{
	// Go to the children that are leaves if there are children
	for (size_t i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->AssignIDtoEntity();
	}

	// If it is a leaf node
	if (m_uChildren == 0)
	{
		// Get all the entities and loop through all of them in order to check if it is colliding with the octant
		int entities = m_pEntityMngr->GetEntityCount();
		for (size_t i = 0; i < entities; i++)
		{
			// Add it to the entity list and add the dimension to the entity if it is colliding
			if (IsColliding(i))
			{
				m_EntityList.push_back(i);
				m_pEntityMngr->AddDimension(i, m_uID);
			}
		}
	}
}

/// Method for resetting and killing the branches depending if its a base octant or not
void Simplex::MyOctant::Release(void)
{
	if (m_uLevel == 0)
		KillBranches();

	m_uChildren = 0;
	m_fSize = 0.0f;
	m_lChild.clear();
}

/// Method needed to initialize the size, the octant count, 
/// the children count, the data, and the level of the octree
void Simplex::MyOctant::Init(void)
{
	m_nData = 0;
	m_uChildren = 0;
	m_fSize = 0.0f;
	m_uID = m_uOctantCount;
	m_uLevel = 0;

	//Center, max and min of the oct tree
	m_v3Center = vector3(0.0f);
	m_v3Min = vector3(0.0f);
	m_v3Max = vector3(0.0f);

	//Retrieve the mesh manager instance
	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	//Initialize the root and parent
	m_pRoot = nullptr;
	m_pParent = nullptr;

	//Initialize the children
	for (uint i = 0; i < 8; i++)
	{
		m_pChild[i] = nullptr;
	}
}

/// Method for constructing the list for the oct tree
void Simplex::MyOctant::ConstructList(void)
{
	// For loop to have each child call the same method
	for (size_t i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->ConstructList();
	}

	// Add it to the root's children
	if (m_EntityList.size() > 0)
	{
		m_pRoot->m_lChild.push_back(this);
	}
}

/* Accessors */
/// Method for returning the size of the side of the bounding box
float Simplex::MyOctant::GetSize(void) {return m_fSize;}

/// Method for returning the global position of center
vector3 Simplex::MyOctant::GetCenterGlobal(void) {return m_v3Center;}

/// Method for returning global position of minimum location
vector3 Simplex::MyOctant::GetMinGlobal(void){return vector3();}

/// Method for returning global position of maximum location
vector3 Simplex::MyOctant::GetMaxGlobal(void){return vector3();}

/// Method for returning the number of octants
uint Simplex::MyOctant::GetOctantCount(void) {return m_uOctantCount;}