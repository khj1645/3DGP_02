#pragma once

#include "Object.h"
#include "WaterShader.h"
// #include "TexturedRectMesh.h" // Removed as we use the existing one from Mesh.h

class CWaterObject : public CGameObject
{
public:
    using CGameObject::Render; // Make base class Render functions visible

    CWaterObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CWaterShader* pWaterShader, float fWidth, float fLength);
    virtual ~CWaterObject();
   

    // No custom Animate or Render needed if base CGameObject methods are sufficient
    // virtual void Animate(float fTimeElapsed);
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, const XMMATRIX& xmmtxReflection);
};
