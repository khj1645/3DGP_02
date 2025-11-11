//-----------------------------------------------------------------------------
// File: PointMesh.h
//-----------------------------------------------------------------------------

#pragma once

#include "Mesh.h"

class CPointMesh : public CMesh
{
public:
	CPointMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nPoints, XMFLOAT3 *pxmf3Positions);
	virtual ~CPointMesh();

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, int nSubSet);
};
