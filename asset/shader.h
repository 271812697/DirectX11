#pragma once
#include"../EffectHelper.h"
#include<string>
namespace asset {
	class Shader {
	private:
		std::shared_ptr<EffectHelper> effect;
	public:
		Shader() = default;
		Shader(std::string path);
		void Init(std::string path);
		std::string source_path;
		std::string source_code;
		std::shared_ptr<EffectHelper> GetEffectHelper();
		void SetSample(std::string_view name, ID3D11SamplerState* samplerState);
		//设置着色器资源视图
		void setSV(std::string name, ID3D11ShaderResourceView* sv);
		template<typename T>
		void SetVal(std::string name,const T& val) {
			effect->GetConstantBufferVariable(name)->SetRaw(&val,0,sizeof(T));
		}
		template<typename T>
		void SetVal(std::string name, T*data,unsigned int count ) {
			effect->GetConstantBufferVariable(name)->SetRaw(data, 0, count*sizeof(T));
		}
		void SetRenderState(ID3D11RasterizerState* pRS, ID3D11DepthStencilState* pDSS, uint32_t stencilRef);
		void Apply();
	};
}