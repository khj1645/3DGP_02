#pragma once

#include "Object.h"
#include "Shader.h"

class CBillboardObject : public CGameObject
{
public:
    CBillboardObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, XMFLOAT3 xmf3Position, CShader* pShader, CTexture* pTexture);
    virtual ~CBillboardObject();

    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
};
