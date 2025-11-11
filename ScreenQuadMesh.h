#pragma once
#include "Mesh.h"

class CScreenQuadMesh : public CMesh
{
public:
	CScreenQuadMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CScreenQuadMesh();

	virtual void ReleaseUploadBuffers();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, int nSubSet);

protected:
	XMFLOAT2*						m_pxmf2TextureCoords0 = NULL;

	ID3D12Resource*					m_pd3dTextureCoord0Buffer = NULL;
	ID3D12Resource*					m_pd3dTextureCoord0UploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dTextureCoord0BufferView;

	ID3D12Resource*					m_pd3dIndexBuffer = NULL;
	ID3D12Resource*					m_pd3dIndexUploadBuffer = NULL;
	D3D12_INDEX_BUFFER_VIEW			m_d3dIndexBufferView;

	int								m_nIndices = 0;
};
