#pragma once
#include"../EffectHelper.h"
#include<string>
namespace asset {
	class Shader {
	private:
		std::shared_ptr<EffectHelper> effect;
	public:
		Shader()=default;
		Shader(std::string path);
		void Init(std::string path);
		std::string source_path;
		std::string source_code;
		std::shared_ptr<EffectHelper> GetEffectHelper();
		void SetRenderState(ID3D11RasterizerState* pRS, ID3D11DepthStencilState* pDSS, uint32_t stencilRef);
		void Apply();
	};
}