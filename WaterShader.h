#pragma once

#include "Shader.h"

class CWaterShader : public CShader
{
public:
    CWaterShader();
    virtual ~CWaterShader();

    virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
    virtual D3D12_BLEND_DESC CreateBlendState();
    virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
    virtual D3D12_RASTERIZER_DESC CreateRasterizerState();

    virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
    virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;

    virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
};
