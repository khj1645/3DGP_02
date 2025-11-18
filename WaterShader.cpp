#include "stdafx.h"
#include "WaterShader.h"
#include <wrl.h>
#include <vector>

using Microsoft::WRL::ComPtr;

CWaterShader::CWaterShader()
{
	m_nPipelineStates = 2; // Standard and Reflection
	m_ppd3dPipelineStates = new ID3D12PipelineState*[m_nPipelineStates];
	::ZeroMemory(m_ppd3dPipelineStates, sizeof(ID3D12PipelineState*) * m_nPipelineStates);
}

CWaterShader::~CWaterShader()
{
	if (m_pd3dVertexShaderBlob) m_pd3dVertexShaderBlob->Release();
	if (m_pd3dPixelShaderBlob) m_pd3dPixelShaderBlob->Release();

	// Pipeline states are released by the base CShader destructor
}

D3D12_INPUT_LAYOUT_DESC CWaterShader::CreateInputLayout()
{
    UINT nInputElementDescs = 2;
    D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

    pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
                                 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
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
    D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

    d3dPipelineStateDesc.InputLayout = CreateInputLayout();
    d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
    d3dPipelineStateDesc.VS = CreateVertexShader();
    d3dPipelineStateDesc.PS = CreatePixelShader();
    d3dPipelineStateDesc.SampleMask = UINT_MAX;
    d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    d3dPipelineStateDesc.NumRenderTargets = 1;
    d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    d3dPipelineStateDesc.SampleDesc.Count = 1;
    d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    // PSO[0]: Standard Water Rendering
    d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
    d3dPipelineStateDesc.BlendState = CreateBlendState();
    d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
    pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_ppd3dPipelineStates[0]);

    // PSO[1]: Reflected Water Rendering
    D3D12_RASTERIZER_DESC d3dRasterizerDesc = CreateRasterizerState();
    d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_FRONT; // Cull front faces for reflection
    d3dPipelineStateDesc.RasterizerState = d3dRasterizerDesc;

    d3dRasterizerDesc.FrontCounterClockwise = TRUE;
    d3dPipelineStateDesc.RasterizerState = d3dRasterizerDesc;
    d3dPipelineStateDesc.DepthStencilState = CreateReflectionStencilState();

    pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_ppd3dPipelineStates[1]);

    if (d3dPipelineStateDesc.InputLayout.pInputElementDescs) {
        delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
		d3dPipelineStateDesc.InputLayout.pInputElementDescs = NULL;
    }
}

