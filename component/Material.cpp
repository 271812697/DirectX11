#include "Material.h"

void component::Material::SetSample(std::string_view name, ID3D11SamplerState* samplerState)
{
	shader->SetSample(name,samplerState);
}

void component::Material::setSV(std::string name, ID3D11ShaderResourceView* sv)
{
	shader->setSV(name,sv);
}

void component::Material::SetRenderState(ID3D11RasterizerState* pRS, ID3D11DepthStencilState* pDSS, uint32_t stencilRef)
{
	shader->SetRenderState(pRS, pDSS,stencilRef);
}

void component::Material::Bind() const
{

	static auto upload = [this](auto& unif) { unif.Upload(this->shader.get());  };
	for (const auto& it : uniforms) {
		std::visit(upload,it.second);	
	}
	shader->Apply();
}
