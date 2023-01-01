#include "scene01.h"
#include"../component/Camera.h"
#include"../component/CameraController.h"
#include"../util/global.h"
#include"../RenderState.h"
#include"../TextureManager.h"
#include"../ui.h"
#include"../util/math.h"
extern TextureManager textureManager;
namespace scene{
void Scene01::Init()
{
    PrecomputeIBL("resources\\HDRI\\Sky.hdr");
	resource_manager.Add(01, MakeAsset<asset::Shader>("HLSL\\SkyBoxV.hlsl"));
	resource_manager.Add(02, MakeAsset<asset::Shader>("HLSL\\PBR.hlsl"));
    resource_manager.Add(05, MakeAsset<asset::Shader>("HLSL\\Light.hlsl"));
    resource_manager.Add(07, MakeAsset<asset::Shader>("HLSL\\post_process01.hlsl"));

	resource_manager.Add(03, MakeAsset<component::Material>(resource_manager.Get<asset::Shader>(1)));
	resource_manager.Add(04, MakeAsset<component::Material>(resource_manager.Get<asset::Shader>(2),true));
    resource_manager.Add(06, MakeAsset<component::Material>(resource_manager.Get<asset::Shader>(5)));
	sphere = CreateEntity("sphere");
	sphere.AddComponent<component::Mesh>(component::Primitive::Sphere);
    sphere.AddComponent<component::Spotlight>(color::white,3.8f,4.0f, 10.0f, 45.0f);	
    if (auto& mat = sphere.AddComponent<component::Material>(resource_manager.Get<component::Material>(04));true) {
		SetupMaterial(mat,01);
	}
    mingyue = CreateEntity("mingyue");
    mingyue.AddComponent<component::Model>("res/Zhaolinger/Hero_Zhaolinger/Hero_Zhaolinger.fbx", component::Quality::High, true);
	sky= CreateEntity("skybox");
	sky.AddComponent<component::Mesh>(component::Primitive::Sphere);
	if (auto& mat = sky.AddComponent<component::Material>(resource_manager.Get<component::Material>(03)); true) {
		SetupMaterial(mat,2);
	}
    camera = CreateEntity("camera");
    auto& c=camera.AddComponent<component::FirstPersonCamera>();
    camera.AddComponent<component::FirstPersonCameraController>().InitCamera(&c);

    bloomsphere = CreateEntity("bloomsphere");
    bloomsphere.AddComponent<component::Mesh>(component::Primitive::Sphere);
    bloomsphere.GetComponent<component::Transform>().Translate({ 0,1,0 }, 2.0);
    bloomsphere.GetComponent<component::Transform>().SetScale({0.1,0.1,0.1});
    if (auto& mat = bloomsphere.AddComponent<component::Material>(resource_manager.Get<component::Material>(06)); true) {
        mat.SetVal("light_color",color::orange);
        mat.SetVal("light_intensity", 1.8f);
        mat.SetVal("bloom_factor", 2.0f);
    }


    
    AddFBO(global::GetWindowInstance()->m_ClientWidth, global::GetWindowInstance()->m_ClientHeight);
    AddFBO(global::GetWindowInstance()->m_ClientWidth, global::GetWindowInstance()->m_ClientHeight);
    auto& fbo= FBOs[0];
    fbo.AddColorTexture(2);
    
}

void Scene01::OnSceneRender()
{
    auto& fbo = FBOs[0];
    
	auto& mat = sky.GetComponent<component::Material>();
	auto it = DirectX::XMMatrixTranspose(camera.GetComponent<component::FirstPersonCamera>().GetViewMatrixXM());
	mat.SetVal("View", it);
	it = DirectX::XMMatrixTranspose(camera.GetComponent<component::FirstPersonCamera>().GetProjMatrixXM());
	mat.SetVal("Proj", it);
	mat.Bind();
    fbo.Bind();
	sky.GetComponent<component::Mesh>().Draw();
    
    sphere.GetComponent<component::Material>().Bind();
   // sphere.GetComponent<component::Mesh>().Draw();
    auto& mesh =mingyue.GetComponent<component::Model>().meshes;
    for (auto& ms : mesh) {
        ms.Draw();
    }

    bloomsphere.GetComponent<component::Material>().Bind();
    bloomsphere.GetComponent<component::Mesh>().Draw();
    
    fbo.UnBind();
    //post-render
    auto shader = resource_manager.Get<asset::Shader>(07);
    shader->setSV("bloom_texture", post_bloom(fbo.GetColorTexture(1)).GetShaderResource());
    shader->setSV("color_texture",fbo.GetColorTexture(0).GetShaderResource());
    shader->SetSample("g_SamLinear", RenderStates::SSLinearClamp.Get());
    shader->Apply();
    component::Mesh::DrawQuad();

    /*
    
      Ui::NewInspector();
    static bool index = 0;
    ImGui::Checkbox("Depth", &index);
    Ui::EndInspector();
    if (index)
        fbo.Draw(-1);
    else fbo.Draw(1);  
    */
}

void Scene01::OnImGuiRender()
{  
    //sphere
    {
    camera.GetComponent<component::FirstPersonCameraController>().Update(global::GetTimer().DeltaTime());
    auto callback = [this](component::Material* e,Entity& s) {
        static bool rotateModel = true;
        static float metalic = 0.1;
        static float roughness = 0.1;
        static bool sample_albedo = 0;
        static bool sample_normal = 0;
        static bool sample_metallic = 0;
        static bool sample_roughness = 0;
        static bool sample_ao = 0;
        static float albedo[4] = { 1.0,0.0,0.1,1.0 };
        static float uv_scale[2] = { 2,2 }; 
        static DirectX::XMFLOAT3 rotate{ 3.141592654 ,0,0 };
        Ui::NewInspector();
        ImGui::ColorEdit4("albedo", albedo);
        ImGui::SliderFloat("metalic", &metalic, 0.0, 1.0);
        ImGui::SliderFloat("roughness", &roughness, 0.0, 1.0);
        ImGui::Checkbox("sample_albedo", &sample_albedo);
        ImGui::SameLine();
        ImGui::Checkbox("sample_normal", &sample_normal);
        ImGui::Checkbox("sample_metallic", &sample_metallic);
        ImGui::SameLine();
        ImGui::Checkbox("sample_roughness", &sample_roughness);
        Ui::DrawVerticalLine();
        Ui::DrawTooltip("try");
        ImGui::Checkbox("sample_ao", &sample_ao);
        ImGui::SliderFloat2("uv_scale", uv_scale, 1, 10);
        ImGui::Checkbox("Rotate",&rotateModel);
        Ui::EndInspector();
        auto& g_Fcamera = camera.GetComponent<component::FirstPersonCamera>();
        auto it = DirectX::XMMatrixTranspose(g_Fcamera.GetViewMatrixXM());
        e->SetVal("g_View", it);
        it = DirectX::XMMatrixTranspose(g_Fcamera.GetProjMatrixXM());
        e->SetVal("g_Proj", it);
       if(rotateModel)
        s.GetComponent<component::Transform>().Rotate(rotate * global::GetTimer().DeltaTime());
        e->SetVal("g_World", DirectX::XMMatrixTranspose(s.GetComponent<component::Transform>().GetLocalToWorldMatrixXM()));
        e->SetVal("position", g_Fcamera.GetPosition());
        e->SetVal("roughness", roughness);
        e->SetVal("metalness", metalic);
        e->SetVal("albedo", albedo, 4);
        e->SetVal("sample_albedo", sample_albedo);
        e->SetVal("sample_normal", sample_normal);
        e->SetVal("sample_metallic", sample_metallic);
        e->SetVal("sample_roughness", sample_roughness);
        e->SetVal("sample_ao", sample_ao);
        e->SetVal("uv_scale", uv_scale, 2);
        auto& l = s.GetComponent<component::Spotlight>();
        e->SetVal("sl_position", g_Fcamera.GetPosition());
        e->SetVal("sl_direction", g_Fcamera.GetLookAxis());
    };
    auto mat=sphere.GetComponent<component::Material>();
    callback(&mat,sphere);
     }
    //bloomsphere
    {
    
    auto& mat = bloomsphere.GetComponent<component::Material>();
    auto& g_Fcamera = camera.GetComponent<component::FirstPersonCamera>();
    auto it = DirectX::XMMatrixTranspose(g_Fcamera.GetViewMatrixXM());
    mat.SetVal("g_View", it);
    it = DirectX::XMMatrixTranspose(g_Fcamera.GetProjMatrixXM());
    mat.SetVal("g_Proj", it);
    mat.SetVal("g_World", DirectX::XMMatrixTranspose(bloomsphere.GetComponent<component::Transform>().GetLocalToWorldMatrixXM()));
    mat.SetVal("time",global::GetTimer().TotalTime());
    }

   
}
void Scene01::PrecomputeIBL(const std::string& hdri)
{
    ID3D11Device* m_pd3dDevice = global::GetGraphicI().m_pDevice.Get();
    auto context = global::GetGraphicI().m_pDeviceContext.Get();

    {
        EffectHelper effect;
        effect.CreateShaderFromFile("CS", L"HLSL\\CS.hlsl", m_pd3dDevice, "CS", "cs_5_0", nullptr, nullptr);
        EffectPassDesc pass;
        pass.nameVS = "";
        pass.nameGS = "";
        pass.namePS = "";
        pass.nameDS = "";
        pass.nameHS = "";
        pass.nameCS = "CS";
        effect.AddEffectPass("CSPASS", m_pd3dDevice, &pass);
        TextureCube* cubemap = new TextureCube(m_pd3dDevice, 2048, 2048, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_BIND_UNORDERED_ACCESS);
        skybox = std::make_shared<TextureCube>(m_pd3dDevice, 2048, 2048, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_BIND_SHADER_RESOURCE);
        CD3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc(D3D11_UAV_DIMENSION_TEXTURE2DARRAY, DXGI_FORMAT_R32G32B32A32_FLOAT);
        ID3D11UnorderedAccessView* ppUAView = nullptr;
        m_pd3dDevice->CreateUnorderedAccessView(cubemap->GetTexture(), &uavDesc, &ppUAView);
        effect.SetUnorderedAccessByName("cubemap", ppUAView);
        auto view = textureManager.CreateFromFile(hdri, false, true);
        effect.SetShaderResourceByName("HDR", view);
        effect.SetSamplerStateByName("g_SamLinear", RenderStates::SSAnistropicWrap16x.Get());
        effect.GetEffectPass("CSPASS")->Apply(context);
        context->Dispatch(64, 64, 6);
        context->CopyResource(skybox->GetTexture(), cubemap->GetTexture());
        delete cubemap;
    }
    {
        EffectHelper effect;
        effect.CreateShaderFromFile("CS", L"HLSL\\irradiance_map.hlsl", m_pd3dDevice, "CS", "cs_5_0", nullptr, nullptr);
        EffectPassDesc pass;
        pass.nameVS = "";
        pass.nameGS = "";
        pass.namePS = "";
        pass.nameDS = "";
        pass.nameHS = "";
        pass.nameCS = "CS";
        effect.AddEffectPass("CSPASS", m_pd3dDevice, &pass);
        TextureCube* cubemap = new TextureCube(m_pd3dDevice, 128, 128, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_BIND_UNORDERED_ACCESS);
        irradiance = std::make_shared < TextureCube>(m_pd3dDevice, 128, 128, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_BIND_SHADER_RESOURCE);
        CD3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc(D3D11_UAV_DIMENSION_TEXTURE2DARRAY, DXGI_FORMAT_R32G32B32A32_FLOAT);
        ID3D11UnorderedAccessView* ppUAView = nullptr;
        m_pd3dDevice->CreateUnorderedAccessView(cubemap->GetTexture(), &uavDesc, &ppUAView);
        effect.SetUnorderedAccessByName("irradiance_map", ppUAView);
        effect.SetSamplerStateByName("g_SamLinear", RenderStates::SSAnistropicWrap16x.Get());
        effect.SetShaderResourceByName("g_TexCube", skybox->GetShaderResource());
        effect.GetEffectPass("CSPASS")->Apply(context);
        context->Dispatch(4, 4, 6);
        context->CopyResource(irradiance->GetTexture(), cubemap->GetTexture());
        delete cubemap;

    }

    {
        EffectHelper effect;
        effect.CreateShaderFromFile("CS", L"HLSL\\prefilter_envmap.hlsl", m_pd3dDevice, "CS", "cs_5_0", nullptr, nullptr);
        EffectPassDesc pass;
        pass.nameVS = "";
        pass.nameGS = "";
        pass.namePS = "";
        pass.nameDS = "";
        pass.nameHS = "";
        pass.nameCS = "CS";
        effect.AddEffectPass("CSPASS", m_pd3dDevice, &pass);
        TextureCube* cubemap = new TextureCube(m_pd3dDevice, 2048, 2048, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_BIND_UNORDERED_ACCESS);
        prefilter_map = std::make_shared < TextureCube>(m_pd3dDevice, 2048, 2048, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_BIND_SHADER_RESOURCE);
        D3D11_BOX box = { 0,0,0,2048,2048,1 };
        for (int i = 0; i < 6; i++) {

            context->CopySubresourceRegion(cubemap->GetTexture(),
                D3D11CalcSubresource(0, i, 12), 0, 0, 0,
                skybox->GetTexture(),
                D3D11CalcSubresource(0, i, 1), nullptr);

        }
        effect.SetSamplerStateByName("g_SamLinear", RenderStates::SSAnistropicWrap16x.Get());
        effect.SetShaderResourceByName("environment_map", skybox->GetShaderResource());
        for (int level = 1; level < 12; level++) {
            float roughness = (level) / 12.0f;
            CD3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc(D3D11_UAV_DIMENSION_TEXTURE2DARRAY,
                DXGI_FORMAT_R32G32B32A32_FLOAT, level);
            ID3D11UnorderedAccessView* ppUAView = nullptr;
            m_pd3dDevice->CreateUnorderedAccessView(cubemap->GetTexture(), &uavDesc, &ppUAView);
            effect.SetUnorderedAccessByName("prefilter_map", ppUAView);
            effect.GetConstantBufferVariable("roughness")->SetFloat(roughness);
            int resolution = 2048 / pow(2, level);
            int groups = resolution > 32 ? resolution / 32 : 1;
            effect.GetEffectPass("CSPASS")->Apply(context);
            context->Dispatch(groups, groups, 6);
        }
        context->CopyResource(prefilter_map->GetTexture(), cubemap->GetTexture());
        delete cubemap;
    }


    {
        EffectHelper effect;
        effect.CreateShaderFromFile("CS", L"HLSL\\environment_BRDF.hlsl", m_pd3dDevice, "CS", "cs_5_0", nullptr, nullptr);
        EffectPassDesc pass;
        pass.nameVS = "";
        pass.nameGS = "";
        pass.namePS = "";
        pass.nameDS = "";
        pass.nameHS = "";
        pass.nameCS = "CS";
        effect.AddEffectPass("CSPASS", m_pd3dDevice, &pass);
        Texture2D* BRDF = new Texture2D(m_pd3dDevice, 1024, 1024, DXGI_FORMAT_R16G16B16A16_FLOAT, 1, D3D11_BIND_UNORDERED_ACCESS);
        BRDF_LUT = std::make_shared < Texture2D>(m_pd3dDevice, 1024, 1024, DXGI_FORMAT_R16G16B16A16_FLOAT, 1, D3D11_BIND_SHADER_RESOURCE);
        effect.SetUnorderedAccessByName("BRDF_LUT", BRDF->GetUnorderedAccess());
        effect.GetEffectPass("CSPASS")->Apply(context);
        context->Dispatch(32, 32, 1);
        context->CopyResource(BRDF_LUT->GetTexture(), BRDF->GetTexture());
    }

}
Texture2D Scene01::post_bloom(Texture2D pic)
{
    ID3D11Device* m_pd3dDevice = global::GetGraphicI().m_pDevice.Get();
    auto context = global::GetGraphicI().m_pDeviceContext.Get();  
    D3D11_TEXTURE2D_DESC pDesc;
    pic.GetTexture()->GetDesc(&pDesc);
    //down sample
    int w = pic.GetWidth()/2, h = pic.GetHeight()/2;
    Texture2D res(m_pd3dDevice, w, h, pDesc.Format);
    static EffectHelper effect;
    static bool initflag = false;
    if(!initflag)
    {
        initflag = true;
        effect.CreateShaderFromFile("CS", L"HLSL\\bloom.hlsl", m_pd3dDevice, "main", "cs_5_0", nullptr, nullptr);
        effect.CreateShaderFromFile("DOWNSAMPLE", L"HLSL\\downsample.hlsl", m_pd3dDevice, "main", "cs_5_0", nullptr, nullptr);
        EffectPassDesc pass;
        pass.nameVS = "";
        pass.nameGS = "";
        pass.namePS = "";
        pass.nameDS = "";
        pass.nameHS = "";
        pass.nameCS = "CS";
        effect.AddEffectPass("CSPASS", m_pd3dDevice, &pass);
        pass.nameCS = "DOWNSAMPLE";
        effect.AddEffectPass("downsample", m_pd3dDevice, &pass);

    }        
    Texture2D ping(m_pd3dDevice,w,h,pDesc.Format,1, D3D11_BIND_UNORDERED_ACCESS);
    Texture2D pong(m_pd3dDevice, w, h, pDesc.Format, 1, D3D11_BIND_UNORDERED_ACCESS);
    //下采样环节
    effect.SetUnorderedAccessByName("output",ping.GetUnorderedAccess());
    effect.SetShaderResourceByName("input",pic.GetShaderResource());
    effect.SetSamplerStateByName("g_SamLinear",RenderStates::SSPointClamp.Get());
    effect.GetEffectPass("downsample")->Apply(context);
    context->Dispatch(w / 32, h / 18, 1);
   //交替模糊环节
    effect.SetUnorderedAccessByName("output", nullptr);
    effect.SetUnorderedAccessByName("ping",ping.GetUnorderedAccess());
    effect.SetUnorderedAccessByName("pong", pong.GetUnorderedAccess());
    for (int i = 0; i < 6; ++i) {
            effect.GetConstantBufferVariable("horizontal")->SetUInt(i % 2 == 0); 
            effect.GetEffectPass("CSPASS")->Apply(context);
            context->Dispatch(w / 32, h / 18,1);
    }
    context->CopyResource(res.GetTexture(), pong.GetTexture());
    return res;
}
void Scene01::SetupMaterial(Material& pbr_mat, int mat_id)
{
	if (mat_id == 1) {
		//小球
        uint32_t shader_model[2] = { 1,0 };
        pbr_mat.SetVal("model", shader_model, 2);
        float dl_direction[4] = { 1.0,1.0,1.0,1.0 };
        pbr_mat.SetVal("dl_direction", dl_direction, 4);
        pbr_mat.SetVal("ao", 1.0f);;
        pbr_mat.SetVal("specular", 0.5f);
        pbr_mat.SetSample("g_SamLinear", RenderStates::SSAnistropicWrap16x.Get());
        pbr_mat.SetSample("g_SSAnistropicClamp16x",RenderStates::SSAnistropicClamp16x.Get());
        pbr_mat.setSV("irradiance_map", irradiance->GetShaderResource());
        pbr_mat.setSV("prefilter_map", prefilter_map->GetShaderResource());
        pbr_mat.setSV("BRDF_LUT", BRDF_LUT->GetShaderResource());
        auto normal_texture = textureManager.CreateFromFile("resources\\textures\\pbr\\wall\\normal.jpg", true, true);;
        auto albedo_texture = textureManager.CreateFromFile("resources\\textures\\pbr\\wall\\albedo.jpg", true, true);;
        auto metallic_texture = textureManager.CreateFromFile("resources\\textures\\pbr\\wall\\metallic.png", true, true);
        auto roughness_texture = textureManager.CreateFromFile("resources\\textures\\pbr\\wall\\roughness.jpg", true, true);;
        auto ao_texture = textureManager.CreateFromFile("resources\\textures\\pbr\\wall\\ao.png", true, true);
        pbr_mat.setSV("albedo_map", albedo_texture);
        pbr_mat.setSV("normal_map", normal_texture);
        pbr_mat.setSV("metallic_map", metallic_texture);
        pbr_mat.setSV("roughness_map", roughness_texture);
        pbr_mat.setSV("ao_map", ao_texture);
        auto& l = sphere.GetComponent<component::Spotlight>();
        pbr_mat.SetVal("sl_color",l.color);
        pbr_mat.SetVal("sl_intensity", l.intensity);
        pbr_mat.SetVal("sl_inner_cos",l.GetInnerCosine() );
        pbr_mat.SetVal("sl_outer_cos", l.GetOuterCosine());
        pbr_mat.SetVal("sl_range", l.range);
	}
	else if (mat_id == 2) {
		//天空盒
		pbr_mat.setSV("g_TexCube", skybox->GetShaderResource());
        //天空盒设置渲染状态
        pbr_mat.SetSample("g_SamLinear", RenderStates::SSAnistropicWrap16x.Get());
        pbr_mat.SetRenderState(RenderStates::RSNoCull.Get(), RenderStates::DSSLessEqual.Get(), 0);
	}
}
}