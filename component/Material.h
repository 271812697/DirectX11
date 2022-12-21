#pragma once
#include<d3d11.h>
#include<memory>
#include<map>
#include"../asset/shader.h"
#include"../util/util.h"
namespace component {
	using namespace util;
	template<typename T>
	class Uniform {
	public:
		std::string name;
		unsigned int size=1;  // if not an array, this is 1
	private:
		T value;
		const T* value_ptr;
		const std::vector<T>* array_ptr;


	public:
		void Upload(asset::Shader*shader) const {
			if (shader) {
				if (size == 1) {
					shader->SetVal(name,*value_ptr);
				}
				else {
					shader->SetVal(name, &array_ptr[0], array_ptr->size());
				}
			}

		}
		Uniform() = default;
		Uniform( std::string name):name(name) {}

		void operator<<(const T& value);
		void operator<<=(const T* value_ptr);
		void operator<<=(const std::vector<T>* array_ptr);

	};

	class Material{

	private:
		using uniform_variant = std::variant<
			Uniform<int>, Uniform<unsigned int>, Uniform<bool>, Uniform<float>,
			Uniform<DirectX::XMFLOAT2>, Uniform<DirectX::XMFLOAT3>, Uniform<DirectX::XMFLOAT4>,
			Uniform<DirectX::XMUINT2>, Uniform<DirectX::XMUINT3>, Uniform<DirectX::XMUINT4>,
			Uniform<DirectX::XMMATRIX>, Uniform<DirectX::XMINT2>, Uniform<DirectX::XMINT3>, Uniform<DirectX::XMINT4>
		>;
	public:
		std::shared_ptr<asset::Shader>shader;
		std::map<std::string, uniform_variant>uniforms;
		
	public:
		Material(asset_ref<Material>s): Material(*s) {

		}
		Material(std::shared_ptr<asset::Shader>s) {
			SetShader(s);
		}
		Material() = default;
		void SetShader(std::shared_ptr<asset::Shader>s) {
			shader = s;
		}
		void SetSample(std::string_view name, ID3D11SamplerState* samplerState);
		//设置着色器资源视图
		void setSV(std::string name, ID3D11ShaderResourceView* sv);
		template<typename T>
		void SetVal(std::string name, const T& val) {
			shader->SetVal(name,val);
		}
		template<typename T>
		void SetVal(std::string name, T* data, unsigned int count) {
			shader->SetVal(name,data,count);
		}
		void SetRenderState(ID3D11RasterizerState* pRS, ID3D11DepthStencilState* pDSS, uint32_t stencilRef);
		void Bind()const;
		template<typename T>
		void BindVal(std::string name, T* pointer) {
			if (!(uniforms.count(name) > 0)) {
				uniforms[name] = Uniform<T>(name);
			}			
			auto& unif_variant = uniforms[name];
		    CORE_ASERT(std::holds_alternative<Uniform<T>>(unif_variant), "Mismatched uniform type!");
		    std::get<Uniform<T>>(unif_variant) <<= pointer;
		}
	};
	template<typename T>
	inline void Uniform<T>::operator<<(const T& value)
	{
		this->value = value;
	}
	template<typename T>
	inline void Uniform<T>::operator<<=(const T* value_ptr)
	{
		this->value_ptr = value_ptr;
	}
	template<typename T>
	inline void Uniform<T>::operator<<=(const std::vector<T>* array_ptr)
	{
		this->array_ptr = array_ptr;
	}
}