#pragma once

#include "Object.h"
#include "Shader.h"

class CBillboardObject : public CGameObject
{
protected:
    float m_fDuration = 1.0f; // Animation duration
    float m_fElapsedTime = 0.0f; // Time since animation started
    bool m_bIsActive = true; // Is the animation currently active?

public:
    CBillboardObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, XMFLOAT3 xmf3Position, CShader* pShader, CTexture* pTexture);
    virtual ~CBillboardObject();

    virtual void Animate(float fTimeElapsed);
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

    bool IsActive() const { return m_bIsActive; }
};
