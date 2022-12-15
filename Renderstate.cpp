
#pragma once
#include "XUtil.h"
#include "RenderState.h"
#include "DXTrace.h"

using namespace Microsoft::WRL;

ComPtr<ID3D11RasterizerState> RenderStates::RSNoCull = nullptr;
ComPtr<ID3D11RasterizerState> RenderStates::RSWireframe = nullptr;
ComPtr<ID3D11RasterizerState> RenderStates::RSCullClockWise = nullptr;
ComPtr<ID3D11RasterizerState> RenderStates::RSShadow = nullptr;

ComPtr<ID3D11SamplerState> RenderStates::SSPointClamp = nullptr;
ComPtr<ID3D11SamplerState> RenderStates::SSLinearWrap = nullptr;
ComPtr<ID3D11SamplerState> RenderStates::SSLinearClamp = nullptr;
ComPtr<ID3D11SamplerState> RenderStates::SSAnistropicWrap16x = nullptr;
ComPtr<ID3D11SamplerState> RenderStates::SSAnistropicClamp2x = nullptr;
ComPtr<ID3D11SamplerState> RenderStates::SSAnistropicClamp4x = nullptr;
ComPtr<ID3D11SamplerState> RenderStates::SSAnistropicClamp8x = nullptr;
ComPtr<ID3D11SamplerState> RenderStates::SSAnistropicClamp16x = nullptr;
ComPtr<ID3D11SamplerState> RenderStates::SSShadowPCF = nullptr;

ComPtr<ID3D11BlendState> RenderStates::BSAlphaToCoverage = nullptr;
ComPtr<ID3D11BlendState> RenderStates::BSTransparent = nullptr;
ComPtr<ID3D11BlendState> RenderStates::BSAdditive = nullptr;
ComPtr<ID3D11BlendState> RenderStates::BSAlphaWeightedAdditive = nullptr;

ComPtr<ID3D11DepthStencilState> RenderStates::DSSEqual = nullptr;
ComPtr<ID3D11DepthStencilState> RenderStates::DSSLessEqual = nullptr;
ComPtr<ID3D11DepthStencilState> RenderStates::DSSGreaterEqual = nullptr;
ComPtr<ID3D11DepthStencilState> RenderStates::DSSNoDepthWrite = nullptr;
ComPtr<ID3D11DepthStencilState> RenderStates::DSSNoDepthTest = nullptr;
ComPtr<ID3D11DepthStencilState> RenderStates::DSSWriteStencil = nullptr;
ComPtr<ID3D11DepthStencilState> RenderStates::DSSEqualStencil = nullptr;


bool RenderStates::IsInit()
{
    // һ����˵��ʼ������������е�״̬����������
    return RSWireframe != nullptr;
}

void RenderStates::InitAll(ID3D11Device* device)
{
    // ��ǰ��ʼ�����Ļ���û��Ҫ������
    if (IsInit())
        return;
    // ******************
    // ��ʼ����դ����״̬
    //
    CD3D11_RASTERIZER_DESC rasterizerDesc(CD3D11_DEFAULT{});

    // �߿�ģʽ
    rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    HR(device->CreateRasterizerState(&rasterizerDesc, RSWireframe.GetAddressOf()));

    // �ޱ����޳�ģʽ
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.FrontCounterClockwise = false;
    HR(device->CreateRasterizerState(&rasterizerDesc, RSNoCull.GetAddressOf()));

    // ˳ʱ���޳�ģʽ
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    rasterizerDesc.FrontCounterClockwise = true;
    HR(device->CreateRasterizerState(&rasterizerDesc, RSCullClockWise.GetAddressOf()));

    // ���ƫ��ģʽ
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.FrontCounterClockwise = false;
    rasterizerDesc.DepthBias = 0;
    rasterizerDesc.DepthBiasClamp = 0.0f;
    rasterizerDesc.SlopeScaledDepthBias = 1.0f;
    HR(device->CreateRasterizerState(&rasterizerDesc, RSShadow.GetAddressOf()));

    // ******************
    // ��ʼ��������״̬
    //
    CD3D11_SAMPLER_DESC sampDesc(CD3D11_DEFAULT{});

    // �������Clampģʽ
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    HR(device->CreateSamplerState(&sampDesc, SSPointClamp.GetAddressOf()));

    // ���Թ�����Clampģʽ
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    HR(device->CreateSamplerState(&sampDesc, SSLinearClamp.GetAddressOf()));

    // 2���������Թ�����Clampģʽ
    sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    sampDesc.MaxAnisotropy = 2;
    HR(device->CreateSamplerState(&sampDesc, SSAnistropicClamp2x.GetAddressOf()));

    // 4���������Թ�����Clampģʽ
    sampDesc.MaxAnisotropy = 4;
    HR(device->CreateSamplerState(&sampDesc, SSAnistropicClamp4x.GetAddressOf()));

    // 8���������Թ�����Clampģʽ
    sampDesc.MaxAnisotropy = 8;
    HR(device->CreateSamplerState(&sampDesc, SSAnistropicClamp8x.GetAddressOf()));

    // 16���������Թ�����Clampģʽ
    sampDesc.MaxAnisotropy = 16;
    HR(device->CreateSamplerState(&sampDesc, SSAnistropicClamp16x.GetAddressOf()));

    // ���Թ�����Wrapģʽ
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.MaxAnisotropy = 0;
    HR(device->CreateSamplerState(&sampDesc, SSLinearWrap.GetAddressOf()));

    // 16���������Թ�����Wrapģʽ
    sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    sampDesc.MaxAnisotropy = 16;
    HR(device->CreateSamplerState(&sampDesc, SSAnistropicWrap16x.GetAddressOf()));

    // ������״̬����ȱȽ���Borderģʽ
    sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
    sampDesc.MaxAnisotropy = 1;
    sampDesc.MinLOD = 0.0f;
    sampDesc.MaxLOD = 0.0f;
    sampDesc.BorderColor[0] = 0.0f;
    sampDesc.BorderColor[1] = 0.0f;
    sampDesc.BorderColor[2] = 0.0f;
    sampDesc.BorderColor[3] = 1.0f;
    HR(device->CreateSamplerState(&sampDesc, SSShadowPCF.GetAddressOf()));

    // ******************
    // ��ʼ�����״̬
    //
    CD3D11_BLEND_DESC blendDesc(CD3D11_DEFAULT{});
    auto& rtDesc = blendDesc.RenderTarget[0];

    // Alpha-To-Coverageģʽ
    blendDesc.AlphaToCoverageEnable = true;
    HR(device->CreateBlendState(&blendDesc, BSAlphaToCoverage.GetAddressOf()));

    // ͸�����ģʽ
    // Color = SrcAlpha * SrcColor + (1 - SrcAlpha) * DestColor 
    // Alpha = SrcAlpha
    blendDesc.AlphaToCoverageEnable = false;
    rtDesc.BlendEnable = true;
    rtDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    rtDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
    rtDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
    rtDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
    rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    HR(device->CreateBlendState(&blendDesc, BSTransparent.GetAddressOf()));

    // �ӷ����ģʽ
    // Color = SrcColor + DestColor
    // Alpha = SrcAlpha + DestAlpha
    rtDesc.SrcBlend = D3D11_BLEND_ONE;
    rtDesc.DestBlend = D3D11_BLEND_ONE;
    rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
    rtDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
    rtDesc.DestBlendAlpha = D3D11_BLEND_ONE;
    rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    HR(device->CreateBlendState(&blendDesc, BSAdditive.GetAddressOf()));

    // ��AlphaȨ�صļӷ����ģʽ
    // Color = SrcAlpha * SrcColor + DestColor
    // Alpha = SrcAlpha
    rtDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    rtDesc.DestBlend = D3D11_BLEND_ONE;
    rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
    rtDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
    rtDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
    rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    HR(device->CreateBlendState(&blendDesc, BSAlphaWeightedAdditive.GetAddressOf()));

    // ******************
    // ��ʼ�����/ģ��״̬
    //
    CD3D11_DEPTH_STENCIL_DESC dsDesc(CD3D11_DEFAULT{});
    // ���������ֵһ�µ����ؽ���д������/ģ��״̬
    // û��Ҫд�����
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = D3D11_COMPARISON_EQUAL;
    HR(device->CreateDepthStencilState(&dsDesc, DSSEqual.GetAddressOf()));

    // LESS_EQUAL����
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    HR(device->CreateDepthStencilState(&dsDesc, DSSLessEqual.GetAddressOf()));

    // ����Z => GREATER_EQUAL����
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
    HR(device->CreateDepthStencilState(&dsDesc, DSSGreaterEqual.GetAddressOf()));

    // ������Ȳ��ԣ�����д�����ֵ��״̬
    // �����Ʒ�͸������ʱ��Ӧʹ��Ĭ��״̬
    // ����͸������ʱ��ʹ�ø�״̬������Чȷ�����״̬�Ľ���
    // ����ȷ����ǰ�ķ�͸����������赲�Ϻ��һ������
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
    dsDesc.StencilEnable = false;
    HR(device->CreateDepthStencilState(&dsDesc, DSSNoDepthWrite.GetAddressOf()));

    // �ر���Ȳ��Ե����/ģ��״̬
    // �����Ʒ�͸�����壬����ϸ��ջ���˳��
    // ����͸����������Ҫ���Ļ���˳��
    // ��Ĭ�������ģ����Ծ��ǹرյ�
    dsDesc.DepthEnable = false;
    HR(device->CreateDepthStencilState(&dsDesc, DSSNoDepthTest.GetAddressOf()));

    // ����Z��Ȳ��ԣ�ģ��ֵ�Ƚ�
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
    dsDesc.StencilEnable = true;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
    dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
    HR(device->CreateDepthStencilState(&dsDesc, DSSEqualStencil.GetAddressOf()));

    // ����Ȳ��ԣ���ģ��д��
    dsDesc.DepthEnable = false;
    dsDesc.StencilEnable = true;
    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_REPLACE;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_REPLACE;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_REPLACE;
    dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_REPLACE;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
    dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    HR(device->CreateDepthStencilState(&dsDesc, DSSWriteStencil.GetAddressOf()));


    // ******************
    // ���õ��Զ�����
    //
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
    SetDebugObjectName(RSCullClockWise.Get(), "RSCullClockWise");
    SetDebugObjectName(RSNoCull.Get(), "RSNoCull");
    SetDebugObjectName(RSWireframe.Get(), "RSWireframe");
    SetDebugObjectName(RSShadow.Get(), "RSShadow");

    SetDebugObjectName(SSPointClamp.Get(), "SSPointClamp");
    SetDebugObjectName(SSLinearWrap.Get(), "SSLinearWrap");
    SetDebugObjectName(SSLinearClamp.Get(), "SSLinearClamp");
    SetDebugObjectName(SSAnistropicWrap16x.Get(), "SSAnistropicWrap16x");
    SetDebugObjectName(SSAnistropicClamp2x.Get(), "SSAnistropicClamp2x");
    SetDebugObjectName(SSAnistropicClamp4x.Get(), "SSAnistropicClamp4x");
    SetDebugObjectName(SSAnistropicClamp8x.Get(), "SSAnistropicClamp8x");
    SetDebugObjectName(SSAnistropicClamp16x.Get(), "SSAnistropicClamp16x");
    SetDebugObjectName(SSShadowPCF.Get(), "SSShadowPCF");

    SetDebugObjectName(BSAlphaToCoverage.Get(), "BSAlphaToCoverage");
    SetDebugObjectName(BSTransparent.Get(), "BSTransparent");
    SetDebugObjectName(BSAdditive.Get(), "BSAdditive");

    SetDebugObjectName(DSSEqual.Get(), "DSSEqual");
    SetDebugObjectName(DSSGreaterEqual.Get(), "DSSGreaterEqual");
    SetDebugObjectName(DSSLessEqual.Get(), "DSSLessEqual");
    SetDebugObjectName(DSSNoDepthWrite.Get(), "DSSNoDepthWrite");
    SetDebugObjectName(DSSNoDepthTest.Get(), "DSSNoDepthTest");
    SetDebugObjectName(DSSWriteStencil.Get(), "DSSWriteStencil");
    SetDebugObjectName(DSSEqualStencil.Get(), "DSSEqualStencil");
#endif
}