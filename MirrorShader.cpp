#include "stdafx.h"
#include "MirrorShader.h"
#include "Scene.h"
#include "GameFramework.h"
#include "Player.h"
#include "BillboardShader.h"
#include "Shader.h"

CMirrorShader::CMirrorShader(CScene* pScene, CGameObject* pMirrorObject)
{
	m_pScene = pScene;
	m_pMirrorObject = pMirrorObject;

	// 필요하면 뒷면 렌더용 거울 오브젝트 생성
	m_pMirrorBackObject = new CGameObject(1, 1);
	m_pMirrorBackObject->SetMesh(0, pMirrorObject->GetMesh(0));
	CMaterial* pMirrorMaterial = new CMaterial();
	pMirrorMaterial->m_xmf4AmbientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.f); // Green for debugging
	pMirrorMaterial->SetShader(this);
	//pMirrorMaterial->m_xmf4AlbedoColor = XMFLOAT4(0.8f, 0.8f, 0.9f, 0.3f); // Semi-transparent
	m_pMirrorBackObject->SetMaterial(0, pMirrorMaterial);

	m_pOriginalLights = nullptr;
	m_nOriginalLights = 0;
}

CMirrorShader::~CMirrorShader()
{
	if (m_pMirrorBackObject)
	{
		m_pMirrorBackObject->Release();
		m_pMirrorBackObject = NULL;
	}

	if (m_pOriginalLights)
	{
		delete[] m_pOriginalLights;
		m_pOriginalLights = nullptr;
	}
}

D3D12_INPUT_LAYOUT_DESC CMirrorShader::CreateInputLayout()
{
	UINT nInputElementDescs = 5;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CMirrorShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSStandard", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CMirrorShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSMirror", "ps_5_1", &m_pd3dPixelShaderBlob));
}

void CMirrorShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	m_nPipelineStates = 4;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];
	::ZeroMemory(m_ppd3dPipelineStates, sizeof(ID3D12PipelineState*) * m_nPipelineStates);

	CreatePipelineStates(pd3dDevice, pd3dGraphicsRootSignature);
}

void CMirrorShader::CreatePipelineStates(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	// 공통 PSO 베이스 설정
	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	d3dPipelineStateDesc.VS = CreateVertexShader();
	d3dPipelineStateDesc.PS = CreatePixelShader();
	d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	d3dPipelineStateDesc.NumRenderTargets = 1;
	d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	// ===== PSO[0] : 거울을 스텐실 버퍼에만 렌더 (Stencil = 1) =====
	D3D12_DEPTH_STENCIL_DESC ds0 = CreateDepthStencilState();
	ds0.DepthEnable = TRUE;
	ds0.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;	// 깊이 버퍼는 건드리지 않음
	ds0.StencilEnable = TRUE;
	ds0.StencilReadMask = 0xff;
	ds0.StencilWriteMask = 0xff;
	ds0.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	ds0.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	ds0.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;	// 통과 시 스텐실 = Ref
	ds0.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	ds0.BackFace = ds0.FrontFace;

	D3D12_BLEND_DESC bs0 = CreateBlendState();
	bs0.RenderTarget[0].BlendEnable = FALSE;
	bs0.RenderTarget[0].RenderTargetWriteMask = 0;		// 컬러 버퍼에는 쓰지 않음 (사용자 제안)

	d3dPipelineStateDesc.DepthStencilState = ds0;
	d3dPipelineStateDesc.BlendState = bs0;
	d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
	pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc,
		IID_PPV_ARGS(&m_ppd3dPipelineStates[0]));

	// ===== PSO[1] : (옵션) 반사된 객체용 PSO 슬롯 – 여기서는 사용 X =====
	//	실제 반사된 객체는 각자 셰이더에서 반사용 PSO를 사용하니,
	//	여기서는 DS/RS만 참고용으로 만들어두고 쓰지는 않는다.
	// 사용자 제안에 따라 PSO[1] 생성 코드 제거
	m_ppd3dPipelineStates[1] = nullptr; // 명시적으로 nullptr 설정

	// ===== PSO[2] : 거울 표면을 알파 블렌딩으로 렌더 (Stencil == 1) =====
	D3D12_DEPTH_STENCIL_DESC ds2 = CreateDepthStencilState();
	ds2.DepthEnable = FALSE;									// 거울 자체는 깊이 검사를 안 함 (사용자 제안)
	ds2.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;			// (사용자 제안)
	ds2.StencilEnable = TRUE;
	ds2.StencilReadMask = 0xff;
	ds2.StencilWriteMask = 0x00;
	ds2.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	ds2.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	ds2.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	ds2.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;	// 스텐실 == 1 부분만
	ds2.BackFace = ds2.FrontFace;

	D3D12_BLEND_DESC bs2 = CreateBlendState();
	bs2.RenderTarget[0].BlendEnable = TRUE;
	bs2.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	bs2.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	bs2.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	bs2.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	bs2.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	bs2.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	bs2.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	d3dPipelineStateDesc.DepthStencilState = ds2;
	d3dPipelineStateDesc.BlendState = bs2;
	d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
	pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc,
		IID_PPV_ARGS(&m_ppd3dPipelineStates[2]));

	// ===== PSO[3] : 거울 뒷면을 깊이 버퍼에만 렌더 (선택) =====
	D3D12_DEPTH_STENCIL_DESC ds3 = CreateDepthStencilState();
	ds3.DepthEnable = TRUE;
	ds3.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	ds3.StencilEnable = FALSE;

	D3D12_BLEND_DESC bs3 = CreateBlendState();
	bs3.RenderTarget[0].BlendEnable = FALSE;
	bs3.RenderTarget[0].RenderTargetWriteMask = 0;	// 컬러 버퍼 쓰지 않음

	D3D12_RASTERIZER_DESC rs3 = CreateRasterizerState();
	rs3.CullMode = D3D12_CULL_MODE_FRONT;	// 앞면 컬링 → 뒷면만 렌더

	d3dPipelineStateDesc.DepthStencilState = ds3;
	d3dPipelineStateDesc.BlendState = bs3;
	d3dPipelineStateDesc.RasterizerState = rs3;
	pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc,
		IID_PPV_ARGS(&m_ppd3dPipelineStates[3]));

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs)
		delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

// ======================================================================
// PASS 1 : 스텐실 마스크 생성 (Scene::Render() 초반에서 호출)
// ======================================================================
void CMirrorShader::PreRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (!m_pMirrorObject) return;

	// 1) 스텐실 버퍼만 0으로 초기화 (깊이는 그대로)
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_pScene->m_pGameFramework->GetDsvCPUDescriptorHandle();
	pd3dCommandList->ClearDepthStencilView(dsvHandle,
		D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// 2) 거울 뒷면을 깊이 버퍼에만 렌더 (옵션)
	//if (m_pMirrorBackObject && m_ppd3dPipelineStates[3])
	//{
		//pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[3]);
		//m_pMirrorBackObject->Render(pd3dCommandList, pCamera);
	//}

	// 3) 거울 앞면을 스텐실 버퍼에 렌더 (Stencil = 1)
	pd3dCommandList->OMSetStencilRef(1);
	pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[0]); // 사용자 제안: PSO[0] 명시적 설정
	m_pMirrorObject->Render(pd3dCommandList, pCamera, 0); // Use MirrorShader's PSO[0]
}

// ======================================================================
// PASS 2~3 : 반사된 장면 렌더 (Scene 의 "보통 물체 렌더" 직후 호출)
// ======================================================================
void CMirrorShader::RenderReflectedObjects(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (!m_pMirrorObject) return;
	// 1) 거울 평면 계산
	//	거울 메쉬의 로컬 노멀을 (0, 0, -1) 로 가정 (정면이 -Z 이라고 가정)
	//	필요하면 이 부분은 실제 거울 메쉬 방향에 맞게 조정
	XMMATRIX xmmtxWorld = XMLoadFloat4x4(&m_pMirrorObject->m_xmf4x4World);

	// 거울 위의 한 점 (월드 좌표계)
	XMVECTOR xmvPoint = XMVectorSet(m_pMirrorObject->m_xmf4x4World._41,
		m_pMirrorObject->m_xmf4x4World._42,
		m_pMirrorObject->m_xmf4x4World._43, 1.0f);

	// 로컬 노멀 (0,0,-1)을 월드 노멀로 변환
	XMVECTOR xmvNormalLocal = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
	XMVECTOR xmvNormalWorld = XMVector3TransformNormal(xmvNormalLocal, xmmtxWorld);
	xmvNormalWorld = XMVector3Normalize(xmvNormalWorld);

	// ★ 노멀이 카메라 쪽을 향하도록 한 번 정리 (앞/뒤 구분에 쓰기 위함)
	XMFLOAT3 xmf3CamPos = pCamera->GetPosition();        // GetPosition 없으면 m_xmf3Position 쓰면 됨
	XMVECTOR xmvCamPos = XMLoadFloat3(&xmf3CamPos);
	XMVECTOR xmvToCamera = XMVectorSubtract(xmvCamPos, xmvPoint);

	if (XMVectorGetX(XMVector3Dot(xmvNormalWorld, xmvToCamera)) < 0.0f)
	{
		// 카메라와 반대 방향을 보고 있으면 뒤집어 준다
		xmvNormalWorld = XMVectorNegate(xmvNormalWorld);
	}

	// 이제 이 평면에서 dot>0 이면 "거울 앞쪽(카메라 쪽)", dot<0 이면 "거울 뒤쪽"
	XMVECTOR xmvMirrorPlane = XMPlaneFromPointNormal(xmvPoint, xmvNormalWorld);

	// 반사용 행렬 (월드 행렬에 곱해서 반사)
	XMMATRIX xmmtxReflect = XMMatrixReflect(xmvMirrorPlane);

	// 2) 카메라는 그대로 복제만 (뷰 행렬은 건드리지 않고, 월드만 반사해서 사용)
	CCamera reflectedCamera = *pCamera;


	// 3) 조명도 반사
	ReflectLights(pd3dCommandList, xmvMirrorPlane);
	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pScene->m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(2, d3dcbLightsGpuVirtualAddress);
	// 카메라를 따라다니는 20x20x20크기의 큐브가 스카이박슨데, 맨 처음에 그리기만 하고, 깊이 테스트는 안 해서 꽉 차는 것처럼 보이는 건데, 
	// 4) 스텐실 값 1인 영역에만 반사된 장면 렌더
	//	실제 객체 렌더링은 각 셰이더의 "반사용 PSO"가 수행한다고 가정
	pd3dCommandList->OMSetStencilRef(1);
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_pScene->m_pGameFramework->GetDsvCPUDescriptorHandle();
	pd3dCommandList->ClearDepthStencilView(dsvHandle,
		D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	// (1) SkyBox : geometry 를 반사시키지 않고, 반사된 카메라로만 렌더
	if (m_pScene->m_pSkyBox)
	{
		m_pScene->m_pSkyBox->Render(pd3dCommandList, &reflectedCamera, xmmtxReflect,1); // 사용자 제안: xmmtxReflect 인자 추가
	}
	
	// (2) Terrain : 마찬가지로 월드는 그대로, 카메라만 반사
	if (m_pScene->m_pTerrain)
	{
		m_pScene->m_pTerrain->Render(pd3dCommandList, &reflectedCamera, xmmtxReflect); // 사용자 제안: xmmtxReflect 인자 추가
	}
	if(m_pScene->m_pWater)
		m_pScene->m_pWater->Render(pd3dCommandList, &reflectedCamera,xmmtxReflect,1);
	// (3) 일반 오브젝트들 (헬기, 건물 등)
	if (m_pScene->m_ppShaders[0])
	{
		CObjectsShader* pObjectsShader = (CObjectsShader*)m_pScene->m_ppShaders[0];
		pObjectsShader->RenderReflected(pd3dCommandList, &reflectedCamera, xmmtxReflect, xmvMirrorPlane);
	}

	// (4) 플레이어
	if (m_pScene->m_pPlayer)
	{
		m_pScene->m_pPlayer->Render(pd3dCommandList, &reflectedCamera, xmmtxReflect);
	}
	for (int i = 0; i < m_pScene->m_nBillboardObjects; i++)
	{
		if (m_pScene->m_ppBillboardObjects[i])
		{
			m_pScene->m_ppBillboardObjects[i]->Render(pd3dCommandList, &reflectedCamera, xmmtxReflect, 1);
		}
	}
	if (m_pScene) {
		m_pScene->RenderBulletssReflect(pd3dCommandList, &reflectedCamera, xmmtxReflect);
	}
	if (m_pScene) {
		m_pScene->RenderExplosionsReflect(pd3dCommandList, &reflectedCamera, xmmtxReflect);
	}
	// reflectedCamera 는 지역 변수이므로 여기서 자동으로 소멸됨
}

// ======================================================================
// PASS 4~5 : 조명/카메라 복원 + 거울 표면 렌더
// ======================================================================
void CMirrorShader::PostRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	// 1) 조명 원본 복원
	RestoreLights(pd3dCommandList);
	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pScene->m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(2, d3dcbLightsGpuVirtualAddress);

	// 2) 카메라는 원본 포인터(pCamera) 자체를 건드리지 않았으므로
	//	여기서 특별히 Restore 할 필요는 없음.
	pCamera->UpdateShaderVariables(pd3dCommandList);

	// 3) 거울 표면 텍스처를 알파 블렌딩으로 렌더
	if (m_pMirrorObject)
	{
		pd3dCommandList->OMSetStencilRef(1);
		pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[2]); // 사용자 제안: PSO[2] 명시적 설정
		m_pMirrorObject->Render(pd3dCommandList, pCamera, 2); // Use MirrorShader's PSO[2]
	}
}

// ======================================================================
// 조명 반사 / 복원
// ======================================================================
void CMirrorShader::ReflectLights(ID3D12GraphicsCommandList* pd3dCommandList, const XMVECTOR& xmvMirrorPlane)
{
	if (!m_pScene->m_pLights || m_pScene->m_nLights <= 0) return;

	if (!m_pOriginalLights)
	{
		m_nOriginalLights = m_pScene->m_nLights;
		m_pOriginalLights = new LIGHT[m_nOriginalLights];
	}

	// 원본 백업
	memcpy(m_pOriginalLights, m_pScene->m_pLights, sizeof(LIGHT) * m_nOriginalLights);

	XMMATRIX xmmtxReflect = XMMatrixReflect(xmvMirrorPlane);

	for (int i = 0; i < m_pScene->m_nLights; i++)
	{
		LIGHT& light = m_pScene->m_pLights[i];

		// 방향성 광원
		if (light.m_nType == DIRECTIONAL_LIGHT)
		{
			XMVECTOR dir = XMLoadFloat3(&light.m_xmf3Direction);
			dir = XMVector3TransformNormal(dir, xmmtxReflect);
			XMStoreFloat3(&light.m_xmf3Direction, dir);
		}
		// 점광 / 스포트라이트
		else
		{
			XMVECTOR pos = XMLoadFloat3(&light.m_xmf3Position);
			pos = XMVector3TransformCoord(pos, xmmtxReflect);
			XMStoreFloat3(&light.m_xmf3Position, pos);

			if (light.m_nType == SPOT_LIGHT)
			{
				XMVECTOR dir = XMLoadFloat3(&light.m_xmf3Direction);
				dir = XMVector3TransformNormal(dir, xmmtxReflect);
				XMStoreFloat3(&light.m_xmf3Direction, dir);
			}
		}
	}

	// 상수 버퍼에 업데이트
	memcpy(m_pScene->m_pcbMappedLights->m_pLights, m_pScene->m_pLights,
		sizeof(LIGHT) * m_pScene->m_nLights);
}

void CMirrorShader::RestoreLights(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (!m_pOriginalLights) return;

	memcpy(m_pScene->m_pLights, m_pOriginalLights, sizeof(LIGHT) * m_nOriginalLights);
	memcpy(m_pScene->m_pcbMappedLights->m_pLights, m_pScene->m_pLights,
		sizeof(LIGHT) * m_pScene->m_nLights);
}