//-----------------------------------------------------------------------------
// File: MirrorShader.h
//-----------------------------------------------------------------------------

#pragma once
#include "Shader.h"
#include "Scene.h"

class CGameObject;
class CPlayer;

class CMirrorShader : public CShader

{

public:

	CMirrorShader(CScene* pScene, CGameObject* pMirrorObject);

	virtual ~CMirrorShader();



	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();



	virtual D3D12_SHADER_BYTECODE CreateVertexShader();



	virtual D3D12_SHADER_BYTECODE CreatePixelShader();







	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) override;



	void CreatePipelineStates(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature);







	void PreRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);







	void RenderReflectedObjects(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera); // 사용자 제안: 다시 추가







	void PostRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	void RenderBackDepth(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
	{
		if (!m_pMirrorBackObject || !m_ppd3dPipelineStates[3] || !m_pMirrorObject) return;

		// Update the back object's transform to match the main mirror
		m_pMirrorBackObject->m_xmf4x4World = m_pMirrorObject->m_xmf4x4World;

		pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[3]);
		m_pMirrorBackObject->Render(pd3dCommandList, pCamera,3);
	}

private:



	void ReflectLights(ID3D12GraphicsCommandList* pd3dCommandList, const XMVECTOR& xmvMirrorPlane);



	void RestoreLights(ID3D12GraphicsCommandList* pd3dCommandList);







	CScene* m_pScene = nullptr;



	CGameObject* m_pMirrorObject = nullptr;



	CGameObject* m_pMirrorBackObject = nullptr; // For optional back-face rendering







	LIGHT* m_pOriginalLights = nullptr;



	int m_nOriginalLights = 0;



};