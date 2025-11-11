struct MATERIAL
{
	float4					m_cAmbient;
	float4					m_cDiffuse;
	float4					m_cSpecular; //a = power
	float4					m_cEmissive;
};

cbuffer cbCameraInfo : register(b1)
{
	matrix					gmtxView : packoffset(c0);
	matrix					gmtxProjection : packoffset(c4);
	matrix					gmtxInverseView : packoffset(c8);
	float3					gvCameraPosition : packoffset(c12);
};

cbuffer cbGameObjectInfo : register(b2)
{
	matrix					gmtxGameObject : packoffset(c0);
	MATERIAL				gMaterial : packoffset(c4);
	uint					gnTexturesMask : packoffset(c8);
};

#include "Light.hlsl"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//#define _WITH_VERTEX_LIGHTING

#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

Texture2D gtxtAlbedoTexture : register(t6);
Texture2D gtxtSpecularTexture : register(t7);
Texture2D gtxtNormalTexture : register(t8);
Texture2D gtxtMetallicTexture : register(t9);
Texture2D gtxtEmissionTexture : register(t10);
Texture2D gtxtDetailAlbedoTexture : register(t11);
Texture2D gtxtDetailNormalTexture : register(t12);

SamplerState gssWrap : register(s0);

struct VS_STANDARD_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

struct VS_STANDARD_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
	float2 uv : TEXCOORD;
};

VS_STANDARD_OUTPUT VSStandard(VS_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.tangentW = (float3)mul(float4(input.tangent, 1.0f), gmtxGameObject);
	output.bitangentW = (float3)mul(float4(input.bitangent, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSStandard(VS_STANDARD_OUTPUT input) : SV_TARGET
{
	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) cEmissionColor = gtxtEmissionTexture.Sample(gssWrap, input.uv);

	float4 cIllumination = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float4 cColor = cAlbedoColor + cSpecularColor + cEmissionColor;
	if (gnTexturesMask & MATERIAL_NORMAL_MAP)
	{
		float3 normalW = input.normalW;
		float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
		float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1]  [-1, 1]
		normalW = normalize(mul(vNormal, TBN));
		cIllumination = Lighting(input.positionW, normalW);
		cColor = lerp(cColor, cIllumination, 0.5f);
	}

	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_SKYBOX_CUBEMAP_INPUT
{
	float3 position : POSITION;
};

struct VS_SKYBOX_CUBEMAP_OUTPUT
{
	float3	positionL : POSITION;
	float4	position : SV_POSITION;
};

VS_SKYBOX_CUBEMAP_OUTPUT VSSkyBox(VS_SKYBOX_CUBEMAP_INPUT input)
{
	VS_SKYBOX_CUBEMAP_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.positionL = input.position;

	return(output);
}

TextureCube gtxtSkyCubeTexture : register(t13);
SamplerState gssClamp : register(s1);

float4 PSSkyBox(VS_SKYBOX_CUBEMAP_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtSkyCubeTexture.Sample(gssClamp, input.positionL);

	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_SPRITE_TEXTURED_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
};

struct VS_SPRITE_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_SPRITE_TEXTURED_OUTPUT VSTextured(VS_SPRITE_TEXTURED_INPUT input)
{
	VS_SPRITE_TEXTURED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

/*
float4 PSTextured(VS_SPRITE_TEXTURED_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
	float4 cColor;
	if (nPrimitiveID < 2)
		cColor = gtxtTextures[0].Sample(gWrapSamplerState, input.uv);
	else if (nPrimitiveID < 4)
		cColor = gtxtTextures[1].Sample(gWrapSamplerState, input.uv);
	else if (nPrimitiveID < 6)
		cColor = gtxtTextures[2].Sample(gWrapSamplerState, input.uv);
	else if (nPrimitiveID < 8)
		cColor = gtxtTextures[3].Sample(gWrapSamplerState, input.uv);
	else if (nPrimitiveID < 10)
		cColor = gtxtTextures[4].Sample(gWrapSamplerState, input.uv);
	else
		cColor = gtxtTextures[5].Sample(gWrapSamplerState, input.uv);
	float4 cColor = gtxtTextures[NonUniformResourceIndex(nPrimitiveID/2)].Sample(gWrapSamplerState, input.uv);

	return(cColor);
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
Texture2D gtxtTerrainTexture : register(t14);
Texture2D gtxtDetailTexture : register(t15);
Texture2D gtxtAlphaTexture : register(t16);
Texture2D gtxtAlphaTextures[] : register(t17);

float4 PSTextured(VS_SPRITE_TEXTURED_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtTerrainTexture.Sample(gssWrap, input.uv);

	return(cColor);
}

struct VS_TERRAIN_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

struct VS_TERRAIN_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input)
{
	VS_TERRAIN_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.color = input.color;
	output.uv0 = input.uv0;
	output.uv1 = input.uv1;

	return(output);
}

float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{
	float4 cBaseTexColor = gtxtTerrainTexture.Sample(gssWrap, input.uv0);
	float4 cDetailTexColor = gtxtDetailTexture.Sample(gssWrap, input.uv1);
	//	float fAlpha = gtxtTerrainTexture.Sample(gssWrap, input.uv0);

	float4 cColor = cBaseTexColor * 0.5f + cDetailTexColor * 0.5f;
	//	float4 cColor = saturate(lerp(cBaseTexColor, cDetailTexColor, fAlpha));

	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UI SHADERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Texture2D gtxtUITexture : register(t0);

// VS_SPRITE_TEXTURED_INPUT and VS_SPRITE_TEXTURED_OUTPUT are already defined and are suitable for UI.

VS_SPRITE_TEXTURED_OUTPUT VS_UI(VS_SPRITE_TEXTURED_INPUT input)
{
	VS_SPRITE_TEXTURED_OUTPUT output;

    // For UI, we assume the input position is already in NDC.
    // The C++ code will provide an identity matrix for the world matrix.
	output.position = float4(input.position, 1.0f);
	output.uv = input.uv;

	return(output);
}

float4 PS_UI(VS_SPRITE_TEXTURED_OUTPUT input) : SV_TARGET
{
	return gtxtUITexture.Sample(gssWrap, input.uv);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------
// Billboard Shaders
//-------------------------------------------------------------------------------------------------------------------------------------------------

Texture2D gtxtBillboard : register(t17); // 단일 텍스처로 변경

//-------------------------------------------------------------------------------------------------------------------------------------------------
// VS_INPUT: 애플리케이션에서 정점 하나의 정보를 받습니다.
//-------------------------------------------------------------------------------------------------------------------------------------------------
struct VS_BILLBOARD_INPUT
{
	float3 position : POSITION; // 빌보드가 생성될 월드 공간의 위치
};

//-------------------------------------------------------------------------------------------------------------------------------------------------
// GS_INPUT: Vertex Shader에서 넘어온 정보를 받습니다.
//-------------------------------------------------------------------------------------------------------------------------------------------------
struct GS_BILLBOARD_INPUT
{
	float3 position : POSITION; // VS에서 전달된 월드 공간 위치
};

//-------------------------------------------------------------------------------------------------------------------------------------------------
// PS_INPUT: Geometry Shader에서 생성된 정점 정보를 받습니다.
//-------------------------------------------------------------------------------------------------------------------------------------------------
struct PS_BILLBOARD_INPUT
{
	float4 position : SV_POSITION; // 최종 클립 공간 위치
	float2 uv : TEXCOORD;       // 텍스처 UV 좌표
};

//-------------------------------------------------------------------------------------------------------------------------------------------------
// Vertex Shader: 입력된 정점 데이터를 Geometry Shader로 그대로 전달합니다.
//-------------------------------------------------------------------------------------------------------------------------------------------------
GS_BILLBOARD_INPUT VSBillboard(VS_BILLBOARD_INPUT input)
{
	GS_BILLBOARD_INPUT output;
	output.position = input.position; // 월드 위치를 그대로 전달
	return output;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------
// Geometry Shader: 정점 하나를 입력받아 카메라를 향하는 사각형(Triangle Strip)을 생성합니다.
//-------------------------------------------------------------------------------------------------------------------------------------------------
[maxvertexcount(4)]
void GSBillboard(point GS_BILLBOARD_INPUT input[1], inout TriangleStream<PS_BILLBOARD_INPUT> outputStream)
{
    // 빌보드의 크기 (가로, 세로)
	float2 size = float2(2.0f, 2.0f); // 이 값은 C++에서 상수 버퍼로 넘겨주는 것이 더 유연합니다.

    // 카메라의 Up 벡터와 Right 벡터를 뷰 역행렬에서 추출
    // gmtxInverseView는 cbCameraInfo에 정의되어 있습니다.
	float3 up = normalize(gmtxInverseView._21_22_23);
	float3 right = normalize(gmtxInverseView._11_12_13);

    // 사각형의 네 꼭짓점 위치 계산
	float3 positions[4];
	positions[0] = input[0].position + (-right * size.x) + (up * size.y); // Top-Left
	positions[1] = input[0].position + (right * size.x) + (up * size.y);  // Top-Right
	positions[2] = input[0].position + (-right * size.x) - (up * size.y); // Bottom-Left
	positions[3] = input[0].position + (right * size.x) - (up * size.y);  // Bottom-Right

    // UV 좌표
	float2 uvs[4] =
	{
		float2(0.0f, 0.0f),
		float2(1.0f, 0.0f),
		float2(0.0f, 1.0f),
		float2(1.0f, 1.0f)
	};

	PS_BILLBOARD_INPUT output;
	
    // 4개의 정점을 Triangle Strip으로 출력
	[unroll]
	for (int i = 0; i < 4; i++)
	{
		output.position = float4(positions[i], 1.0f); // 월드 변환이 이미 적용된 꼭지점이므로 gmtxGameObject 곱셈 제거
		output.position = mul(output.position, gmtxView);
		output.position = mul(output.position, gmtxProjection);
		output.uv = uvs[i];
		outputStream.Append(output);
	}
	
	outputStream.RestartStrip();
}

//-------------------------------------------------------------------------------------------------------------------------------------------------
// Pixel Shader: 텍스처를 샘플링하고 알파값이 낮은 픽셀을 버립니다.
//-------------------------------------------------------------------------------------------------------------------------------------------------
float4 PSBillboard(PS_BILLBOARD_INPUT input) : SV_TARGET
{
	float4 color = gtxtBillboard.Sample(gssWrap, input.uv); // gssWrap (s0) 샘플러 재사용
    
    // 알파 값이 0.1보다 작으면 픽셀을 그리지 않음 (Alpha Test)
	clip(color.a - 0.1f);
	
	return color;
}