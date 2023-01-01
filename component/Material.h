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
		Material(std::shared_ptr<asset::Shader>s,bool pbr_mat=false) {
			SetShader(s);
			if (shader != nullptr&& pbr_mat) {
				shader->SetVal("sample_albedo",false);
				shader->SetVal("sample_normal", false);
				shader->SetVal("sample_metallic", false);
				shader->SetVal("sample_roughness", false);
				shader->SetVal("sample_ao", false);
				shader->SetVal("sample_emission", false);
				shader->SetVal("sample_displace", false);
				shader->SetVal("sample_opacity", false);
				shader->SetVal("sample_lightmap", false);
				shader->SetVal("sample_anisotan", false);

				// shared properties
				shader->SetVal("albedo",DirectX::XMFLOAT4(1.0,1.0,1.0,1.0));
				shader->SetVal("roughness", 1.0f);
				shader->SetVal("ao", 0.5f);
				shader->SetVal("emission", DirectX::XMFLOAT4(0.0,0.0,0.0,1.0));
				shader->SetVal("uv_scale",DirectX::XMFLOAT2(1.0,1.0));
				shader->SetVal("alpha_mask",0.0f);
				// standard model
				shader->SetVal("metalness", 0.0f);                    // metalness
				shader->SetVal("specular", 0.5f);                    // specular reflectance ~ [0.35, 1]
				shader->SetVal("anisotropy", 0.0f);                    // anisotropy ~ [-1, 1]
				shader->SetVal("aniso_dir", DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f));  // anisotropy direction

				// refraction model
				shader->SetVal("transmission", 0.0f);                    // transmission
				shader->SetVal("thickness", 2.0f);                    // thickness
				shader->SetVal("ior", 1.5f);                    // index of refraction (IOR)
				shader->SetVal("transmittance", DirectX::XMFLOAT3(1.0f,1.0f,1.0f));              // transmittance color
				shader->SetVal("tr_distance", 4.0f);                    // transmission distance
				shader->SetVal("volume_type", 0U);                      // volume type, 0 = uniform sphere, 1 = cube/box/glass

				// cloth model
				shader->SetVal("sheen_color", DirectX::XMFLOAT3(1.0f,1.0f,1.0f));              // sheen color
				shader->SetVal("subsurface_color", DirectX::XMFLOAT3(0.0f,0.0f,0.0f));              // subsurface color

				// additive clear coat layer
				shader->SetVal("clearcoat", 0.0f);                    // clearcoat
				shader->SetVal("clearcoat_roughness", 0.0f);                    // clearcoat roughness

				// shading model switch
				shader->SetVal("model", DirectX::XMUINT2(1, 0));             // uvec2 model

			}
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