/*----------------------------------------------
Programmer: Christopher Lawrence (cml3051@rit.edu)
Date: 07/2019
----------------------------------------------*/
#include "MyOctant.h"
using namespace Simplex;

uint MyOctant::m_uOctantCount = 0;
uint MyOctant::m_uMaxLevel = 0;
uint MyOctant::m_uIdealEntityCount = 0;

// constructors
Simplex::MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
	Init();

	m_uOctantCount = 0;
	m_uMaxLevel = a_nMaxLevel;
	m_uIdealEntityCount = a_nIdealEntityCount;
	m_uID = m_uOctantCount;

	m_pRoot = this;
	m_lChild.clear();

	std::vector<vector3> lMinMax;

	uint iObjects = m_pEntityMngr->GetEntityCount();
	for (size_t i = 0; i < iObjects; i++)
	{
		MyEntity* pEntity = m_pEntityMngr->GetEntity(i);
		MyRigidBody* pRB = pEntity->GetRigidBody();

		lMinMax.push_back(pRB->GetMinGlobal());
		lMinMax.push_back(pRB->GetMaxGlobal());
	}

	MyRigidBody* pRigidBody = new MyRigidBody(lMinMax);

	vector3 halfWidth = pRigidBody->GetHalfWidth();

	float fMax = halfWidth.x;
	for (size_t i = 0; i < 3; i++)
	{
		if (fMax < halfWidth[i])
		{
			fMax = halfWidth[i];
		}
	}

	vector3 center = pRigidBody->GetCenterLocal();
	lMinMax.clear();
	SafeDelete(pRigidBody);

	m_fSize = fMax * 2.0f;
	m_v3Center = center;
	m_v3Min = m_v3Center - (vector3(fMax));
	m_v3Max = m_v3Center + (vector3(fMax));

	m_uOctantCount++;

	ConstructTree(m_uMaxLevel);
}

Simplex::MyOctant::MyOctant(vector3 a_v3Center, float a_fSize)
{
	Init();
	m_v3Center = a_v3Center;
	m_fSize = a_fSize;

	m_v3Min = m_v3Center - (vector3(m_fSize) / 2.0f);
	m_v3Max = m_v3Center + (vector3(m_fSize) / 2.0f);

	m_uOctantCount++;
}

Simplex::MyOctant::MyOctant(MyOctant const & other)
{
	m_nData = other.m_nData;
	m_lData = other.m_lData;
}

Simplex::MyOctant & Simplex::MyOctant::operator=(MyOctant const & other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyOctant temp(other);
		Swap(temp);
	}
	return *this;
}

Simplex::MyOctant::~MyOctant() { Release(); }

void Simplex::MyOctant::Swap(MyOctant & other)
{
	std::swap(m_nData, other.m_nData);
	std::swap(m_lData, other.m_lData);
}

bool Simplex::MyOctant::IsColliding(uint a_uRBIndex)
{
	int objectCount = m_pEntityMngr->GetEntityCount();
	if (a_uRBIndex >= objectCount) return false;

	MyEntity* entity = m_pEntityMngr->GetEntity(a_uRBIndex);
	MyRigidBody* rb = entity->GetRigidBody();
	vector3 min = rb->GetMinGlobal();
	vector3 max = rb->GetMaxGlobal();

	if (m_v3Max.x < min.x) return false;
	if (m_v3Min.x > max.x) return false;
	if (m_v3Max.y < min.y) return false;
	if (m_v3Min.y > max.y) return false;
	if (m_v3Max.z < min.z) return false;
	if (m_v3Min.z > max.z) return false;
	return true;
}

void Simplex::MyOctant::Display(uint a_nIndex, vector3 a_v3Color)
{
	if (m_uID == a_nIndex)
	{
		m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
		return;
	}

	for (size_t i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->Display(a_v3Color);
	}
}

void Simplex::MyOctant::Display(vector3 a_v3Color)
{
	for (size_t i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->Display(a_v3Color);
	}

	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
}

void Simplex::MyOctant::DisplayLeafs(vector3 a_v3Color)
{
}

void Simplex::MyOctant::ClearEntityList(void)
{
	for (size_t i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->ClearEntityList();
	}
	m_EntityList.clear();
}

void Simplex::MyOctant::Subdivide(void)
{
	if (m_uLevel >= m_uMaxLevel) return;
	if (m_uChildren != 0) return;

	m_uChildren = 8;

	float fSize = m_fSize / 4.0f;
	float newfSize = m_fSize / 2.0f;
	vector3 center;

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

MyOctant * Simplex::MyOctant::GetChild(uint a_nChild)
{
	return nullptr;
}

MyOctant * Simplex::MyOctant::GetParent(void)
{
	return nullptr;
}

bool Simplex::MyOctant::IsLeaf(void)
{
	return false;
}

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

void Simplex::MyOctant::AssignIDtoEntity(void)
{
	for (size_t i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->AssignIDtoEntity();
	}

	if (m_uChildren == 0)
	{
		int entities = m_pEntityMngr->GetEntityCount();
		for (size_t i = 0; i < entities; i++)
		{
			if (IsColliding(i))
			{
				m_EntityList.push_back(i);
				m_pEntityMngr->AddDimension(i, m_uID);
			}
		}
	}
}

void Simplex::MyOctant::Release(void)
{
	if (m_uLevel == 0)
		KillBranches();

	m_uChildren = 0;
	m_fSize = 0.0f;
	m_lChild.clear();
}

void Simplex::MyOctant::Init(void)
{
	m_nData = 0;
	m_uChildren = 0;
	m_fSize = 0.0f;
	m_uID = m_uOctantCount;
	m_uLevel = 0;

	m_v3Center = vector3(0.0f);
	m_v3Min = vector3(0.0f);
	m_v3Max = vector3(0.0f);

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	m_pRoot = nullptr;
	m_pParent = nullptr;

	for (uint i = 0; i < 8; i++)
	{
		m_pChild[i] = nullptr;
	}
}

void Simplex::MyOctant::ConstructList(void)
{
	for (size_t i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->ConstructList();
	}

	if (m_EntityList.size() > 0)
	{
		m_pRoot->m_lChild.push_back(this);
	}
}

// Accessors
float Simplex::MyOctant::GetSize(void) {return m_fSize;}

vector3 Simplex::MyOctant::GetCenterGlobal(void) {return m_v3Center;}

vector3 Simplex::MyOctant::GetMinGlobal(void){return vector3();}

vector3 Simplex::MyOctant::GetMaxGlobal(void){return vector3();}

uint Simplex::MyOctant::GetOctantCount(void) {return m_uOctantCount;}