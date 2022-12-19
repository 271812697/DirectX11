#include "shader.h"
#include "../util/global.h"
#include"../core/log.h"
#include"../RenderState.h"
asset::Shader::Shader(std::string path)
{
	Init(path);
}

void asset::Shader::Init(std::string path)
{
    CORE_INFO("Load Shader from {}",path);
	source_path = path;
    auto wpath=global::UTF8ToWString(path);
   
	effect = std::shared_ptr<EffectHelper>(new EffectHelper());
    //load shader
    auto m_pd3dDevice = global::GetGraphicI().m_pDevice.Get();
    D3D_SHADER_MACRO vDefines[] = { "VS",NULL,NULL,NULL };
    effect->CreateShaderFromFile("VS", wpath.c_str(), m_pd3dDevice, "main", "vs_5_0", vDefines);
    D3D_SHADER_MACRO pDefines[] = { "PS",NULL,NULL,NULL };
    effect->CreateShaderFromFile("PS", wpath.c_str(), m_pd3dDevice, "main", "ps_5_0", pDefines);
    D3D_SHADER_MACRO gDefines[] = { "GS",NULL,NULL,NULL };
    effect->CreateShaderFromFile("GS", wpath.c_str(), m_pd3dDevice, "main", "gs_5_0", vDefines);
    D3D_SHADER_MACRO dDefines[] = { "DS",NULL,NULL,NULL };
    effect->CreateShaderFromFile("DS", wpath.c_str(), m_pd3dDevice, "main", "ds_5_0", pDefines);
    D3D_SHADER_MACRO hDefines[] = { "HS",NULL,NULL,NULL };
    effect->CreateShaderFromFile("HS", wpath.c_str(), m_pd3dDevice, "main", "hs_5_0", vDefines);

    EffectPassDesc pass;
    pass.nameVS = "VS";
    pass.nameGS = "GS";
    pass.namePS = "PS";
    pass.nameDS = "HS";
    pass.nameHS = "DS";
    effect->AddEffectPass("Shader", m_pd3dDevice, &pass);
    //Ä¬ÈÏ¹âÕ¤Æ÷²»²Ã¼ô
    effect->GetEffectPass("Shader")->SetRasterizerState(RenderStates::RSNoCull.Get());

}

std::shared_ptr<EffectHelper> asset::Shader::GetEffectHelper()
{
	return effect;
}

void asset::Shader::SetSample(std::string_view name, ID3D11SamplerState* samplerState)
{
    effect->SetSamplerStateByName(name,samplerState);
}

void asset::Shader::setSV(std::string name, ID3D11ShaderResourceView* sv)
{
    effect->SetShaderResourceByName(name,sv);
}

void asset::Shader::SetRenderState(ID3D11RasterizerState* pRS, ID3D11DepthStencilState* pDSS, uint32_t stencilRef)
{
    if (pRS) {
        effect->GetEffectPass("Shader")->SetRasterizerState(pRS);
     }
    if (pDSS) {
        effect->GetEffectPass("Shader")->SetDepthStencilState(pDSS,stencilRef);
    }
}

void asset::Shader::Apply()
{
	CORE_ASERT(effect.get(), "you must first initalize the shader code");
	effect->GetEffectPass("Shader")->Apply(global::GetGraphicI().m_pDeviceContext.Get());
	
}
