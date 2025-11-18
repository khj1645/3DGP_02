#include "stdafx.h"
#include "BillboardObject.h"
#include "PointMesh.h" // For CPointMesh

CBillboardObject::CBillboardObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, XMFLOAT3 xmf3Position, CShader* pShader, CTexture* pTexture)
    : CGameObject(1, 1) // 1 mesh, 1 material
{
    // Create a CPointMesh for the billboard
    XMFLOAT3* pxmf3Positions = new XMFLOAT3[1];
    pxmf3Positions[0] = xmf3Position;
    CMesh* pMesh = new CPointMesh(pd3dDevice, pd3dCommandList, 1, pxmf3Positions);
    SetMesh(0, pMesh);
    delete[] pxmf3Positions; // The mesh copies the data, so we can delete the array

    // Create a material for the billboard
    CMaterial* pMaterial = new CMaterial();
    pMaterial->SetShader(pShader);
    pMaterial->SetTexture(pTexture);
    SetMaterial(0, pMaterial);

    // Set the initial position of the billboard
    SetPosition(xmf3Position);

    // Update the world transform
    UpdateTransform(NULL);
}

CBillboardObject::~CBillboardObject()
{
}

void CBillboardObject::Animate(float fTimeElapsed)
{
    if (!m_bIsActive) return;

    m_fElapsedTime += fTimeElapsed;
    if (m_fElapsedTime > m_fDuration)
    {
        m_bIsActive = false;
    }
}

void CBillboardObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    if (!m_bIsActive) return;

    XMFLOAT3 pos = GetPosition();
    CGameObject::Render(pd3dCommandList, pCamera);
}
