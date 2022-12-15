
#pragma once

#ifndef RENDER_STATES_H
#define RENDER_STATES_H

#include "WinMin.h"
#include <wrl/client.h>
#include <d3d11_1.h>


class RenderStates
{
public:
    template <class T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    static bool IsInit();

    static void InitAll(ID3D11Device* device);
    // ʹ��ComPtr�����ֹ��ͷ�

public:
    static ComPtr<ID3D11RasterizerState> RSWireframe;		            // ��դ����״̬���߿�ģʽ
    static ComPtr<ID3D11RasterizerState> RSNoCull;			            // ��դ����״̬���ޱ���ü�ģʽ
    static ComPtr<ID3D11RasterizerState> RSCullClockWise;	            // ��դ����״̬��˳ʱ��ü�ģʽ
    static ComPtr<ID3D11RasterizerState> RSShadow;						// ��դ����״̬�����ƫ��ģʽ

    static ComPtr<ID3D11SamplerState> SSPointClamp;						// ������״̬���������Clampģʽ
    static ComPtr<ID3D11SamplerState> SSLinearWrap;			            // ������״̬�����Թ�����Wrapģʽ
    static ComPtr<ID3D11SamplerState> SSLinearClamp;					// ������״̬�����Թ�����Clampģʽ
    static ComPtr<ID3D11SamplerState> SSAnistropicWrap16x;		        // ������״̬��16���������Թ�����Wrapģʽ
    static ComPtr<ID3D11SamplerState> SSAnistropicClamp2x;		        // ������״̬��2���������Թ�����Clampģʽ
    static ComPtr<ID3D11SamplerState> SSAnistropicClamp4x;		        // ������״̬��4���������Թ�����Clampģʽ
    static ComPtr<ID3D11SamplerState> SSAnistropicClamp8x;		        // ������״̬��8���������Թ�����Clampģʽ
    static ComPtr<ID3D11SamplerState> SSAnistropicClamp16x;		        // ������״̬��16���������Թ�����Clampģʽ
    static ComPtr<ID3D11SamplerState> SSShadowPCF;						// ������״̬����ȱȽ���Borderģʽ

    static ComPtr<ID3D11BlendState> BSTransparent;		                // ���״̬��͸�����
    static ComPtr<ID3D11BlendState> BSAlphaToCoverage;	                // ���״̬��Alpha-To-Coverage
    static ComPtr<ID3D11BlendState> BSAdditive;			                // ���״̬���ӷ����
    static ComPtr<ID3D11BlendState> BSAlphaWeightedAdditive;            // ���״̬����AlphaȨ�صļӷ����ģʽ


    static ComPtr<ID3D11DepthStencilState> DSSEqual;					// ���/ģ��״̬��������������ֵ��ȵ�����
    static ComPtr<ID3D11DepthStencilState> DSSLessEqual;                // ���/ģ��״̬�����ڴ�ͳ��ʽ��պл���
    static ComPtr<ID3D11DepthStencilState> DSSGreaterEqual;             // ���/ģ��״̬�����ڷ���Z����
    static ComPtr<ID3D11DepthStencilState> DSSNoDepthWrite;             // ���/ģ��״̬�������ԣ�����д�����ֵ
    static ComPtr<ID3D11DepthStencilState> DSSNoDepthTest;              // ���/ģ��״̬���ر���Ȳ���
    static ComPtr<ID3D11DepthStencilState> DSSWriteStencil;		        // ���/ģ��״̬������Ȳ��ԣ�д��ģ��ֵ
    static ComPtr<ID3D11DepthStencilState> DSSEqualStencil;	            // ���/ģ��״̬������Z�����ģ��ֵ
};



#endif

