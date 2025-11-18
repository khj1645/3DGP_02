//-----------------------------------------------------------------------------
// File: BillboardShader.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "BillboardShader.h"

CBillboardShader::CBillboardShader()
{
    m_nPipelineStates = 2; // 0 for Normal, 1 for Reflection
    m_ppd3dPipelineStates = new ID3D12PipelineState*[m_nPipelineStates];
    ::ZeroMemory(m_ppd3dPipelineStates, sizeof(ID3D12PipelineState*) * m_nPipelineStates);
}

CBillboardShader::~CBillboardShader()
{
    if (m_pd3dGeometryShaderBlob) m_pd3dGeometryShaderBlob->Release();
}

D3D12_INPUT_LAYOUT_DESC CBillboardShader::CreateInputLayout()
{
    // VS_BILLBOARD_INPUT struct: float3 position : POSITION;
    UINT nInputElementDescs = 1;
    D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

    pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

    D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
    d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
    d3dInputLayoutDesc.NumElements = nInputElementDescs;

    return d3dInputLayoutDesc;
}

D3D12_RASTERIZER_DESC CBillboardShader::CreateRasterizerState()
{
    D3D12_RASTERIZER_DESC d3dRasterizerDesc = CShader::CreateRasterizerState();
    d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_NONE; // 빌보드는 양면을 모두 그려야 하므로 컬링 비활성화
    return d3dRasterizerDesc;
}

D3D12_BLEND_DESC CBillboardShader::CreateBlendState()
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

D3D12_DEPTH_STENCIL_DESC CBillboardShader::CreateDepthStencilState()
{
    D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc = CShader::CreateDepthStencilState();
    d3dDepthStencilDesc.DepthEnable = TRUE;
    d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // 깊이 쓰기 비활성화 (투명 객체)
    d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    return d3dDepthStencilDesc;
}

D3D12_SHADER_BYTECODE CBillboardShader::CreateVertexShader()
{
    return CompileShaderFromFile(L"Shaders.hlsl", "VSBillboard", "vs_5_1", &m_pd3dVertexShaderBlob);
}

D3D12_SHADER_BYTECODE CBillboardShader::CreateGeometryShader()
{
    return CompileShaderFromFile(L"Shaders.hlsl", "GSBillboard", "gs_5_1", &m_pd3dGeometryShaderBlob);
}

D3D12_SHADER_BYTECODE CBillboardShader::CreatePixelShader()
{
    return CompileShaderFromFile(L"Shaders.hlsl", "PSBillboard", "ps_5_1", &m_pd3dPixelShaderBlob);
}

void CBillboardShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature)
{
    m_d3dPipelineStateDesc.InputLayout = CreateInputLayout();
    m_d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
    m_d3dPipelineStateDesc.VS = CreateVertexShader();
    m_d3dPipelineStateDesc.GS = CreateGeometryShader(); // Geometry Shader 설정
    m_d3dPipelineStateDesc.PS = CreatePixelShader();
    m_d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
    m_d3dPipelineStateDesc.BlendState = CreateBlendState();
    m_d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
    m_d3dPipelineStateDesc.SampleMask = UINT_MAX;
    m_d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT; // 포인트 리스트로 설정
    m_d3dPipelineStateDesc.NumRenderTargets = 1;
    m_d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    m_d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    m_d3dPipelineStateDesc.SampleDesc.Count = 1;
    m_d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    // PSO[0]: Normal Billboard
    HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&m_d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_ppd3dPipelineStates[0]);

    // PSO[1]: Reflected Billboard
    m_d3dPipelineStateDesc.DepthStencilState = CreateReflectionStencilState();
    pd3dDevice->CreateGraphicsPipelineState(&m_d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_ppd3dPipelineStates[1]);
    
    // InputLayout에 할당된 메모리 해제
    if (m_d3dPipelineStateDesc.InputLayout.pInputElementDescs)
    {
        delete[] m_d3dPipelineStateDesc.InputLayout.pInputElementDescs;
        m_d3dPipelineStateDesc.InputLayout.pInputElementDescs = NULL;
    }
}
