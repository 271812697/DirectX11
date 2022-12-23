#pragma once
#include <DirectXMath.h>
#include<cmath>
namespace util {
	constexpr float PI = 3.141592654;
	constexpr float PI_2 = 3.141592654 / 2;
	
	inline float radians(float degree){
		return PI * degree / 180.0f;
	}
	inline float degree(float radians) {
		return radians * 180 / PI;
	}
	inline DirectX::XMFLOAT3 NormalizeXMFLOAT3(const DirectX::XMFLOAT3& a) {
		DirectX::XMVECTOR directionVec = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&a));
		DirectX::XMFLOAT3 res{};
		DirectX::XMStoreFloat3(&res, directionVec);
		return res;

	}
	inline DirectX::XMFLOAT3 operator*(const DirectX::XMFLOAT3& a, float b) {
		return DirectX::XMFLOAT3{ a.x * b,a.y * b,a.z * b };
	}
	inline DirectX::XMFLOAT3 operator*(float b, const DirectX::XMFLOAT3& a) {
		return DirectX::XMFLOAT3{ a.x * b,a.y * b,a.z * b };
	}
	inline DirectX::XMFLOAT3 operator/(DirectX::XMFLOAT3 a, float b) {
		return DirectX::XMFLOAT3{ a.x / b,a.y / b,a.z / b };
	}
	inline DirectX::XMFLOAT3 Cross(DirectX::XMFLOAT3 a, DirectX::XMFLOAT3 b) {
		DirectX::XMFLOAT3 res = { a.y * b.z - a.z * b.y,a.z * b.x - a.x * b.z,a.x * b.y - a.y * b.x };
		float l = res.x * res.x + res.y * res.y + res.z * res.z;
		l=sqrt(l);
		return res * (1.0 / l);
	}


}
