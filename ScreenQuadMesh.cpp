#include "stdafx.h"
#include "ScreenQuadMesh.h"

CScreenQuadMesh::CScreenQuadMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_nVertices = 4;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// NDC(Normalized Device Coordinates) 공간의 정점 위치를 정의합니다. (-1~1)
	// 이렇게 하면 뷰/투영 행렬을 단위행렬로 설정 시 화면을 꽉 채우게 됩니다.
	m_pxmf3Positions = new XMFLOAT3[m_nVertices];
	m_pxmf3Positions[0] = XMFLOAT3(-1.0f, 1.0f, 0.0f);
	m_pxmf3Positions[1] = XMFLOAT3(1.0f, 1.0f, 0.0f);
	m_pxmf3Positions[2] = XMFLOAT3(1.0f, -1.0f, 0.0f);
	m_pxmf3Positions[3] = XMFLOAT3(-1.0f, -1.0f, 0.0f);

	// 텍스처 UV 좌표를 정의합니다.
	m_pxmf2TextureCoords0 = new XMFLOAT2[m_nVertices];
	m_pxmf2TextureCoords0[0] = XMFLOAT2(0.0f, 0.0f);
	m_pxmf2TextureCoords0[1] = XMFLOAT2(1.0f, 0.0f);
	m_pxmf2TextureCoords0[2] = XMFLOAT2(1.0f, 1.0f);
	m_pxmf2TextureCoords0[3] = XMFLOAT2(0.0f, 1.0f);

	
	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);
	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	
	m_pd3dTextureCoord0Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord0UploadBuffer);
	m_d3dTextureCoord0BufferView.BufferLocation = m_pd3dTextureCoord0Buffer->GetGPUVirtualAddress();
	m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

	
	m_nIndices = 6;
	UINT pIndices[6] = { 0, 1, 2, 0, 2, 3 };
	m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pIndices, sizeof(UINT) * m_nIndices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pd3dIndexUploadBuffer);
	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_nIndices;
}

CScreenQuadMesh::~CScreenQuadMesh()
{
	if (m_pd3dTextureCoord0Buffer) m_pd3dTextureCoord0Buffer->Release();
	if (m_pxmf2TextureCoords0) delete[] m_pxmf2TextureCoords0;
	if (m_pd3dIndexBuffer) m_pd3dIndexBuffer->Release();
}

void CScreenQuadMesh::ReleaseUploadBuffers()
{
	// CMesh의 ReleaseUploadBuffers는 m_pd3dPositionUploadBuffer를 해제합니다.
	CMesh::ReleaseUploadBuffers();

	if (m_pd3dTextureCoord0UploadBuffer) m_pd3dTextureCoord0UploadBuffer->Release();
	m_pd3dTextureCoord0UploadBuffer = NULL;

	if (m_pd3dIndexUploadBuffer) m_pd3dIndexUploadBuffer->Release();
	m_pd3dIndexUploadBuffer = NULL;
}

void CScreenQuadMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	// 위치와 텍스처 좌표, 2개의 정점 버퍼를 설정합니다.
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[2] = { m_d3dPositionBufferView, m_d3dTextureCoord0BufferView };
	pd3dCommandList->IASetVertexBuffers(0, 2, pVertexBufferViews);
	// 인덱스 버퍼를 설정하고 인덱스 기반으로 렌더링합니다.
	pd3dCommandList->IASetIndexBuffer(&m_d3dIndexBufferView);
	pd3dCommandList->DrawIndexedInstanced(m_nIndices, 1, 0, 0, 0);
}
