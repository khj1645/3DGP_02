#pragma once
#include "Mesh.h"

class CUIRectMesh : public CMesh
{
public:
    CUIRectMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float x, float y, float width, float height);
    virtual ~CUIRectMesh();

    void SetUVRect(float u0, float v0, float u1, float v1);

    virtual void ReleaseUploadBuffers();
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);

    float GetNormalizedX() const { return m_normalizedX; }
    float GetNormalizedY() const { return m_normalizedY; }
    float GetNormalizedWidth() const { return m_normalizedWidth; }
    float GetNormalizedHeight() const { return m_normalizedHeight; }

protected:
    XMFLOAT2* m_pxmf2TextureCoords0 = NULL;

    ID3D12Resource* m_pd3dTextureCoord0Buffer = NULL;
    ID3D12Resource* m_pd3dTextureCoord0UploadBuffer = NULL;
    D3D12_VERTEX_BUFFER_VIEW m_d3dTextureCoord0BufferView;

    ID3D12Resource* m_pd3dIndexBuffer = NULL;
    ID3D12Resource* m_pd3dIndexUploadBuffer = NULL;
    D3D12_INDEX_BUFFER_VIEW m_d3dIndexBufferView;

    int m_nIndices = 0;

    float m_normalizedX;
    float m_normalizedY;
    float m_normalizedWidth;
    float m_normalizedHeight;
};
