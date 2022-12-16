#pragma once
#include <d3d11_1.h>
#include <DirectXMath.h>
#include <wrl/client.h>
namespace global {
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	struct GraphicI
	{
		Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice=nullptr;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pDeviceContext = nullptr;
		
	};
	inline GraphicI graphic;

	inline void InitGraphicI(Microsoft::WRL::ComPtr<ID3D11Device> D, Microsoft::WRL::ComPtr<ID3D11DeviceContext> C) {
		if (graphic.m_pDevice != nullptr)return;
		graphic.m_pDevice = D;
		graphic.m_pDeviceContext = C;
	}
	inline GraphicI& GetGraphicI() {
		return graphic;
	}


}
