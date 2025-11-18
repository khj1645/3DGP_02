//-----------------------------------------------------------------------------
// File: PointMesh.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PointMesh.h"

CPointMesh::CPointMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nPoints, XMFLOAT3 *pxmf3Positions)
    : CMesh(pd3dDevice, pd3dCommandList)
{
    m_nVertices = nPoints;
    m_nType = VERTEXT_POSITION; // Only position data

    m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST; // Set topology to point list

    m_pxmf3Positions = new XMFLOAT3[m_nVertices];
    for (int i = 0; i < m_nVertices; i++)
    {
        m_pxmf3Positions[i] = pxmf3Positions[i];
    }

    // Create vertex buffer for positions
    m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

    m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
    m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
    m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
}

CPointMesh::~CPointMesh()
{
}

void CPointMesh::Render(ID3D12GraphicsCommandList *pd3dCommandList, int nSubSet)
{
    pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
    pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dPositionBufferView);
    pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
}
