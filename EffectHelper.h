#pragma once

#ifndef EFFECT_HELPER_H
#define EFFECT_HELPER_H

#include "WinMin.h"
#include <string_view>
#include <memory>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include "Property.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11GeometryShader;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;
struct ID3D11SamplerState;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11BlendState;
//
// EffectHelper
//

// ��Ⱦͨ������
// ͨ��ָ�������ɫ��ʱ�ṩ��������������ɫ��
struct EffectPassDesc
{
    std::string_view nameVS;
    std::string_view nameDS;
    std::string_view nameHS;
    std::string_view nameGS;
    std::string_view namePS;
    std::string_view nameCS;
};

// �����������ı���
// ��COM���
struct IEffectConstantBufferVariable
{
    // �����޷���������Ҳ����Ϊbool����
    virtual void SetUInt(uint32_t val) = 0;
    // �����з�������
    virtual void SetSInt(int val) = 0;
    // ���ø�����
    virtual void SetFloat(float val) = 0;

    // �����޷���������������������1����4������
    // ��ɫ����������ΪboolҲ����ʹ��
    // ����Ҫ���õķ���������ȡdata��ǰ��������
    virtual void SetUIntVector(uint32_t numComponents, const uint32_t data[4]) = 0;

    // �����з���������������������1����4������
    // ����Ҫ���õķ���������ȡdata��ǰ��������
    virtual void SetSIntVector(uint32_t numComponents, const int data[4]) = 0;

    // ���ø�������������������1����4������
    // ����Ҫ���õķ���������ȡdata��ǰ��������
    virtual void SetFloatVector(uint32_t numComponents, const float data[4]) = 0;

    // �����޷�����������������������1-4
    // Ҫ��������û����䣬����3x3�������ֱ�Ӵ���UINT[3][3]����
    virtual void SetUIntMatrix(uint32_t rows, uint32_t cols, const uint32_t* noPadData) = 0;

    // �����з�����������������������1-4
    // Ҫ��������û����䣬����3x3�������ֱ�Ӵ���INT[3][3]����
    virtual void SetSIntMatrix(uint32_t rows, uint32_t cols, const int* noPadData) = 0;

    // ���ø���������������������1-4
    // Ҫ��������û����䣬����3x3�������ֱ�Ӵ���FLOAT[3][3]����
    virtual void SetFloatMatrix(uint32_t rows, uint32_t cols, const float* noPadData) = 0;

    // �����������ͣ�����ָ�����÷�Χ
    virtual void SetRaw(const void* data, uint32_t byteOffset = 0, uint32_t byteCount = 0xFFFFFFFF) = 0;

    // ��������
    virtual void Set(const Property& prop) = 0;

    // ��ȡ���һ�����õ�ֵ������ָ����ȡ��Χ
    virtual HRESULT GetRaw(void* pOutput, uint32_t byteOffset = 0, uint32_t byteCount = 0xFFFFFFFF) = 0;
    template<typename T>
    void SetVal(T val) {
        SetRaw(&val, 0, sizeof(T));
    }
    virtual ~IEffectConstantBufferVariable() {}
};

// ��Ⱦͨ��
// ��COM���
class EffectHelper;
struct IEffectPass
{
    // ���ù�դ��״̬
    virtual void SetRasterizerState(ID3D11RasterizerState* pRS) = 0;
    // ���û��״̬
    virtual void SetBlendState(ID3D11BlendState* pBS, const float blendFactor[4], uint32_t sampleMask) = 0;
    // ������Ȼ��״̬
    virtual void SetDepthStencilState(ID3D11DepthStencilState* pDSS, uint32_t stencilValue) = 0;

    // ��ȡ������ɫ����uniform�β���������ֵ
    virtual std::shared_ptr<IEffectConstantBufferVariable> VSGetParamByName(std::string_view paramName) = 0;
    // ��ȡ����ɫ����uniform�β���������ֵ
    virtual std::shared_ptr<IEffectConstantBufferVariable> DSGetParamByName(std::string_view paramName) = 0;
    // ��ȡ�����ɫ����uniform�β���������ֵ
    virtual std::shared_ptr<IEffectConstantBufferVariable> HSGetParamByName(std::string_view paramName) = 0;
    // ��ȡ������ɫ����uniform�β���������ֵ
    virtual std::shared_ptr<IEffectConstantBufferVariable> GSGetParamByName(std::string_view paramName) = 0;
    // ��ȡ������ɫ����uniform�β���������ֵ
    virtual std::shared_ptr<IEffectConstantBufferVariable> PSGetParamByName(std::string_view paramName) = 0;
    // ��ȡ������ɫ����uniform�β���������ֵ
    virtual std::shared_ptr<IEffectConstantBufferVariable> CSGetParamByName(std::string_view paramName) = 0;
    // ��ȡ������Ч����
    virtual EffectHelper* GetEffectHelper() = 0;
    // ��ȡ��Ч��
    virtual const std::string& GetPassName() = 0;

    // Ӧ����ɫ��������������(���������β�)������������ɫ����Դ�Ϳɶ�д��Դ����Ⱦ����
    virtual void Apply(ID3D11DeviceContext* deviceContext) = 0;

    virtual ~IEffectPass() {};
};

// ��Ч����
// ���������ɫ��������������ɫ����Դ����������������ɫ���βΡ��ɶ�д��Դ����Ⱦ״̬
class EffectHelper
{
public:

    EffectHelper();
    ~EffectHelper();
    // ���������������ƶ�
    EffectHelper(const EffectHelper&) = delete;
    EffectHelper& operator=(const EffectHelper&) = delete;
    EffectHelper(EffectHelper&&) = default;
    EffectHelper& operator=(EffectHelper&&) = default;

    // ���ñ���õ���ɫ���ļ�����·��������
    // ������Ϊ""����رջ���
    // ��forceWriteΪtrue��ÿ�����г��򶼻�ǿ�Ƹ��Ǳ���
    // Ĭ������²��Ỻ�����õ���ɫ��
    // ��shaderû����޸ĵ�ʱ��Ӧ����forceWrite
    void SetBinaryCacheDirectory(std::wstring_view cacheDir, bool forceWrite = false);

    // ������ɫ�� �� ��ȡ��ɫ���ֽ��룬������˳��
    // 1. ���������ɫ���ֽ����ļ�����·�� �� �ر�ǿ�Ƹ��ǣ������ȳ��Զ�ȡ${cacheDir}/${shaderName}.cso�����
    // 2. �����ȡfilename����Ϊ��ɫ���ֽ��룬ֱ�����
    // 3. ��filenameΪhlslԴ�룬����б������ӡ�������ɫ���ֽ����ļ�����ᱣ����ɫ���ֽ��뵽${cacheDir}/${shaderName}.cso
    // ע�⣺
    // 1. ��ͬ��ɫ�����룬������������ʹ��ͬһ���ۣ���Ӧ�Ķ���Ӧ������ȫһ��
    // 2. ��ͬ��ɫ�����룬������ȫ�ֱ���������Ӧ������ȫһ��
    // 3. ��ͬ��ɫ�����룬������������ɫ����Դ��ɶ�д��Դʹ��ͬһ���ۣ���Ӧ�Ķ���Ӧ������ȫһ�£�����ֻ��ʹ�ð�������
    HRESULT CreateShaderFromFile(std::string_view shaderName, std::wstring_view filename, ID3D11Device* device,
        LPCSTR entryPoint = nullptr, LPCSTR shaderModel = nullptr, const D3D_SHADER_MACRO* pDefines = nullptr, ID3DBlob** ppShaderByteCode = nullptr);

    // ��������ɫ��
    static HRESULT CompileShaderFromFile(std::wstring_view filename, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob** ppShaderByteCode, ID3DBlob** ppErrorBlob = nullptr,
        const D3D_SHADER_MACRO* pDefines = nullptr, ID3DInclude* pInclude = D3D_COMPILE_STANDARD_FILE_INCLUDE);

    // ��ӱ���õ���ɫ����������Ϣ��Ϊ�����ñ�ʶ��
    // �ú������ᱣ����ɫ�������Ʊ��뵽�ļ�
    // ע�⣺
    // 1. ��ͬ��ɫ�����룬������������ʹ��ͬһ��register����Ӧ�Ķ���Ӧ������ȫһ��
    // 2. ��ͬ��ɫ�����룬������ȫ�ֱ���������Ӧ������ȫһ��
    // 3. ��ͬ��ɫ�����룬������������ɫ����Դ��ɶ�д��Դʹ��ͬһ���ۣ���Ӧ�Ķ���Ӧ������ȫһ��
    HRESULT AddShader(std::string_view name, ID3D11Device* device, ID3DBlob* blob);

    // ��Ӵ�������ļ�����ɫ����Ϊ�����ñ�ʶ��
    // �ú������ᱣ����ɫ�������Ʊ��뵽�ļ�
    // ע�⣺
    // 1. ��ͬ��ɫ�����룬������������ʹ��ͬһ���ۣ���Ӧ�Ķ���Ӧ������ȫһ��
    // 2. ��ͬ��ɫ�����룬������ȫ�ֱ���������Ӧ������ȫһ��
    // 3. ��ͬ��ɫ�����룬������������ɫ����Դ��ɶ�д��Դʹ��ͬһ���ۣ���Ӧ�Ķ���Ӧ������ȫһ�£�����ֻ��ʹ�ð������� 
    HRESULT AddGeometryShaderWithStreamOutput(std::string_view name, ID3D11Device* device, ID3D11GeometryShader* gsWithSO, ID3DBlob* blob);

    // �����������
    void Clear();

    // ������Ⱦͨ��
    HRESULT AddEffectPass(std::string_view effectPassName, ID3D11Device* device, const EffectPassDesc* pDesc);
    // ��ȡ�ض���Ⱦͨ��
    std::shared_ptr<IEffectPass> GetEffectPass(std::string_view effectPassName);

    // ��ȡ�����������ı�����������ֵ
    std::shared_ptr<IEffectConstantBufferVariable> GetConstantBufferVariable(std::string_view name);

    // �������ò�����״̬
    void SetSamplerStateBySlot(uint32_t slot, ID3D11SamplerState* samplerState);
    // �������ò�����״̬(������ͬ�۶�������ֻ��ʹ�ð�������)
    void SetSamplerStateByName(std::string_view name, ID3D11SamplerState* samplerState);
    // ����ӳ�������״̬��(�Ҳ�������-1)
    int MapSamplerStateSlot(std::string_view name);

    // ����������ɫ����Դ
    void SetShaderResourceBySlot(uint32_t slot, ID3D11ShaderResourceView* srv);
    // ����������ɫ����Դ(������ͬ�۶�������ֻ��ʹ�ð�������)
    void SetShaderResourceByName(std::string_view name, ID3D11ShaderResourceView* srv);
    // ����ӳ����ɫ����Դ��(�Ҳ�������-1)
    int MapShaderResourceSlot(std::string_view name);

    // �������ÿɶ�д��Դ
    void SetUnorderedAccessBySlot(uint32_t slot, ID3D11UnorderedAccessView* uav, uint32_t* pInitialCount = nullptr);
    // �������ÿɶ�д��Դ(������ͬ�۶�������ֻ��ʹ�ð�������)
    void SetUnorderedAccessByName(std::string_view name, ID3D11UnorderedAccessView* uav, uint32_t* pInitialCount = nullptr);
    // ����ӳ��ɶ�д��Դ��(�Ҳ�������-1)
    int MapUnorderedAccessSlot(std::string_view name);


    // ���õ��Զ�����
    void SetDebugObjectName(std::string name);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};



#endif
