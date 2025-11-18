#include "stdafx.h"
#include "WaterObject.h"
#include "Scene.h" // For CScene::CreateShaderResourceViews
#include "Object.h" // Explicitly include Object.h for CB_GAMEOBJECT_INFO and CGameObject members

CWaterObject::CWaterObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CWaterShader* pWaterShader, float fWidth, float fLength) : CGameObject(1, 1)
{
    // 1. 물 메시 생성 및 할당
    // fHeight, fDepth, fxPosition, fyPosition, fzPosition는 물 평면이므로 0으로 설정
    CTexturedRectMesh* pWaterMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, fWidth, 0.0f, fLength, 0.0f, 0.0f, 0.0f);
    SetMesh(0, pWaterMesh);

    // 2. 셰이더 변수 생성 (CGameObject::CreateShaderVariables 호출) - 사용자 프로젝트에서는 필요 없음
    // CreateShaderVariables(pd3dDevice, pd3dCommandList); 

    // 3. 물 텍스처 로드 및 SRV 생성
    CTexture* pWaterTexture = new CTexture(3, RESOURCE_TEXTURE2D, 0, 1); // 3개의 텍스처, RESOURCE_TEXTURE2D 타입

    // 텍스처 파일 경로 및 할당된 레지스터 번호에 맞게 수정
    pWaterTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Water_Base_Texture_0.dds", RESOURCE_TEXTURE2D, 0); // t6
    pWaterTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Water_Detail_Texture_0.dds", RESOURCE_TEXTURE2D, 1); // t7
    pWaterTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/WaveFoam.dds", RESOURCE_TEXTURE2D, 2); // t8

    
    CScene::CreateShaderResourceViews(pd3dDevice, pWaterTexture, 0, 13); 
    // 4. 물 셰이더는 외부에서 생성하여 전달받음

    // 6. 재질 생성 및 텍스처/셰이더 할당
    CMaterial* pWaterMaterial = new CMaterial();
    pWaterMaterial->SetTexture(pWaterTexture);
    pWaterMaterial->SetShader(pWaterShader);     // 셰이더를 재질에 직접 할당하여 소유권 관리

    // Explicitly ensure material array is set up, in case CGameObject constructor is not fully effective
    if (m_nMaterials == 0) {
        m_nMaterials = 1;
        m_ppMaterials = new CMaterial*[m_nMaterials];
        m_ppMaterials[0] = NULL;
        
    } else if (m_ppMaterials == NULL) {
        m_ppMaterials = new CMaterial*[m_nMaterials];
        m_ppMaterials[0] = NULL;
        
    }

    SetMaterial(0, pWaterMaterial); // CWaterObject에 재질 할당 (이 재질이 셰이더를 소유)
}

CWaterObject::~CWaterObject()
{
}

void CWaterObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, const XMMATRIX& xmmtxReflection)
{
	CGameObject::Render(pd3dCommandList, pCamera, xmmtxReflection, 1); 
}
