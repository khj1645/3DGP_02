//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"
#include "GameFramework.h" // Added for GameFramework access
#include "UIRectMesh.h" // Added for UI Rect Mesh
#include "ScreenQuadMesh.h" // Added for Screen Quad Mesh
#include "UIShader.h" // Added for UI Shader
#include "Object.h"
#include "WaterObject.h" // Added for CWaterObject
#include "WaterShader.h" // Added for CWaterShader
#include "GameFramework.h"
#include "ScreenQuadMesh.h"
#include "UIShader.h"
#include "MirrorShader.h"
#include "Mesh.h"

CDescriptorHeap* CScene::m_pDescriptorHeap = NULL;

CDescriptorHeap::CDescriptorHeap()
{
	m_d3dSrvCPUDescriptorStartHandle.ptr = NULL;
	m_d3dSrvGPUDescriptorStartHandle.ptr = NULL;
}

CDescriptorHeap::~CDescriptorHeap()
{
	if (m_pd3dCbvSrvDescriptorHeap) m_pd3dCbvSrvDescriptorHeap->Release();
}

void CScene::CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pDescriptorHeap->m_pd3dCbvSrvDescriptorHeap);

	m_pDescriptorHeap->m_d3dCbvCPUDescriptorStartHandle = m_pDescriptorHeap->m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pDescriptorHeap->m_d3dCbvGPUDescriptorStartHandle = m_pDescriptorHeap->m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_pDescriptorHeap->m_d3dSrvCPUDescriptorStartHandle.ptr = m_pDescriptorHeap->m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_pDescriptorHeap->m_d3dSrvGPUDescriptorStartHandle.ptr = m_pDescriptorHeap->m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);

	m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle = m_pDescriptorHeap->m_d3dCbvCPUDescriptorStartHandle;
	m_pDescriptorHeap->m_d3dCbvGPUDescriptorNextHandle = m_pDescriptorHeap->m_d3dCbvGPUDescriptorStartHandle;
	m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle = m_pDescriptorHeap->m_d3dSrvCPUDescriptorStartHandle;
	m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle = m_pDescriptorHeap->m_d3dSrvGPUDescriptorStartHandle;
}

void CScene::CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride)
{
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	for (int j = 0; j < nConstantBufferViews; j++)
	{
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle);
		m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		m_pDescriptorHeap->m_d3dCbvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	}
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateConstantBufferView(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dConstantBuffer, UINT nStride)
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	d3dCBVDesc.BufferLocation = pd3dConstantBuffer->GetGPUVirtualAddress();
	pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle);
	D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle = m_pDescriptorHeap->m_d3dCbvGPUDescriptorNextHandle;
	m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	m_pDescriptorHeap->m_d3dCbvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;

	return(d3dCbvGPUDescriptorHandle);
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateConstantBufferView(ID3D12Device* pd3dDevice, D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress, UINT nStride)
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress;
	pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle);
	D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle = m_pDescriptorHeap->m_d3dCbvGPUDescriptorNextHandle;
	m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	m_pDescriptorHeap->m_d3dCbvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;

	return(d3dCbvGPUDescriptorHandle);
}

void CScene::CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
{
	m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);
	m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);

	int nTextures = pTexture->GetTextures();
	for (int i = 0; i < nTextures; i++)
	{
		ID3D12Resource* pShaderResource = pTexture->GetResource(i);
		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(i);
		pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle);
		m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		pTexture->SetGpuDescriptorHandle(i, m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle);
		m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	}
	if (pTexture->GetTextures() > 0) {
		pTexture->SetRootParameterIndex(0, nRootParameterStartIndex); // Set root parameter for the first texture
	}
}

void CScene::CreateShaderResourceView(ID3D12Device* pd3dDevice, CTexture* pTexture, int nIndex, UINT nRootParameterStartIndex)
{
	ID3D12Resource* pShaderResource = pTexture->GetResource(nIndex);
	D3D12_GPU_DESCRIPTOR_HANDLE d3dGpuDescriptorHandle = pTexture->GetGpuDescriptorHandle(nIndex);
	if (pShaderResource && !d3dGpuDescriptorHandle.ptr)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(nIndex);
		pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle);
		m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;

		pTexture->SetGpuDescriptorHandle(nIndex, m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle);
		m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;

		pTexture->SetRootParameterIndex(nIndex, nRootParameterStartIndex + nIndex);
	}
}

void CScene::CreateShaderResourceView(ID3D12Device* pd3dDevice, CTexture* pTexture, int nIndex)
{
	ID3D12Resource* pShaderResource = pTexture->GetResource(nIndex);
	D3D12_GPU_DESCRIPTOR_HANDLE d3dGpuDescriptorHandle = pTexture->GetGpuDescriptorHandle(nIndex);
	if (pShaderResource && !d3dGpuDescriptorHandle.ptr)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(nIndex);
		pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle);
		m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;

		pTexture->SetGpuDescriptorHandle(nIndex, m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle);
		m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	}
}

CScene::CScene(CGameFramework* pGameFramework)
{
	m_pGameFramework = pGameFramework;
	m_xmf4x4WaterAnimation = XMFLOAT4X4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}

CScene::~CScene()
{
}

void CScene::BuildDefaultLightsAndMaterials()
{
	m_nLights = 4;
	m_pLights = new LIGHT[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

	m_pLights[0].m_bEnable = true;
	m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights[0].m_fRange = 1000.0f;
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.0f, 0.0f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	m_pLights[0].m_xmf3Position = XMFLOAT3(30.0f, 30.0f, 30.0f);
	m_pLights[0].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);
	m_pLights[1].m_bEnable = true;
	m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights[1].m_fRange = 500.0f;
	m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights[1].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[1].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[1].m_fFalloff = 8.0f;
	m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(20.0f));
	m_pLights[2].m_bEnable = true;
	m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_pLights[2].m_xmf4Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 0.0f);
	m_pLights[2].m_xmf3Direction = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_pLights[3].m_bEnable = true;
	m_pLights[3].m_nType = SPOT_LIGHT;
	m_pLights[3].m_fRange = 600.0f;
	m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.3f, 0.7f, 0.0f, 1.0f);
	m_pLights[3].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[3].m_xmf3Position = XMFLOAT3(50.0f, 30.0f, 30.0f);
	m_pLights[3].m_xmf3Direction = XMFLOAT3(0.0f, 1.0f, 1.0f);
	m_pLights[3].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[3].m_fFalloff = 8.0f;
	m_pLights[3].m_fPhi = (float)cos(XMConvertToRadians(90.0f));
	m_pLights[3].m_fTheta = (float)cos(XMConvertToRadians(30.0f));
}

void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	m_pDescriptorHeap = new CDescriptorHeap();
	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 505);

	BuildDefaultLightsAndMaterials();

	m_nShaders = 5;
	m_ppShaders = new CShader * [m_nShaders];

	CObjectsShader* pObjectsShader = new CObjectsShader();
	pObjectsShader->AddRef();
	pObjectsShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	pObjectsShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pTerrain);
	m_ppShaders[0] = pObjectsShader;

	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	XMFLOAT3 xmf3Scale(18.0f, 6.0f, 18.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.5f, 0.0f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/HeightMap.raw"), 257, 257, 257, 257, xmf3Scale, xmf4Color);

	CWaterShader* pWaterShader = new CWaterShader();
	pWaterShader->AddRef();
	pWaterShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_ppShaders[3] = pWaterShader;

	float waterWidth = 257 * xmf3Scale.x;
	float waterLength = 257 * xmf3Scale.z;
	m_pWater = new CWaterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pWaterShader, waterWidth, waterLength);
	m_pWater->SetPosition(920.0f, 545.0f, 1270.0f);

	CUIShader* pUIShader = new CUIShader();
	pUIShader->AddRef();
	pUIShader->CreateShader(pd3dDevice, pd3dCommandList);
	m_ppShaders[1] = pUIShader;

	m_pBackgroundObject = new CGameObject(1, 1);
	CScreenQuadMesh* pBackgroundMesh = new CScreenQuadMesh(pd3dDevice, pd3dCommandList);
	m_pBackgroundObject->SetMesh(0, pBackgroundMesh);

	CTexture* pBackgroundTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pBackgroundTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"UI/title.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceView(pd3dDevice, pBackgroundTexture, 0);
	pBackgroundTexture->SetRootParameterIndex(0, 0);

	CMaterial* pBackgroundMaterial = new CMaterial();
	pBackgroundMaterial->SetTexture(pBackgroundTexture);
	pBackgroundMaterial->SetShader(pUIShader);
	m_pBackgroundObject->SetMaterial(0, pBackgroundMaterial);

	m_pStartButtonObject = new CGameObject(1, 1);
	CUIRectMesh* pStartButtonMesh = new CUIRectMesh(pd3dDevice, pd3dCommandList, 0.25f, 0.35f, 0.2f, 0.2f);
	m_pStartButtonObject->SetMesh(0, pStartButtonMesh);

	m_pStartButtonDefaultTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	m_pStartButtonDefaultTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"UI/start.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceView(pd3dDevice, m_pStartButtonDefaultTexture, 0);
	m_pStartButtonDefaultTexture->SetRootParameterIndex(0, 0);

	CMaterial* pStartButtonMaterial = new CMaterial();
	pStartButtonMaterial->SetTexture(m_pStartButtonDefaultTexture);
	pStartButtonMaterial->SetShader(pUIShader);
	m_pStartButtonObject->SetMaterial(0, pStartButtonMaterial);

	m_pStartButtonHoverObject = new CGameObject(1, 1);
	CUIRectMesh* pStartButtonHoverMesh = new CUIRectMesh(pd3dDevice, pd3dCommandList, 0.25f - (0.2f * 0.05f), 0.35f - (0.2f * 0.05f), 0.2f * 1.1f, 0.2f * 1.1f);
	m_pStartButtonHoverObject->SetMesh(0, pStartButtonHoverMesh);
	CMaterial* pStartButtonHoverMaterial = new CMaterial();
	pStartButtonHoverMaterial->SetTexture(m_pStartButtonDefaultTexture);
	pStartButtonHoverMaterial->SetShader(pUIShader);
	m_pStartButtonHoverObject->SetMaterial(0, pStartButtonHoverMaterial);

	m_pExitButtonObject = new CGameObject(1, 1);
	CUIRectMesh* pExitButtonMesh = new CUIRectMesh(pd3dDevice, pd3dCommandList, 0.55f, 0.35f, 0.2f, 0.2f);
	m_pExitButtonObject->SetMesh(0, pExitButtonMesh);

	m_pExitButtonDefaultTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	m_pExitButtonDefaultTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"UI/exit.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceView(pd3dDevice, m_pExitButtonDefaultTexture, 0);
	m_pExitButtonDefaultTexture->SetRootParameterIndex(0, 0);

	CMaterial* pExitButtonMaterial = new CMaterial();
	pExitButtonMaterial->SetTexture(m_pExitButtonDefaultTexture);
	pExitButtonMaterial->SetShader(pUIShader);
	m_pExitButtonObject->SetMaterial(0, pExitButtonMaterial);

	m_pExitButtonHoverObject = new CGameObject(1, 1);
	CUIRectMesh* pExitButtonHoverMesh = new CUIRectMesh(pd3dDevice, pd3dCommandList, 0.55f - (0.2f * 0.05f), 0.35f - (0.2f * 0.05f), 0.2f * 1.1f, 0.2f * 1.1f);
	m_pExitButtonHoverObject->SetMesh(0, pExitButtonHoverMesh);
	CMaterial* pExitButtonHoverMaterial = new CMaterial();
	pExitButtonHoverMaterial->SetTexture(m_pExitButtonDefaultTexture);
	pExitButtonHoverMaterial->SetShader(pUIShader);
	m_pExitButtonHoverObject->SetMaterial(0, pExitButtonHoverMaterial);

	// Create UI for enemy count
	// 1) Load number texture (0-9 texture sheet)
	m_pNumberTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	m_pNumberTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"UI/num.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceView(pd3dDevice, m_pNumberTexture, 0);
	m_pNumberTexture->SetRootParameterIndex(0, 0);

	// 2) Create material for numbers
	m_pNumberMaterial = new CMaterial();
	m_pNumberMaterial->AddRef();
	m_pNumberMaterial->SetTexture(m_pNumberTexture);
	m_pNumberMaterial->SetShader(pUIShader); // Reuse the same CUIShader

	// 3) Create digit objects (e.g., 3 digits on the top-right of the screen)
	float digitWidth = 0.05f;
	float digitHeight = 0.08f;
	// Base position in normalized coordinates [0,1] (Top-Left is 0,0)
	float baseX = 0.83f; // Near top-right
	float baseY = 0.05f;

	for (int i = 0; i < m_nMaxEnemyDigits; i++)
	{
		m_pEnemyCountDigits[i] = new CGameObject(1, 1);

		// Position each digit next to the previous one
		float x = baseX + (digitWidth * i);
		float y = baseY;

		CUIRectMesh* pDigitMesh = new CUIRectMesh(pd3dDevice, pd3dCommandList, x, y, digitWidth, digitHeight);
		m_pEnemyCountDigits[i]->SetMesh(0, pDigitMesh);
		m_pEnemyCountDigits[i]->SetMaterial(0, m_pNumberMaterial);

		// Set initial UV to display '0'. The SetDigitUV function will be implemented next.
		SetDigitUV(m_pEnemyCountDigits[i], 0);
	}

	m_pBillboardShader = new CBillboardShader();
	m_pBillboardShader->AddRef();
	m_pBillboardShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_ppShaders[2] = m_pBillboardShader;

	m_pBillboardTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	m_pBillboardTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Model/Textures/Tree02.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceView(pd3dDevice, m_pBillboardTexture, 0, 12);

	int nBillboardsToGenerate = 100;
	m_nBillboardObjects = nBillboardsToGenerate;
	m_ppBillboardObjects = new CGameObject * [m_nBillboardObjects];

	XMFLOAT3 playerInitialPos = XMFLOAT3(920.0f, 745.0f, 1270.0f);
	float generationRadius = 600.0f;
	for (int i = 0; i < nBillboardsToGenerate; ++i)
	{
		float x = playerInitialPos.x + (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * generationRadius;
		float z = playerInitialPos.z + (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * generationRadius;
		float y = m_pTerrain->GetHeight(x, z) + 1.0f;

		XMFLOAT3 billboardPosition = XMFLOAT3(x, y, z);

		CBillboardObject* pBillboardObject = new CBillboardObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, billboardPosition, m_pBillboardShader, m_pBillboardTexture);
		m_ppBillboardObjects[i] = pBillboardObject;
	}

	// Bullet Resources
	m_pBulletMesh = new CCubeMesh(pd3dDevice, pd3dCommandList, 5.0f, 4.0f, 5.0f);
	m_pBulletMesh->AddRef();
	m_pBulletMaterial = new CMaterial();
	m_pBulletMaterial->AddRef();
	m_pBulletMaterial->m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f); //Yellow
	m_pBulletMaterial->SetShader(pObjectsShader); //Reuse the standard object shader

	// 1. Explosion Resources
	// 1.1. Create Explosion Shader
	m_pExplosionShader = new CExplosionShader();
	m_pExplosionShader->AddRef();
	m_pExplosionShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_ppShaders[4] = m_pExplosionShader;

	// 1.2. Load Texture and Create Material
	CTexture* pExplosionTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pExplosionTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Effect.dds", RESOURCE_TEXTURE2D, 0);
	CScene::CreateShaderResourceView(pd3dDevice, pExplosionTexture, 0, 3); // Assuming root parameter 3 is for albedo

	m_pExplosionMaterial = new CMaterial();
	m_pExplosionMaterial->AddRef();
	m_pExplosionMaterial->SetTexture(pExplosionTexture);
	m_pExplosionMaterial->SetShader(m_pExplosionShader);

	// 1.3. Create single-point mesh
	XMFLOAT3 xmf3Position(0.0f, 0.0f, 0.0f);
	m_pExplosionMesh = new CPointMesh(pd3dDevice, pd3dCommandList, 1, &xmf3Position);
	m_pExplosionMesh->AddRef();

	// 1.4. Create explosion object pool
	m_vExplosions.resize(50);
	for (int i = 0; i < 50; ++i)
	{
		m_vExplosions[i] = new CExplosionObject();
		m_vExplosions[i]->SetMesh(0, m_pExplosionMesh);
		m_vExplosions[i]->SetMaterial(0, m_pExplosionMaterial);
		m_vExplosions[i]->m_bRender = false; // Initially inactive
		m_vExplosions[i]->AddRef();
	}

	// Create Mirror
	CTexturedRectMesh* pMirrorMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 1000.0f, 1000.0f, 0.0f);
	m_pMirrorObject = new CGameObject(1, 1);
	m_pMirrorObject->SetMesh(0, pMirrorMesh);

	CMaterial* pMirrorMaterial = new CMaterial();
	pMirrorMaterial->m_xmf4AmbientColor = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.5f); // Green for debugging
	pMirrorMaterial->m_xmf4AlbedoColor = XMFLOAT4(0.8f, 0.8f, 0.9f, 0.3f); // Semi-transparent
	m_pMirrorObject->SetMaterial(0, pMirrorMaterial);
	// m_pMirrorObject->SetPosition(920.0f, 745.0f, 1300.0f); // Original position
	// m_pMirrorObject->Rotate(0.0f, 0.0f, 0.0f); // Original rotation

	// Temporarily move mirror in front of player for debugging
	XMFLOAT3 playerLook = XMFLOAT3(0.0f, 0.0f, 1.0f); // Assuming player initially looks along +Z

	XMFLOAT3 mirrorDebugPos = Vector3::Add(playerInitialPos, Vector3::ScalarProduct(playerLook, 50.0f));
	m_pMirrorObject->SetPosition(mirrorDebugPos.x, mirrorDebugPos.y, mirrorDebugPos.z);
	m_pMirrorObject->Rotate(0.0f, 180.0f, 0.0f); // Rotate to face the player
	m_pMirrorObject->UpdateTransform(NULL);

	m_pMirrorShader = new CMirrorShader(this, m_pMirrorObject);
	m_pMirrorShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	// Assign the correct shader to the mirror's material
	pMirrorMaterial->SetShader(m_pMirrorShader);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CScene::ReleaseObjects()
{
	if (m_pMirrorObject) m_pMirrorObject->Release();

	if (m_pStartButtonObject) m_pStartButtonObject->Release();
	if (m_pExitButtonObject) m_pExitButtonObject->Release();
	if (m_pStartButtonHoverObject) m_pStartButtonHoverObject->Release();
	if (m_pExitButtonHoverObject) m_pExitButtonHoverObject->Release();
	if (m_pBackgroundObject) m_pBackgroundObject->Release();

	if (m_pMainMenuObject) m_pMainMenuObject->Release();

	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();

	ReleaseShaderVariables();

	if (m_ppShaders)
	{
		for (int i = 0; i < m_nShaders; i++)
		{
			if (m_ppShaders[i])
			{
				m_ppShaders[i]->ReleaseShaderVariables();
				m_ppShaders[i]->ReleaseObjects();
				m_ppShaders[i]->Release();
			}
		}
		delete[] m_ppShaders;
	}

	if (m_pTerrain) delete m_pTerrain;
	if (m_pWater) delete m_pWater;
	if (m_pSkyBox) delete m_pSkyBox;

	for (auto pBullet : m_vBullets)
	{
		if (pBullet) pBullet->Release();
	}
	m_vBullets.clear();

	if (m_pBulletMesh) m_pBulletMesh->Release();
	if (m_pBulletMaterial) m_pBulletMaterial->Release();

	// Release explosion resources
	for (auto pExplosion : m_vExplosions)
	{
		if (pExplosion) pExplosion->Release();
	}
	m_vExplosions.clear();
	if (m_pExplosionMesh) m_pExplosionMesh->Release();
	if (m_pExplosionMaterial) m_pExplosionMaterial->Release();

	if (m_ppBillboardObjects)
	{
		for (int i = 0; i < m_nBillboardObjects; i++) if (m_ppBillboardObjects[i]) m_ppBillboardObjects[i]->Release();
		delete[] m_ppBillboardObjects;
	}

	if (m_ppGameObjects)
	{
		for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->Release();
		delete[] m_ppGameObjects;
	}

	if (m_pLights) delete[] m_pLights;

	if (m_pDescriptorHeap) delete m_pDescriptorHeap;
}

ID3D12RootSignature* CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[14];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 24; //t24: gtxtAlbedoTexture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 25; //t25: gtxtSpecularTexture
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 26; //t26: gtxtNormalTexture
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 27; //t27: gtxtMetallicTexture
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[4].NumDescriptors = 1;
	pd3dDescriptorRanges[4].BaseShaderRegister = 28; //t28: gtxtEmissionTexture
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 29; //t29: gtxtDetailAlbedoTexture
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[6].NumDescriptors = 1;
	pd3dDescriptorRanges[6].BaseShaderRegister = 30; //t30: gtxtDetailNormalTexture
	pd3dDescriptorRanges[6].RegisterSpace = 0;
	pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[7].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[7].NumDescriptors = 1;
	pd3dDescriptorRanges[7].BaseShaderRegister = 13; //t13: gtxtSkyBoxTexture
	pd3dDescriptorRanges[7].RegisterSpace = 0;
	pd3dDescriptorRanges[7].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[8].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[8].NumDescriptors = 3;
	pd3dDescriptorRanges[8].BaseShaderRegister = 14; //t14~16: gtxtTerrainTexture
	pd3dDescriptorRanges[8].RegisterSpace = 0;
	pd3dDescriptorRanges[8].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[9].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[9].NumDescriptors = 3;
	pd3dDescriptorRanges[9].BaseShaderRegister = 17; //t17: gtxtBillboards[]
	pd3dDescriptorRanges[9].RegisterSpace = 0;
	pd3dDescriptorRanges[9].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[10].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[10].NumDescriptors = 3;
	pd3dDescriptorRanges[10].BaseShaderRegister = 6;
	pd3dDescriptorRanges[10].RegisterSpace = 0;
	pd3dDescriptorRanges[10].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[11].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[11].NumDescriptors = 1;
	pd3dDescriptorRanges[11].BaseShaderRegister = 23;
	pd3dDescriptorRanges[11].RegisterSpace = 0;
	pd3dDescriptorRanges[11].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[16];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 33; // 16 for matrix + 16 for material + 1 for mask = 33
	pd3dRootParameters[1].Constants.ShaderRegister = 2; //GameObject
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[2].Descriptor.ShaderRegister = 4; //Lights
	pd3dRootParameters[2].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]);
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[1]);
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[5].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[2]);
	pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[6].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[6].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[3]);
	pd3dRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[7].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[7].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[4]);
	pd3dRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[8].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[8].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[5]);
	pd3dRootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[9].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[9].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[6]);
	pd3dRootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[10].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[10].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[7]);
	pd3dRootParameters[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[11].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[11].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[8]);
	pd3dRootParameters[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[12].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[12].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[9]);
	pd3dRootParameters[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[13].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[13].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[10]);
	pd3dRootParameters[13].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[14].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[14].Descriptor.ShaderRegister = 3;
	pd3dRootParameters[14].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[14].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[15].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[15].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[15].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[11]);
	pd3dRootParameters[15].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[2];

	pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].MipLODBias = 0;
	pd3dSamplerDescs[0].MaxAnisotropy = 1;
	pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[0].MinLOD = 0;
	pd3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[0].ShaderRegister = 0;
	pd3dSamplerDescs[0].RegisterSpace = 0;
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 1;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	HRESULT hr = D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);

	if (FAILED(hr)) {
		OutputDebugString(L"D3D12SerializeRootSignature FAILED!\n");
		if (pd3dErrorBlob) {
			OutputDebugStringA((char*)pd3dErrorBlob->GetBufferPointer());
			pd3dErrorBlob->Release();
		}
		return NULL; // Or handle more gracefully
	}

	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	// if (pd3dErrorBlob) pd3dErrorBlob->Release(); // Already released if it existed and failed

	return(pd3dGraphicsRootSignature);
}

void CScene::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256 
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void**)&m_pcbMappedLights);

	ncbElementBytes = ((sizeof(VS_CB_WATER_ANIMATION) + 255) & ~255); //256
	m_pd3dcbWaterAnimation = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbWaterAnimation->Map(0, NULL, (void**)&m_pcbMappedWaterAnimation);
}

void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHT) * m_nLights);
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));

	XMStoreFloat4x4(&m_pcbMappedWaterAnimation->m_xmf4x4TextureAnimation, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4WaterAnimation)));
	pd3dCommandList->SetGraphicsRootConstantBufferView(14, m_pd3dcbWaterAnimation->GetGPUVirtualAddress());
}

void CScene::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
		m_pd3dcbLights->Release();
	}

	if (m_pd3dcbWaterAnimation)
	{
		m_pd3dcbWaterAnimation->Unmap(0, NULL);
		m_pd3dcbWaterAnimation->Release();
	}

	if (m_pTerrain) m_pTerrain->ReleaseShaderVariables();
	if (m_pSkyBox) m_pSkyBox->ReleaseShaderVariables();
}

void CScene::ReleaseUploadBuffers()
{
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();
	if (m_pSkyBox) m_pSkyBox->ReleaseUploadBuffers();
	if (m_pWater) m_pWater->ReleaseUploadBuffers();

	for (int i = 0; i < m_nShaders; i++) m_ppShaders[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nGameObjects; i++) m_ppGameObjects[i]->ReleaseUploadBuffers();
}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pGameFramework->GetGameState() == GameState::MainMenu)
	{
		switch (nMessageID)
		{
		case WM_LBUTTONDOWN:
		{
			if (m_pHoveredObject == m_pStartButtonObject)
			{
				m_pGameFramework->SetGameState(GameState::InGame);
			}
			else if (m_pHoveredObject == m_pExitButtonObject)
			{
				PostQuitMessage(0);
			}
			return true;
		}
		case WM_MOUSEMOVE:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);

			float ndcX = (2.0f * x) / m_pGameFramework->GetCamera()->GetViewport().Width - 1.0f;
			float ndcY = 1.0f - (2.0f * y) / m_pGameFramework->GetCamera()->GetViewport().Height;

			XMFLOAT3 pickPos = XMFLOAT3(ndcX, ndcY, 0.0f);

			m_pHoveredObject = PickObjectByRayIntersection(pickPos, m_pGameFramework->GetCamera()->GetViewMatrix());
			return true;
		}
		default:
			break;
		}
	}
	return(false);
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		default:
			break;
		}
		break;
	default:
		break;
	}
	return(false);
}

bool CScene::ProcessInput(UCHAR* pKeysBuffer)
{
	return(false);
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->Animate(fTimeElapsed, NULL);
	for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->UpdateTransform(NULL);

	for (int i = 0; i < m_nShaders; i++) if (m_ppShaders[i]) m_ppShaders[i]->AnimateObjects(fTimeElapsed);

	if (m_pLights)
	{
		m_pLights[1].m_xmf3Position = m_pPlayer->GetPosition();
		m_pLights[1].m_xmf3Direction = m_pPlayer->GetLookVector();
	}

	// Collision Detection
	if (m_pPlayer && m_ppShaders && m_ppShaders[0])
	{
		CObjectsShader* pObjectsShader = dynamic_cast<CObjectsShader*>(m_ppShaders[0]);
		if (pObjectsShader)
		{
			int nObjects = pObjectsShader->GetNumberOfObjects();
			for (int i = 0; i < nObjects; ++i)
			{
				CGameObject* pOtherObject = pObjectsShader->GetObject(i);
				if (pOtherObject)
				{
					if (m_pPlayer->GetWorldAABB().Intersects(pOtherObject->GetWorldAABB()))
					{
						pOtherObject->m_bRender = false;
					}
				}
			}
		}
	}

	AnimateBullets(fTimeElapsed);
	AnimateExplosions(fTimeElapsed);
	CheckBulletCollisions();

	UpdateUIButtons(fTimeElapsed);

	m_xmf4x4WaterAnimation._32 += fTimeElapsed * 0.00125f;
}

void CScene::UpdateUIButtons(float fTimeElapsed)
{
	if (m_pGameFramework->GetGameState() == GameState::MainMenu)
	{
	}
}

CGameObject* CScene::PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View)
{
	auto IsPointInBox = [](const XMFLOAT3& point, const CGameObject* pObject) -> bool {
		CMesh* pMesh = pObject->GetMesh(0);
		if (!pMesh) return false;

		CUIRectMesh* pUIMesh = dynamic_cast<CUIRectMesh*>(pMesh);
		if (!pUIMesh) return false;

		float normalizedX = pUIMesh->GetNormalizedX();
		float normalizedY = pUIMesh->GetNormalizedY();
		float normalizedWidth = pUIMesh->GetNormalizedWidth();
		float normalizedHeight = pUIMesh->GetNormalizedHeight();

		float ndcLeft = (normalizedX * 2.0f) - 1.0f;
		float ndcRight = ndcLeft + (normalizedWidth * 2.0f);
		float ndcTop = 1.0f - (normalizedY * 2.0f);
		float ndcBottom = ndcTop - (normalizedHeight * 2.0f);

		const float epsilon = 0.0001f;
		if (point.x >= (ndcLeft - epsilon) && point.x <= (ndcRight + epsilon) &&
			point.y >= (ndcBottom - epsilon) && point.y <= (ndcTop + epsilon)) {
			return true;
		}
		return false;
		};

	if (m_pStartButtonObject && IsPointInBox(xmf3PickPosition, m_pStartButtonObject))
	{
		return m_pStartButtonObject;
	}

	if (m_pExitButtonObject && IsPointInBox(xmf3PickPosition, m_pExitButtonObject))
	{
		return m_pExitButtonObject;
	}

	return nullptr;
}

CBullet* CScene::SpawnBullet(const XMFLOAT3& xmf3Position, XMFLOAT3& xmf3Direction)
{
	CBullet* pBullet = new CBullet();
	pBullet->AddRef();

	pBullet->SetMesh(0, m_pBulletMesh);
	pBullet->SetMaterial(0, m_pBulletMaterial);

	pBullet->SetPosition(xmf3Position);
	pBullet->SetDirection(xmf3Direction);
	pBullet->SetSpeed(250.0f);
	pBullet->SetLifeTime(5.0f);

	pBullet->UpdateTransform(NULL);

	m_vBullets.push_back(pBullet);
	return(pBullet);
}

void CScene::AnimateBullets(float fTimeElapsed)
{
	for (auto it = m_vBullets.begin(); it != m_vBullets.end(); )
	{
		CBullet* pBullet = *it;
		if (pBullet && pBullet->IsAlive())
		{
			pBullet->Animate(fTimeElapsed, NULL);
			pBullet->UpdateTransform(NULL);
			++it;
		}
		else
		{
			if (pBullet) pBullet->Release();
			it = m_vBullets.erase(it);
		}
	}
}

void CScene::CheckBulletCollisions()
{
	CObjectsShader* pObjectsShader = dynamic_cast<CObjectsShader*>(m_ppShaders[0]);
	if (!pObjectsShader) return;

	int nEnemies = pObjectsShader->GetNumberOfObjects();

	for (auto* pBullet : m_vBullets)
	{
		if (!pBullet || !pBullet->IsAlive())
			continue;

		const BoundingBox& bulletBox = pBullet->GetWorldAABB();

		for (int i = 0; i < nEnemies; i++)
		{
			CGameObject* pEnemy = pObjectsShader->GetObject(i);
			if (!pEnemy || !pEnemy->m_bRender) continue;

			const BoundingBox& enemyBox = pEnemy->GetWorldAABB();

			if (bulletBox.Intersects(enemyBox))
			{
				SpawnExplosion(pEnemy->GetPosition());
				pEnemy->m_bRender = false;
				pBullet->Kill();
				break;
			}
		}
	}
}

int CScene::GetRemainingEnemyCount()
{
	// Assuming m_ppShaders[0] is the CObjectsShader that manages enemy objects.
	CObjectsShader* pObjectsShader = dynamic_cast<CObjectsShader*>(m_ppShaders[0]);
	if (!pObjectsShader)
	{
		return 0;
	}

	int nEnemies = pObjectsShader->GetNumberOfObjects();
	int nAlive = 0;

	for (int i = 0; i < nEnemies; i++)
	{
		CGameObject* pEnemy = pObjectsShader->GetObject(i);
		// Count only enemies that are set to be rendered.
		if (pEnemy && pEnemy->m_bRender)
		{
			nAlive++;
		}
	}
	return nAlive;
}

void CScene::SetDigitUV(CGameObject* pDigit, int digit)
{
	if (!pDigit) return;

	// 0~9 클램프
	if (digit < 0) digit = 0;
	if (digit > 9) digit = 9;

	int col = digit % 5;
	int row = digit / 5;   // 0: 0~4, 1: 5~9

	const float cellW = 1.0f / 5.0f;
	const float cellH = 1.0f / 2.0f;

	float u0 = col * cellW;
	float u1 = u0 + cellW;
	float v0 = row * cellH;
	float v1 = v0 + cellH;

	//  두 번째 줄(5~9)의 세로 위치를 약간 위로 당겨서 정렬
	//   v는 아래로 갈수록 값이 커지므로, 위로 올리려면 '감소'시켜야 함.
	if (row == 1)
	{
		const float vBias = 0.057f;   // -0.02f ~ -0.05f 사이에서 취향대로 튜닝
		v0 += vBias;
		v1 += vBias;
	}

	// 테두리 블리딩 방지 (필요하면 값 더 줄여도 됨)
	const float epsU = 0.002f;
	const float epsV = 0.002f;
	u0 += epsU; u1 -= epsU;
	v0 += epsV; v1 -= epsV;

	CUIRectMesh* pMesh = dynamic_cast<CUIRectMesh*>(pDigit->GetMesh(0));
	if (pMesh) pMesh->SetUVRect(u0, v0, u1, v1);
}

void CScene::UpdateEnemyCountUI()
{
	int nCount = GetRemainingEnemyCount();

	// Clamp count to the displayable range (0-999).
	if (nCount < 0) nCount = 0;
	if (nCount > 999) nCount = 999;

	// Decompose the count into hundreds, tens, and ones digits.
	int d2 = (nCount / 100) % 10; // Hundreds
	int d1 = (nCount / 10) % 10;  // Tens
	int d0 = nCount % 10;        // Ones

	// Update each digit's UV coordinates to display the correct number.
	if (m_pEnemyCountDigits[0]) SetDigitUV(m_pEnemyCountDigits[0], d2);
	if (m_pEnemyCountDigits[1]) SetDigitUV(m_pEnemyCountDigits[1], d1);
	if (m_pEnemyCountDigits[2]) SetDigitUV(m_pEnemyCountDigits[2], d0);
}

void CScene::RenderBullets(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	for (auto* pBullet : m_vBullets)
	{
		if (pBullet && pBullet->IsAlive())
		{
			pBullet->Render(pd3dCommandList, pCamera);
		}
	}
}

void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pGameFramework->GetGameState() == GameState::MainMenu)
	{
		// ... (MainMenu rendering code remains the same)
		if (m_pGameFramework->GetCamera()) m_pGameFramework->GetCamera()->SetViewportsAndScissorRects(pd3dCommandList);

		if (m_pBackgroundObject)
		{
			CUIShader* pUIShader = (CUIShader*)m_ppShaders[1];
			pd3dCommandList->SetGraphicsRootSignature(pUIShader->GetGraphicsRootSignature());
			ID3D12DescriptorHeap* ppHeaps[] = { m_pDescriptorHeap->m_pd3dCbvSrvDescriptorHeap };
			pd3dCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
			m_pBackgroundObject->Render(pd3dCommandList, NULL);
		}

		if (m_ppShaders[1])
		{
			CUIShader* pUIShader = (CUIShader*)m_ppShaders[1];
			pd3dCommandList->SetGraphicsRootSignature(pUIShader->GetGraphicsRootSignature());

			if (m_pHoveredObject == m_pStartButtonObject)
			{
				if (m_pStartButtonHoverObject) m_pStartButtonHoverObject->Render(pd3dCommandList, NULL);
			}
			else
			{
				if (m_pStartButtonObject) m_pStartButtonObject->Render(pd3dCommandList, NULL);
			}

			if (m_pHoveredObject == m_pExitButtonObject)
			{
				if (m_pExitButtonHoverObject) m_pExitButtonHoverObject->Render(pd3dCommandList, NULL);
			}
			else
			{
				if (m_pExitButtonObject) m_pExitButtonObject->Render(pd3dCommandList, NULL);
			}
		}
	}
	else // InGame state
	{
		// Common setup
		pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
		ID3D12DescriptorHeap* ppHeaps[] = { m_pDescriptorHeap->m_pd3dCbvSrvDescriptorHeap };
		pd3dCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		pCamera->SetViewportsAndScissorRects(pd3dCommandList);
		pCamera->UpdateShaderVariables(pd3dCommandList);
		UpdateShaderVariables(pd3dCommandList); // Updates lights
		D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(2, d3dcbLightsGpuVirtualAddress);

		// PASS 0: Render scene normally (excluding the mirror surface itself)
		if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, pCamera);
		if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera);
		if (m_ppShaders[0]) m_ppShaders[0]->Render(pd3dCommandList, pCamera);
		if (m_pPlayer) m_pPlayer->Render(pd3dCommandList, pCamera);
		for (int i = 0; i < m_nBillboardObjects; i++)
		{
			if (m_ppBillboardObjects[i]) m_ppBillboardObjects[i]->Render(pd3dCommandList, pCamera);
		}
		RenderBullets(pd3dCommandList, pCamera);
		if (m_pWater) m_pWater->Render(pd3dCommandList, pCamera);
		RenderExplosions(pd3dCommandList, pCamera);

		// --- MIRROR RENDERING PASSES ---
		if (m_pMirrorShader)
		{
			// PASS 1: Create Stencil Mask
			m_pMirrorShader->PreRender(pd3dCommandList, pCamera);

			// PASS 2 & 3: Setup reflection state and render reflected objects
			m_pMirrorShader->RenderReflectedObjects(pd3dCommandList, pCamera);

			// PASS 4 & 5: Restore state and render mirror surface
			m_pMirrorShader->PostRender(pd3dCommandList, pCamera);
		}

		// --- UI RENDERING ---
		UpdateEnemyCountUI();
		CUIShader* pUIShader = dynamic_cast<CUIShader*>(m_ppShaders[1]);
		if (pUIShader)
		{
			pd3dCommandList->SetGraphicsRootSignature(pUIShader->GetGraphicsRootSignature());
			pUIShader->Render(pd3dCommandList, nullptr, 0);
			if (m_pNumberTexture && m_pNumberTexture->m_pd3dSrvGpuDescriptorHandles[0].ptr != 0)
			{
				pd3dCommandList->SetGraphicsRootDescriptorTable(0, m_pNumberTexture->m_pd3dSrvGpuDescriptorHandles[0]);
			}
			for (int i = 0; i < m_nMaxEnemyDigits; i++)
			{
				if (m_pEnemyCountDigits[i])
				{
					CMesh* pMesh = m_pEnemyCountDigits[i]->GetMesh(0);
					if (pMesh) pMesh->Render(pd3dCommandList, 0);
				}
			}
		}
	}
}



void CScene::SpawnExplosion(const XMFLOAT3& position)

{

	CExplosionObject* pExplosion = m_vExplosions[m_nNextExplosion];

	pExplosion->Start(position);



	m_nNextExplosion = (m_nNextExplosion + 1) % m_vExplosions.size();

}



void CScene::AnimateExplosions(float fTimeElapsed)

{

	for (auto& pExplosion : m_vExplosions)

	{

		if (pExplosion->IsAlive())

		{

			pExplosion->Animate(fTimeElapsed, nullptr);

			pExplosion->UpdateTransform(nullptr);

		}

	}

}



void CScene::RenderExplosions(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)

{

	for (auto& pExplosion : m_vExplosions)

	{

		if (pExplosion && pExplosion->m_bRender)

		{

			pExplosion->Render(pd3dCommandList, pCamera);

		}

	}

}

