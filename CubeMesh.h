#pragma once
#include "Mesh.h"

class CCubeMesh : public CStandardMesh
{
public:
	CCubeMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f);
	virtual ~CCubeMesh();
};
