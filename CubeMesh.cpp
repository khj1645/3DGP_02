#include "stdafx.h"
#include "CubeMesh.h"

CCubeMesh::CCubeMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth)
    : CStandardMesh(pd3dDevice, pd3dCommandList)
{
    float fhw = fWidth * 0.5f;
    float fhh = fHeight * 0.5f;
    float fhd = fDepth * 0.5f;

    m_nVertices = 24;
    m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    m_pxmf3Positions = new XMFLOAT3[m_nVertices];
    m_pxmf3Normals = new XMFLOAT3[m_nVertices];
    m_pxmf2TextureCoords0 = new XMFLOAT2[m_nVertices];

    // Front
    m_pxmf3Positions[0] = XMFLOAT3(-fhw, +fhh, -fhd); m_pxmf3Normals[0] = XMFLOAT3(0.0f, 0.0f, -1.0f); m_pxmf2TextureCoords0[0] = XMFLOAT2(0.0f, 0.0f);
    m_pxmf3Positions[1] = XMFLOAT3(+fhw, +fhh, -fhd); m_pxmf3Normals[1] = XMFLOAT3(0.0f, 0.0f, -1.0f); m_pxmf2TextureCoords0[1] = XMFLOAT2(1.0f, 0.0f);
    m_pxmf3Positions[2] = XMFLOAT3(-fhw, -fhh, -fhd); m_pxmf3Normals[2] = XMFLOAT3(0.0f, 0.0f, -1.0f); m_pxmf2TextureCoords0[2] = XMFLOAT2(0.0f, 1.0f);
    m_pxmf3Positions[3] = XMFLOAT3(+fhw, -fhh, -fhd); m_pxmf3Normals[3] = XMFLOAT3(0.0f, 0.0f, -1.0f); m_pxmf2TextureCoords0[3] = XMFLOAT2(1.0f, 1.0f);
    // Back
    m_pxmf3Positions[4] = XMFLOAT3(+fhw, +fhh, +fhd); m_pxmf3Normals[4] = XMFLOAT3(0.0f, 0.0f, +1.0f); m_pxmf2TextureCoords0[4] = XMFLOAT2(0.0f, 0.0f);
    m_pxmf3Positions[5] = XMFLOAT3(-fhw, +fhh, +fhd); m_pxmf3Normals[5] = XMFLOAT3(0.0f, 0.0f, +1.0f); m_pxmf2TextureCoords0[5] = XMFLOAT2(1.0f, 0.0f);
    m_pxmf3Positions[6] = XMFLOAT3(+fhw, -fhh, +fhd); m_pxmf3Normals[6] = XMFLOAT3(0.0f, 0.0f, +1.0f); m_pxmf2TextureCoords0[6] = XMFLOAT2(0.0f, 1.0f);
    m_pxmf3Positions[7] = XMFLOAT3(-fhw, -fhh, +fhd); m_pxmf3Normals[7] = XMFLOAT3(0.0f, 0.0f, +1.0f); m_pxmf2TextureCoords0[7] = XMFLOAT2(1.0f, 1.0f);
    // Top
    m_pxmf3Positions[8] = XMFLOAT3(-fhw, +fhh, +fhd); m_pxmf3Normals[8] = XMFLOAT3(0.0f, +1.0f, 0.0f); m_pxmf2TextureCoords0[8] = XMFLOAT2(0.0f, 0.0f);
    m_pxmf3Positions[9] = XMFLOAT3(+fhw, +fhh, +fhd); m_pxmf3Normals[9] = XMFLOAT3(0.0f, +1.0f, 0.0f); m_pxmf2TextureCoords0[9] = XMFLOAT2(1.0f, 0.0f);
    m_pxmf3Positions[10] = XMFLOAT3(-fhw, +fhh, -fhd); m_pxmf3Normals[10] = XMFLOAT3(0.0f, +1.0f, 0.0f); m_pxmf2TextureCoords0[10] = XMFLOAT2(0.0f, 1.0f);
    m_pxmf3Positions[11] = XMFLOAT3(+fhw, +fhh, -fhd); m_pxmf3Normals[11] = XMFLOAT3(0.0f, +1.0f, 0.0f); m_pxmf2TextureCoords0[11] = XMFLOAT2(1.0f, 1.0f);
    // Bottom
    m_pxmf3Positions[12] = XMFLOAT3(-fhw, -fhh, -fhd); m_pxmf3Normals[12] = XMFLOAT3(0.0f, -1.0f, 0.0f); m_pxmf2TextureCoords0[12] = XMFLOAT2(0.0f, 0.0f);
    m_pxmf3Positions[13] = XMFLOAT3(+fhw, -fhh, -fhd); m_pxmf3Normals[13] = XMFLOAT3(0.0f, -1.0f, 0.0f); m_pxmf2TextureCoords0[13] = XMFLOAT2(1.0f, 0.0f);
    m_pxmf3Positions[14] = XMFLOAT3(-fhw, -fhh, +fhd); m_pxmf3Normals[14] = XMFLOAT3(0.0f, -1.0f, 0.0f); m_pxmf2TextureCoords0[14] = XMFLOAT2(0.0f, 1.0f);
    m_pxmf3Positions[15] = XMFLOAT3(+fhw, -fhh, +fhd); m_pxmf3Normals[15] = XMFLOAT3(0.0f, -1.0f, 0.0f); m_pxmf2TextureCoords0[15] = XMFLOAT2(1.0f, 1.0f);
    // Right
    m_pxmf3Positions[16] = XMFLOAT3(+fhw, +fhh, -fhd); m_pxmf3Normals[16] = XMFLOAT3(+1.0f, 0.0f, 0.0f); m_pxmf2TextureCoords0[16] = XMFLOAT2(0.0f, 0.0f);
    m_pxmf3Positions[17] = XMFLOAT3(+fhw, +fhh, +fhd); m_pxmf3Normals[17] = XMFLOAT3(+1.0f, 0.0f, 0.0f); m_pxmf2TextureCoords0[17] = XMFLOAT2(1.0f, 0.0f);
    m_pxmf3Positions[18] = XMFLOAT3(+fhw, -fhh, -fhd); m_pxmf3Normals[18] = XMFLOAT3(+1.0f, 0.0f, 0.0f); m_pxmf2TextureCoords0[18] = XMFLOAT2(0.0f, 1.0f);
    m_pxmf3Positions[19] = XMFLOAT3(+fhw, -fhh, +fhd); m_pxmf3Normals[19] = XMFLOAT3(+1.0f, 0.0f, 0.0f); m_pxmf2TextureCoords0[19] = XMFLOAT2(1.0f, 1.0f);
    // Left
    m_pxmf3Positions[20] = XMFLOAT3(-fhw, +fhh, +fhd); m_pxmf3Normals[20] = XMFLOAT3(-1.0f, 0.0f, 0.0f); m_pxmf2TextureCoords0[20] = XMFLOAT2(0.0f, 0.0f);
    m_pxmf3Positions[21] = XMFLOAT3(-fhw, +fhh, -fhd); m_pxmf3Normals[21] = XMFLOAT3(-1.0f, 0.0f, 0.0f); m_pxmf2TextureCoords0[21] = XMFLOAT2(1.0f, 0.0f);
    m_pxmf3Positions[22] = XMFLOAT3(-fhw, -fhh, +fhd); m_pxmf3Normals[22] = XMFLOAT3(-1.0f, 0.0f, 0.0f); m_pxmf2TextureCoords0[22] = XMFLOAT2(0.0f, 1.0f);
    m_pxmf3Positions[23] = XMFLOAT3(-fhw, -fhh, -fhd); m_pxmf3Normals[23] = XMFLOAT3(-1.0f, 0.0f, 0.0f); m_pxmf2TextureCoords0[23] = XMFLOAT2(1.0f, 1.0f);

    m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);
    m_pd3dNormalBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Normals, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dNormalUploadBuffer);
    m_pd3dTextureCoord0Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord0UploadBuffer);

    m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
    m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
    m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

    m_d3dNormalBufferView.BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
    m_d3dNormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
    m_d3dNormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

    m_d3dTextureCoord0BufferView.BufferLocation = m_pd3dTextureCoord0Buffer->GetGPUVirtualAddress();
    m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
    m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

    m_nSubMeshes = 1;
    m_ppnSubSetIndices = new UINT * [m_nSubMeshes];
    m_pnSubSetIndices = new int[m_nSubMeshes];
    m_ppd3dSubSetIndexBuffers = new ID3D12Resource * [m_nSubMeshes];
    m_ppd3dSubSetIndexUploadBuffers = new ID3D12Resource * [m_nSubMeshes];
    m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubMeshes];

    UINT* pIndices = new UINT[36];
    int n = 0;
    // Front
    pIndices[n++] = 0; pIndices[n++] = 1; pIndices[n++] = 2; pIndices[n++] = 2; pIndices[n++] = 1; pIndices[n++] = 3;
    // Back
    pIndices[n++] = 4; pIndices[n++] = 5; pIndices[n++] = 6; pIndices[n++] = 6; pIndices[n++] = 5; pIndices[n++] = 7;
    // Top
    pIndices[n++] = 8; pIndices[n++] = 9; pIndices[n++] = 10; pIndices[n++] = 10; pIndices[n++] = 9; pIndices[n++] = 11;
    // Bottom
    pIndices[n++] = 12; pIndices[n++] = 13; pIndices[n++] = 14; pIndices[n++] = 14; pIndices[n++] = 13; pIndices[n++] = 15;
    // Right
    pIndices[n++] = 16; pIndices[n++] = 17; pIndices[n++] = 18; pIndices[n++] = 18; pIndices[n++] = 17; pIndices[n++] = 19;
    // Left
    pIndices[n++] = 20; pIndices[n++] = 21; pIndices[n++] = 22; pIndices[n++] = 22; pIndices[n++] = 21; pIndices[n++] = 23;

    m_pnSubSetIndices[0] = 36;
    m_ppnSubSetIndices[0] = pIndices;

    m_ppd3dSubSetIndexBuffers[0] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_ppnSubSetIndices[0], sizeof(UINT) * m_pnSubSetIndices[0], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_ppd3dSubSetIndexUploadBuffers[0]);

    m_pd3dSubSetIndexBufferViews[0].BufferLocation = m_ppd3dSubSetIndexBuffers[0]->GetGPUVirtualAddress();
    m_pd3dSubSetIndexBufferViews[0].Format = DXGI_FORMAT_R32_UINT;
    m_pd3dSubSetIndexBufferViews[0].SizeInBytes = sizeof(UINT) * m_pnSubSetIndices[0];
}

CCubeMesh::~CCubeMesh()
{
}