#include "stdafx.h"
#include "UIRectMesh.h"

CUIRectMesh::CUIRectMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float x, float y, float width, float height) : CMesh(pd3dDevice, pd3dCommandList)
{
    m_nVertices = 4;
    m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    m_normalizedX = x;
    m_normalizedY = y;
    m_normalizedWidth = width;
    m_normalizedHeight = height;

    
    float ndcX = (x * 2.0f) - 1.0f;
    float ndcY = 1.0f - (y * 2.0f);
    float ndcWidth = width * 2.0f;
    float ndcHeight = height * 2.0f;

    m_pxmf3Positions = new XMFLOAT3[m_nVertices];
    m_pxmf3Positions[0] = XMFLOAT3(ndcX, ndcY, 0.0f); 
    m_pxmf3Positions[1] = XMFLOAT3(ndcX + ndcWidth, ndcY, 0.0f); 
    m_pxmf3Positions[2] = XMFLOAT3(ndcX + ndcWidth, ndcY - ndcHeight, 0.0f); 
    m_pxmf3Positions[3] = XMFLOAT3(ndcX, ndcY - ndcHeight, 0.0f); 

    m_pxmf2TextureCoords0 = new XMFLOAT2[m_nVertices];
    m_pxmf2TextureCoords0[0] = XMFLOAT2(0.0f, 0.0f);
    m_pxmf2TextureCoords0[1] = XMFLOAT2(1.0f, 0.0f);
    m_pxmf2TextureCoords0[2] = XMFLOAT2(1.0f, 1.0f);
    m_pxmf2TextureCoords0[3] = XMFLOAT2(0.0f, 1.0f);

    m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);
    m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
    m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
    m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

    // This buffer will be updated frequently from the CPU, so create it in an UPLOAD heap.
    D3D12_HEAP_PROPERTIES d3dHeapProperties = { D3D12_HEAP_TYPE_UPLOAD };
    D3D12_RESOURCE_DESC d3dResourceDesc = { D3D12_RESOURCE_DIMENSION_BUFFER };
    d3dResourceDesc.Width = sizeof(XMFLOAT2) * m_nVertices;
    d3dResourceDesc.Height = 1;
    d3dResourceDesc.DepthOrArraySize = 1;
    d3dResourceDesc.MipLevels = 1;
    d3dResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    d3dResourceDesc.SampleDesc.Count = 1;
    d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    pd3dDevice->CreateCommittedResource(
        &d3dHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &d3dResourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        NULL,
        __uuidof(ID3D12Resource),
        (void**)&m_pd3dTextureCoord0Buffer);

    // Copy the initial UV data to the buffer.
    UINT8* pData;
    D3D12_RANGE readRange = { 0, 0 };
    m_pd3dTextureCoord0Buffer->Map(0, &readRange, (void**)&pData);
    memcpy(pData, m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices);
    m_pd3dTextureCoord0Buffer->Unmap(0, NULL);

    // The separate upload buffer is not needed for this resource.
    m_pd3dTextureCoord0UploadBuffer = NULL;

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

CUIRectMesh::~CUIRectMesh()
{
    if (m_pd3dTextureCoord0Buffer) m_pd3dTextureCoord0Buffer->Release();
    if (m_pxmf2TextureCoords0) delete[] m_pxmf2TextureCoords0;
    if (m_pd3dIndexBuffer) m_pd3dIndexBuffer->Release();
}

void CUIRectMesh::ReleaseUploadBuffers()
{
    CMesh::ReleaseUploadBuffers();

    if (m_pd3dTextureCoord0UploadBuffer) m_pd3dTextureCoord0UploadBuffer->Release();
    m_pd3dTextureCoord0UploadBuffer = NULL;

    if (m_pd3dIndexUploadBuffer) m_pd3dIndexUploadBuffer->Release();
    m_pd3dIndexUploadBuffer = NULL;
}

void CUIRectMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
    pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
    D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[2] = { m_d3dPositionBufferView, m_d3dTextureCoord0BufferView };
    pd3dCommandList->IASetVertexBuffers(0, 2, pVertexBufferViews);
    pd3dCommandList->IASetIndexBuffer(&m_d3dIndexBufferView);
    pd3dCommandList->DrawIndexedInstanced(m_nIndices, 1, 0, 0, 0);
}

void CUIRectMesh::SetUVRect(float u0, float v0, float u1, float v1)
{
	// This mesh is assumed to always have 4 vertices (TL, TR, BR, BL)
	XMFLOAT2 newUVs[4] =
	{
		XMFLOAT2(u0, v0), // Top-Left
		XMFLOAT2(u1, v0), // Top-Right
		XMFLOAT2(u1, v1), // Bottom-Right
		XMFLOAT2(u0, v1)  // Bottom-Left
	};

	XMFLOAT2* pMappedUV = nullptr;
	// We do not intend to read from this resource on the CPU.
	D3D12_RANGE readRange = { 0, 0 }; 

	HRESULT hResult = m_pd3dTextureCoord0Buffer->Map(
		0, &readRange, reinterpret_cast<void**>(&pMappedUV));

	if (FAILED(hResult) || !pMappedUV)
	{
		// Handle error: failed to map buffer
		return;
	}

	memcpy(pMappedUV, newUVs, sizeof(newUVs));

	m_pd3dTextureCoord0Buffer->Unmap(0, nullptr);
}
