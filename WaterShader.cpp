#include "stdafx.h"
#include "WaterShader.h"
#include <wrl.h>
#include <vector>

using Microsoft::WRL::ComPtr;

CWaterShader::CWaterShader()
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState*[m_nPipelineStates];
}

CWaterShader::~CWaterShader()
{
	if (m_pd3dVertexShaderBlob) m_pd3dVertexShaderBlob->Release();
	if (m_pd3dPixelShaderBlob) m_pd3dPixelShaderBlob->Release();
}

D3D12_INPUT_LAYOUT_DESC CWaterShader::CreateInputLayout()
{
    UINT nInputElementDescs = 2;
    D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

    pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
                                 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    // ⬇️ UV는 slot=1, offset=0 로!
    pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,     1, 0,
                                 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

    D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
    d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
    d3dInputLayoutDesc.NumElements = nInputElementDescs;

    return d3dInputLayoutDesc;
}

D3D12_BLEND_DESC CWaterShader::CreateBlendState()
{
    D3D12_BLEND_DESC d3dBlendDesc = CShader::CreateBlendState();
    d3dBlendDesc.AlphaToCoverageEnable = FALSE;
    d3dBlendDesc.IndependentBlendEnable = FALSE;
    d3dBlendDesc.RenderTarget[0].BlendEnable = TRUE;
    d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
    d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
    d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    return d3dBlendDesc;
}

D3D12_DEPTH_STENCIL_DESC CWaterShader::CreateDepthStencilState()
{
    D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc = CShader::CreateDepthStencilState();
    d3dDepthStencilDesc.DepthEnable = TRUE;
    d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    return d3dDepthStencilDesc;
}

D3D12_RASTERIZER_DESC CWaterShader::CreateRasterizerState()
{
    D3D12_RASTERIZER_DESC d3dRasterizerDesc = CShader::CreateRasterizerState();
    d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
    d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    return d3dRasterizerDesc;
}

D3D12_SHADER_BYTECODE CWaterShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSTerrainWater", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CWaterShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSTerrainWater", "ps_5_1", &m_pd3dPixelShaderBlob));
}

void CWaterShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
    ComPtr<ID3D12InfoQueue> info;
    if (SUCCEEDED(pd3dDevice->QueryInterface(IID_PPV_ARGS(&info))))
    {
        info->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        info->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
    }

	::ZeroMemory(&m_d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

    m_d3dPipelineStateDesc.InputLayout = CreateInputLayout();
    m_d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
    m_d3dPipelineStateDesc.VS = CreateVertexShader();
    m_d3dPipelineStateDesc.PS = CreatePixelShader();
    m_d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
    m_d3dPipelineStateDesc.BlendState = CreateBlendState();
    m_d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
    m_d3dPipelineStateDesc.SampleMask = UINT_MAX;
    m_d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    m_d3dPipelineStateDesc.NumRenderTargets = 1;
    m_d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    m_d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    m_d3dPipelineStateDesc.SampleDesc.Count = 1;
    m_d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    HRESULT hr_pso = pd3dDevice->CreateGraphicsPipelineState(&m_d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_ppd3dPipelineStates[0]);
    
    if (FAILED(hr_pso) || !m_ppd3dPipelineStates[0]) {
        OutputDebugStringA("\n--- ERROR: Failed to create Water PSO ---\n");
        ComPtr<ID3D12InfoQueue> pInfoQueue;
        if (SUCCEEDED(pd3dDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue))))
        {
            const UINT64 nMessages = pInfoQueue->GetNumStoredMessages();
            for (UINT64 i = 0; i < nMessages; ++i)
            {
                SIZE_T messageLength = 0;
                pInfoQueue->GetMessage(i, nullptr, &messageLength);
                std::vector<char> buffer(messageLength);
                D3D12_MESSAGE* pMessage = reinterpret_cast<D3D12_MESSAGE*>(buffer.data());
                if (SUCCEEDED(pInfoQueue->GetMessage(i, pMessage, &messageLength)))
                {
                    OutputDebugStringA("\n--- D3D12 InfoQueue Message ---\n");
                    OutputDebugStringA(pMessage->pDescription);
                    OutputDebugStringA("\n---------------------------------\n");
                }
            }
        }
    }

    TCHAR buffer[256];
    _stprintf_s(buffer, L"CWaterShader::CreateShader: m_ppd3dPipelineStates[0] = %p (after creation attempt)\n", m_ppd3dPipelineStates[0]);
    OutputDebugString(buffer);

    if (m_d3dPipelineStateDesc.InputLayout.pInputElementDescs) {
        delete[] m_d3dPipelineStateDesc.InputLayout.pInputElementDescs;
		m_d3dPipelineStateDesc.InputLayout.pInputElementDescs = NULL;
    }
}

