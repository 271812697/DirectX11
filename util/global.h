#pragma once
#include <d3d11_1.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include"../core/log.h"
#include"../EffectHelper.h"
#include"../component/mesh.h"
namespace global {
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	struct GraphicI
	{
		ComPtr<ID3D11Device> m_pDevice=nullptr;
		ComPtr<ID3D11DeviceContext> m_pDeviceContext = nullptr;
		
		
	};
	inline GraphicI graphic;
	//默认的顶点布局
	inline ComPtr<ID3D11InputLayout> m_pVertexLayout = nullptr;

	inline void InitGraphicI(Microsoft::WRL::ComPtr<ID3D11Device> D, Microsoft::WRL::ComPtr<ID3D11DeviceContext> C) {
		if (graphic.m_pDevice != nullptr) { 
			CORE_WARN("global GraphicI has been initilaize!!!");
			return;
		}
		//初始化默认的顶点布局
		ComPtr<ID3DBlob> blob;
		EffectHelper effect;
		D3D_SHADER_MACRO vDefines[] = { "VS",NULL,NULL,NULL };
		effect.CreateShaderFromFile("VS", L"HLSL/bulltinVS.hlsl", D.Get(), "main", "vs_5_0", vDefines, blob.GetAddressOf());
		// 创建并绑定顶点布局
		D->CreateInputLayout(component::Mesh::Vertex::GetInputLayout(), 8,
			blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout.GetAddressOf());
		graphic.m_pDevice = D;
		graphic.m_pDeviceContext = C;
	}
	inline GraphicI& GetGraphicI() {
		return graphic;
	}
	// 以下不可删除
#pragma warning(push)
#pragma warning(disable: 28251)
	extern "C" __declspec(dllimport) int __stdcall MultiByteToWideChar(unsigned int cp, unsigned long flags, const char* str, int cbmb, wchar_t* widestr, int cchwide);
	extern "C" __declspec(dllimport) int __stdcall WideCharToMultiByte(unsigned int cp, unsigned long flags, const wchar_t* widestr, int cchwide, char* str, int cbmb, const char* defchar, int* used_default);
#pragma warning(pop)

	inline std::wstring UTF8ToWString(std::string_view utf8str)
	{
		if (utf8str.empty()) return std::wstring();
		int cbMultiByte = static_cast<int>(utf8str.size());
		int req = MultiByteToWideChar(65001, 0, utf8str.data(), cbMultiByte, nullptr, 0);
		std::wstring res(req, 0);
		MultiByteToWideChar(65001, 0, utf8str.data(), cbMultiByte, &res[0], req);
		return res;
	}

	inline std::string WStringToUTF8(std::wstring_view wstr)
	{
		if (wstr.empty()) return std::string();
		int cbMultiByte = static_cast<int>(wstr.size());
		int req = WideCharToMultiByte(65001, 0, wstr.data(), cbMultiByte, nullptr, 0, nullptr, nullptr);
		std::string res(req, 0);
		WideCharToMultiByte(65001, 0, wstr.data(), cbMultiByte, &res[0], req, nullptr, nullptr);
		return res;
	}


}
