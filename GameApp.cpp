#define C11
#ifdef C11
#include"ui.h"
#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"
#include"EffectHelper.h"
#include"std_image_write.h"
#include"Vertex.h"
#include<filesystem>
#include"RenderState.h"
#include"TextureManager.h"
#include"Camera.h"
#include"CameraController.h"
#include"Texture2D.h"
#include"ModelManager.h"
using namespace DirectX;
//特效助理

EffectHelper effect;
TextureManager textureManager;
ModelManager model_manager;
FirstPersonCamera g_Fcamera;
FirstPersonCameraController controller;

TextureCube* skybox = nullptr;
TextureCube* irradiance = nullptr;
TextureCube* prefilter_map = nullptr;
Texture2D* BRDF_LUT = nullptr;
using EffectCallBack=std::function<void(EffectHelper*,Model*)>;
void PreComputeIBL(ID3D11DeviceContext* context) {
    ID3D11Device* m_pd3dDevice = nullptr;
    context->GetDevice(&m_pd3dDevice);
    {
        EffectHelper effect;
        effect.CreateShaderFromFile("CS", L"CS.hlsl", m_pd3dDevice, "CS", "cs_5_0", nullptr, nullptr);
        EffectPassDesc pass;
        pass.nameVS = "";
        pass.nameGS = "";
        pass.namePS = "";
        pass.nameDS = "";
        pass.nameHS = "";
        pass.nameCS = "CS";
        effect.AddEffectPass("CSPASS", m_pd3dDevice, &pass);
        TextureCube* cubemap = new TextureCube(m_pd3dDevice, 2048, 2048, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_BIND_UNORDERED_ACCESS);
        skybox = new TextureCube(m_pd3dDevice, 2048, 2048, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_BIND_SHADER_RESOURCE);
        CD3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc(D3D11_UAV_DIMENSION_TEXTURE2DARRAY, DXGI_FORMAT_R32G32B32A32_FLOAT);
        ID3D11UnorderedAccessView* ppUAView = nullptr;
        m_pd3dDevice->CreateUnorderedAccessView(cubemap->GetTexture(), &uavDesc, &ppUAView);
        effect.SetUnorderedAccessByName("cubemap", ppUAView);
        auto view = textureManager.CreateFromFile("resources\\HDRI\\Sky.hdr", false, true);
        effect.SetShaderResourceByName("HDR", view);
        effect.SetSamplerStateByName("g_SamLinear", RenderStates::SSAnistropicWrap16x.Get());
        effect.GetEffectPass("CSPASS")->Apply(context);
        context->Dispatch(64, 64, 6);
        context->CopyResource(skybox->GetTexture(), cubemap->GetTexture());
        delete cubemap;
    }
    {
        EffectHelper effect;
        effect.CreateShaderFromFile("CS", L"irradiance_map.hlsl", m_pd3dDevice, "CS", "cs_5_0", nullptr, nullptr);
        EffectPassDesc pass;
        pass.nameVS = "";
        pass.nameGS = "";
        pass.namePS = "";
        pass.nameDS = "";
        pass.nameHS = "";
        pass.nameCS = "CS";
        effect.AddEffectPass("CSPASS", m_pd3dDevice, &pass);
        TextureCube* cubemap = new TextureCube(m_pd3dDevice, 128, 128, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_BIND_UNORDERED_ACCESS);
        irradiance = new TextureCube(m_pd3dDevice, 128, 128, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_BIND_SHADER_RESOURCE);
        CD3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc(D3D11_UAV_DIMENSION_TEXTURE2DARRAY, DXGI_FORMAT_R32G32B32A32_FLOAT);
        ID3D11UnorderedAccessView* ppUAView = nullptr;
        m_pd3dDevice->CreateUnorderedAccessView(cubemap->GetTexture(), &uavDesc, &ppUAView);
        effect.SetUnorderedAccessByName("irradiance_map", ppUAView);
        effect.SetSamplerStateByName("g_SamLinear",RenderStates::SSAnistropicWrap16x.Get());
        effect.SetShaderResourceByName("g_TexCube", skybox->GetShaderResource());
        effect.GetEffectPass("CSPASS")->Apply(context);
        context->Dispatch(4,4, 6);
        context->CopyResource(irradiance->GetTexture(),cubemap->GetTexture());
        delete cubemap;

    }
    
    {
        EffectHelper effect;
        effect.CreateShaderFromFile("CS", L"prefilter_envmap.hlsl", m_pd3dDevice, "CS", "cs_5_0", nullptr, nullptr);
        EffectPassDesc pass;
        pass.nameVS = "";
        pass.nameGS = "";
        pass.namePS = "";
        pass.nameDS = "";
        pass.nameHS = "";
        pass.nameCS = "CS";
        effect.AddEffectPass("CSPASS", m_pd3dDevice, &pass);
        TextureCube* cubemap = new TextureCube(m_pd3dDevice, 2048, 2048, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_BIND_UNORDERED_ACCESS);
        prefilter_map = new TextureCube(m_pd3dDevice, 2048, 2048, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_BIND_SHADER_RESOURCE);
        D3D11_BOX box = {0,0,0,2048,2048,1};
        for (int i = 0; i < 6; i++) {
           
            context->CopySubresourceRegion(cubemap->GetTexture(),
            D3D11CalcSubresource(0, i, 12),0,0,0,
            skybox->GetTexture(),
            D3D11CalcSubresource(0, i, 1),nullptr);
        
        }        
        effect.SetSamplerStateByName("g_SamLinear", RenderStates::SSAnistropicWrap16x.Get());
        effect.SetShaderResourceByName("environment_map", skybox->GetShaderResource());
        for (int level = 1; level < 12; level++) {
            float roughness = (level) / 12.0f;
            CD3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc(D3D11_UAV_DIMENSION_TEXTURE2DARRAY,
                DXGI_FORMAT_R32G32B32A32_FLOAT,level);
            ID3D11UnorderedAccessView* ppUAView = nullptr;
            m_pd3dDevice->CreateUnorderedAccessView(cubemap->GetTexture(), &uavDesc, &ppUAView);
            effect.SetUnorderedAccessByName("prefilter_map", ppUAView);
            effect.GetConstantBufferVariable("roughness")->SetFloat(roughness);
            int resolution = 2048 / pow(2, level);
            int groups = resolution>32?resolution/32:1;
            effect.GetEffectPass("CSPASS")->Apply(context);
            context->Dispatch(groups, groups, 6);
        }
        context->CopyResource(prefilter_map->GetTexture(), cubemap->GetTexture());
        delete cubemap;
    }
    

    {
        EffectHelper effect;
        effect.CreateShaderFromFile("CS", L"environment_BRDF.hlsl", m_pd3dDevice, "CS", "cs_5_0", nullptr, nullptr);
        EffectPassDesc pass;
        pass.nameVS = "";
        pass.nameGS = "";
        pass.namePS = "";
        pass.nameDS = "";
        pass.nameHS = "";
        pass.nameCS = "CS";
        effect.AddEffectPass("CSPASS", m_pd3dDevice, &pass);
        Texture2D* BRDF = new Texture2D(m_pd3dDevice, 1024, 1024, DXGI_FORMAT_R16G16B16A16_FLOAT, 1, D3D11_BIND_UNORDERED_ACCESS);
        BRDF_LUT = new Texture2D(m_pd3dDevice, 1024, 1024, DXGI_FORMAT_R16G16B16A16_FLOAT, 1, D3D11_BIND_SHADER_RESOURCE);
        effect.SetUnorderedAccessByName("BRDF_LUT",BRDF->GetUnorderedAccess());
        effect.GetEffectPass("CSPASS")->Apply(context);
        context->Dispatch(32,32,1);
        context->CopyResource(BRDF_LUT->GetTexture(),BRDF->GetTexture());
    }
}
void Skydraw(ID3D11DeviceContext* context) {
    static bool initflag = false;
    if (!initflag) {
        initflag = true;
        model_manager.CreateFromGeometry("box", Geometry::CreateBox());
        effect.SetShaderResourceByName("g_TexCube", skybox->GetShaderResource());
    }
    effect.GetEffectPass("SkyBox")->Apply(context);
    Model* model = model_manager.GetModel("box");
    for (int i = 0; i < model->meshdatas.size(); i++) {
        ID3D11Buffer* buffer[5] = { model->meshdatas[i].m_pVertices.Get(),
            model->meshdatas[i].m_pNormals.Get(),
            model->meshdatas[i].m_pTangents.Get(),
            model->meshdatas[i].m_pTexcoordArrays.size() > 0 ? model->meshdatas[i].m_pTexcoordArrays[0].Get() : nullptr,
            model->meshdatas[i].m_pIndices.Get()
        };
        UINT stride[4] = { 12,12,16,8 };
        UINT offset[4] = { 0,0,0,0 };
        context->IASetVertexBuffers(0, 4, buffer, stride, offset);
        context->IASetIndexBuffer(buffer[4], model->meshdatas[i].m_IndexCount > 65535 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT, 0);
        context->DrawIndexed(model->meshdatas[i].m_IndexCount, 0, 0);
    }
}
void PBRDraw(ID3D11DeviceContext* context,Model* model, EffectCallBack call) {
    static bool initflag = false;
    static EffectHelper effect;
    if (!initflag) {
        initflag = true;
        ID3D11Device* m_pd3dDevice = nullptr;
        context->GetDevice(&m_pd3dDevice);
        //load shader
        D3D_SHADER_MACRO vDefines[] = {"PBR_VERTEX",NULL,NULL,NULL};
        effect.CreateShaderFromFile("PBRV", L"PBR.hlsl", m_pd3dDevice, "main", "vs_5_0",vDefines);
        D3D_SHADER_MACRO pDefines[] = { "PBR_PIXEL",NULL,NULL,NULL };
        effect.CreateShaderFromFile("PBRP", L"PBR.hlsl", m_pd3dDevice, "main", "ps_5_0", pDefines);
        EffectPassDesc pass;
        pass.nameVS = "PBRV";
        pass.nameGS = "";
        pass.namePS = "PBRP";
        pass.nameDS = "";
        pass.nameHS = "";
        effect.AddEffectPass("PBR", m_pd3dDevice, &pass);
        effect.GetEffectPass("PBR")->SetRasterizerState(RenderStates::RSNoCull.Get());
       // effect.GetEffectPass("PBR")->SetDepthStencilState(RenderStates::DSSLessEqual.Get(), 0);
        uint32_t shader_model[4] = {1,0};
        effect.GetConstantBufferVariable("model")->SetUIntVector(2, shader_model);
        float albedo[4] = { 1.0,0.0,0.0,1.0 };
        effect.GetConstantBufferVariable("albedo")->SetFloatVector(4,albedo);
        float dl_direction[4] = { 1.0,1.0,1.0,1.0 };
        effect.GetConstantBufferVariable("dl_direction")->SetFloatVector(4, dl_direction);
        effect.GetConstantBufferVariable("ao")->SetFloat(1.0);
        effect.GetConstantBufferVariable("specular")->SetFloat(0.5);
        effect.SetSamplerStateByName("g_SamLinear", RenderStates::SSAnistropicWrap16x.Get());

        effect.SetShaderResourceByName("irradiance_map", irradiance->GetShaderResource());
        effect.SetShaderResourceByName("prefilter_map", prefilter_map->GetShaderResource());
        effect.SetShaderResourceByName("BRDF_LUT", BRDF_LUT->GetShaderResource());
        auto normal_texture = textureManager.CreateFromFile("resources\\textures\\pbr\\wall\\normal.jpg",true,true);;
        auto albedo_texture = textureManager.CreateFromFile("resources\\textures\\pbr\\wall\\albedo.jpg", true, true);;
        auto metallic_texture = textureManager.CreateFromFile("resources\\textures\\pbr\\wall\\metallic.png", true, true);
        auto roughness_texture = textureManager.CreateFromFile("resources\\textures\\pbr\\wall\\roughness.jpg", true, true);;
        auto ao_texture = textureManager.CreateFromFile("resources\\textures\\pbr\\wall\\ao.png", true, true);

        effect.SetShaderResourceByName("albedo_map",albedo_texture);
        effect.SetShaderResourceByName("normal_map", normal_texture);
        effect.SetShaderResourceByName("metallic_map", metallic_texture);
        effect.SetShaderResourceByName("roughness_map", roughness_texture);
        effect.SetShaderResourceByName("ao_map", ao_texture);
    }
    call(&effect,model);
    effect.GetConstantBufferVariable("g_World")->SetVal(model->getTransform().GetLocalToWorldMatrixXM());

    Ui::NewInspector();   
    static float metalic = 0.1;
    static float roughness = 0.1;
    ImGui::SliderFloat("metalic",&metalic,0.0,1.0);
    ImGui::SliderFloat("roughness",&roughness,0.0,1.0);
    effect.GetConstantBufferVariable("roughness")->SetFloat(roughness);
    effect.GetConstantBufferVariable("metalness")->SetFloat(metalic); 

    static bool sample_albedo = 0;
    static bool sample_normal = 0;
    static bool sample_metallic = 0;
    static bool sample_roughness = 0;
    static bool sample_ao = 0;
    ImGui::Checkbox("sample_albedo", &sample_albedo);
    ImGui::SameLine();
    ImGui::Checkbox("sample_normal", &sample_normal);
    ImGui::Checkbox("sample_metallic", &sample_metallic);
    ImGui::SameLine();
    ImGui::Checkbox("sample_roughness", &sample_roughness);
    Ui::DrawVerticalLine();
    Ui::DrawTooltip("try");
    ImGui::Checkbox("sample_ao", &sample_ao);
    effect.GetConstantBufferVariable("sample_albedo")->SetSInt(sample_albedo);
    effect.GetConstantBufferVariable("sample_normal")->SetSInt(sample_normal);
    effect.GetConstantBufferVariable("sample_metallic")->SetSInt(sample_metallic);;
    effect.GetConstantBufferVariable("sample_roughness")->SetSInt(sample_roughness);;
    effect.GetConstantBufferVariable("sample_ao")->SetSInt(sample_ao);;
    static float uv_scale[2] = { 2,2 };
    ImGui::SliderFloat2("uv_scale",uv_scale,1,10);
    Ui::EndInspector();
    effect.GetConstantBufferVariable("uv_scale")->SetFloatVector(2, uv_scale);


    effect.GetEffectPass("PBR")->Apply(context);
    for (int i = 0; i < model->meshdatas.size(); i++) {
        ID3D11Buffer* buffer[5] = {
            model->meshdatas[i].m_pVertices.Get(),
            model->meshdatas[i].m_pNormals.Get(),
            model->meshdatas[i].m_pTangents.Get(),
            model->meshdatas[i].m_pTexcoordArrays.size() > 0 ? model->meshdatas[i].m_pTexcoordArrays[0].Get() : nullptr,
            model->meshdatas[i].m_pIndices.Get()
        };
        UINT stride[4] = { 12,12,16,8 };
        UINT offset[4] = { 0,0,0,0 };
        context->IASetVertexBuffers(0, 4, buffer, stride, offset);
        context->IASetIndexBuffer(buffer[4], model->meshdatas[i].m_IndexCount > 65535 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT, 0);
        context->DrawIndexed(model->meshdatas[i].m_IndexCount, 0, 0);
    }
}
GameApp::GameApp(HINSTANCE hInstance)
    : D3DApp(hInstance)
{
}
GameApp::~GameApp()
{
}
bool GameApp::Init()
{
    if (!D3DApp::Init())
        return false;

    RenderStates::InitAll(m_pd3dDevice.Get());
    textureManager.Init(m_pd3dDevice.Get());
    model_manager.Init(m_pd3dDevice.Get());
    if (!InitEffect())
        return false;
    if (!InitResource())
        return false;
    //初始化相机和相机控制器
    controller.InitCamera(&g_Fcamera);
    g_Fcamera.SetViewPort(m_ScreenViewport);
    g_Fcamera.SetFrustum(45, this->AspectRatio(), 0.1, 1000.0);
    //controller.SetMouseSensitivity(0.01,0.01);
    return true;
}
void GameApp::OnResize()
{
    D3DApp::OnResize();
    g_Fcamera.SetViewPort(m_ScreenViewport);
    g_Fcamera.SetFrustum(45, this->AspectRatio(), 0.1, 1000.0);
}
void GameApp::UpdateScene(float dt)
{
    controller.Update(dt);
    auto it = DirectX::XMMatrixTranspose(g_Fcamera.GetViewMatrixXM());
    effect.GetConstantBufferVariable("View")->SetFloatMatrix(4, 4, (float*)it.r);
    it = DirectX::XMMatrixTranspose(g_Fcamera.GetProjMatrixXM());
    effect.GetConstantBufferVariable("Proj")->SetFloatMatrix(4, 4, (float*)it.r);
    it = DirectX::XMMatrixTranspose(g_Fcamera.GetViewProjMatrixXM());
    effect.GetConstantBufferVariable("g_ViewProj")->SetFloatMatrix(4, 4, (float*)it.r);
}
void GameApp::DrawScene()
{
    assert(m_pd3dImmediateContext);
    assert(m_pSwapChain);
    static float black[4] = { 0.3f, 0.3f, 0.3f, 0.0f };
    m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), black);
    m_pd3dImmediateContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());
    Skydraw(m_pd3dImmediateContext.Get());
    auto callback = [] (EffectHelper* e,Model* model) {
        auto it = DirectX::XMMatrixTranspose(g_Fcamera.GetViewMatrixXM());
        e->GetConstantBufferVariable("g_View")->SetFloatMatrix(4, 4, (float*)it.r);
        it = DirectX::XMMatrixTranspose(g_Fcamera.GetProjMatrixXM());
        e->GetConstantBufferVariable("g_Proj")->SetFloatMatrix(4, 4, (float*)it.r);
        it = DirectX::XMMatrixTranspose(model->getTransform().GetLocalToWorldMatrixXM());
        e->GetConstantBufferVariable("g_World")->SetFloatMatrix(4, 4, (float*)it.r);
        
        e->GetConstantBufferVariable("position")->SetVal(g_Fcamera.GetPosition());;
    };
    PBRDraw(m_pd3dImmediateContext.Get(),model_manager.GetModel("Sphere"),callback);
#ifdef USE_IMGUI
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif
    m_pSwapChain->Present(0, 0);
}
LRESULT GameApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT res = D3DApp::MsgProc(hwnd, msg, wParam, lParam);
    return res;
}
bool GameApp::InitEffect()
{
    ComPtr<ID3DBlob> blob;

    effect.CreateShaderFromFile("SkyBoxV", L"SkyBoxV.hlsl", m_pd3dDevice.Get(), "main", "vs_5_0");

    effect.CreateShaderFromFile("SkyBoxP", L"SkyBoxP.hlsl", m_pd3dDevice.Get(), "main", "ps_5_0");

    effect.CreateShaderFromFile("VS", L"VS.hlsl", m_pd3dDevice.Get(), "main", "vs_5_0", nullptr, blob.GetAddressOf());
    // 创建并绑定顶点布局
    HR(m_pd3dDevice->CreateInputLayout(VertexPosNormalTangentTex::GetInputLayout(), 4,
        blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout.GetAddressOf()));
    effect.CreateShaderFromFile("PS", L"PS.hlsl", m_pd3dDevice.Get(), "main", "ps_5_0");
    EffectPassDesc pass;
    pass.nameVS = "SkyBoxV";
    pass.nameGS = "";
    pass.namePS = "SkyBoxP";
    pass.nameDS = "";
    pass.nameHS = "";
    effect.AddEffectPass("SkyBox", m_pd3dDevice.Get(), &pass);
    return true;
}
bool GameApp::InitResource()
{
    m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout.Get());
    effect.SetSamplerStateByName("g_SamLinear", RenderStates::SSAnistropicWrap16x.Get());
    effect.GetEffectPass("SkyBox")->SetRasterizerState(RenderStates::RSNoCull.Get());
    effect.GetEffectPass("SkyBox")->SetDepthStencilState(RenderStates::DSSLessEqual.Get(), 0);
    model_manager.CreateFromGeometry("Sphere",Geometry::CreateSphere(1.0,40,40));
    PreComputeIBL(m_pd3dImmediateContext.Get());
    return true;
}
#else
#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"
#include"Settings.h"
#include"EffectHelper.h"
#include"std_image_write.h"
#include"Vertex.h"
#include<direct.h>
#include<filesystem>
#include"RenderState.h"
#include"TextureManager.h"
#include"Camera.h"
#include"CameraController.h"
#include"Texture2D.h"
#include"ModelManager.h"
#include"LightHelper.h"
#include"SimpleMath.h"
#include"ScreenGrab11.h"
using namespace DirectX;
//特效助理

EffectHelper effect;
EffectHelper depth_effect;
Depth2D* depthtexture;
TextureCube* depthtexture_Cube;
TextureManager textureManager;
FirstPersonCamera g_Fcamera;
FirstPersonCameraController controller;
ModelManager model_manager;
float light[3] = { 0,0,0 };
static XMFLOAT3 lightpos[5] = { XMFLOAT3(10.0f, 0.0f, 0.0f),XMFLOAT3(-10.0f, 0.0f, 0.0f),XMFLOAT3(0.0f, 0.0f, 10.0f),XMFLOAT3(0.0f, 0.0f, -10.0f),XMFLOAT3(1.0f, 5.0f, 1.0f) };
void Basicdraw(Model* model, ID3D11DeviceContext* context) {
    auto it = DirectX::XMMatrixTranspose(model->getTransform().GetLocalToWorldMatrixXM());
    effect.GetConstantBufferVariable("g_World")->SetFloatMatrix(4, 4, (float*)&it.r);
    it = XMath::InverseTranspose(it);
    effect.GetConstantBufferVariable("g_WorldInvTranspose")->SetFloatMatrix(4, 4, (float*)it.r);
    for (int i = 0; i < model->meshdatas.size(); i++) {
        PhongMaterial phongMat{};
        auto material = model->materials[model->meshdatas[i].m_MaterialIndex];
        phongMat.ambient = material.Get<XMFLOAT4>("$AmbientColor");
        phongMat.diffuse = material.Get<XMFLOAT4>("$DiffuseColor");
        phongMat.diffuse.w = material.Get<float>("$Opacity");
        phongMat.specular = material.Get<XMFLOAT4>("$SpecularColor");
        phongMat.specular.w = material.Has<float>("$SpecularFactor") ? material.Get<float>("$SpecularFactor") : 1.0f;
        effect.GetConstantBufferVariable("g_Material")->SetRaw(&phongMat);
        auto it = material.TryGet<std::string>("$Diffuse");
        if (it) {
            effect.GetConstantBufferVariable("haveT")->SetSInt(1);
            effect.SetShaderResourceByName("g_DiffuseMap", textureManager.GetTexture(material.Get<std::string>("$Diffuse")));
        }
        else {
            effect.GetConstantBufferVariable("haveT")->SetSInt(0);
        }
        effect.GetEffectPass("Basic")->Apply(context);
        ID3D11Buffer* buffer[5] = {
            model->meshdatas[i].m_pVertices.Get(),
            model->meshdatas[i].m_pNormals.Get(),
            model->meshdatas[i].m_pTangents.Get(),
            model->meshdatas[i].m_pTexcoordArrays.size() > 0 ? model->meshdatas[i].m_pTexcoordArrays[0].Get() : nullptr,
            model->meshdatas[i].m_pIndices.Get()
        };
        UINT stride[4] = { 12,12,16,8 };
        UINT offset[4] = { 0,0,0,0 };
        context->IASetVertexBuffers(0, 4, buffer, stride, offset);
        context->IASetIndexBuffer(buffer[4], model->meshdatas[i].m_IndexCount > 65535 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT, 0);
        context->DrawIndexed(model->meshdatas[i].m_IndexCount, 0, 0);
    }

}
void Skydraw(ID3D11DeviceContext* context) {
    static bool initflag = false;
    ID3D11ShaderResourceView* it = nullptr;

    if (!initflag) {
        initflag = true;
        model_manager.CreateFromGeometry("box", Geometry::CreateBox());
        ID3D11Device* m_pd3dDevice = nullptr;
        context->GetDevice(&m_pd3dDevice);
        //使用计算着色器从hdr加载天空图

        effect.SetSamplerStateByName("g_SamLinear", RenderStates::SSAnistropicWrap16x.Get());
        effect.GetEffectPass("SkyBox")->SetRasterizerState(RenderStates::RSNoCull.Get());
        effect.GetEffectPass("SkyBox")->SetDepthStencilState(RenderStates::DSSLessEqual.Get(), 0);

        effect.CreateShaderFromFile("CS", L"CS.hlsl", m_pd3dDevice, "CS", "cs_5_0", nullptr, nullptr);
        EffectPassDesc pass;
        pass.nameVS = "";
        pass.nameGS = "";
        pass.namePS = "";
        pass.nameDS = "";
        pass.nameHS = "";
        pass.nameCS = "CS";
        effect.AddEffectPass("CSPASS", m_pd3dDevice, &pass);
        {
            TextureCube* cubemap = new TextureCube(m_pd3dDevice, 2048, 2048, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_BIND_UNORDERED_ACCESS);
            TextureCube* cubemapcopy = new TextureCube(m_pd3dDevice, 2048, 2048, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_BIND_SHADER_RESOURCE);
            CD3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc(D3D11_UAV_DIMENSION_TEXTURE2DARRAY, DXGI_FORMAT_R32G32B32A32_FLOAT);
            ID3D11UnorderedAccessView* ppUAView = nullptr;
            m_pd3dDevice->CreateUnorderedAccessView(cubemap->GetTexture(), &uavDesc, &ppUAView);
            effect.SetUnorderedAccessByName("cubemap", ppUAView);
            auto view = textureManager.CreateFromFile("E:\\C++\\opengl\\opengl\\res\\texture\\HDRI\\Sky.hdr", false, true);
            effect.SetShaderResourceByName("HDR", view);
            effect.SetSamplerStateByName("g_SamLinear", RenderStates::SSAnistropicWrap16x.Get());
            effect.GetEffectPass("CSPASS")->Apply(context);
            context->Dispatch(64, 64, 6);
            context->CopyResource(cubemapcopy->GetTexture(), cubemap->GetTexture());
            effect.SetShaderResourceByName("g_TexCube", cubemapcopy->GetShaderResource());
            SaveDDSTextureToFile(context, cubemapcopy->GetTexture(), L"./res.dds");
        }

    }
    effect.GetEffectPass("SkyBox")->Apply(context);
    Model* model = model_manager.GetModel("box");
    for (int i = 0; i < model->meshdatas.size(); i++) {
        ID3D11Buffer* buffer[5] = { model->meshdatas[i].m_pVertices.Get(),
            model->meshdatas[i].m_pNormals.Get(),
            model->meshdatas[i].m_pTangents.Get(),
            model->meshdatas[i].m_pTexcoordArrays.size() > 0 ? model->meshdatas[i].m_pTexcoordArrays[0].Get() : nullptr,
            model->meshdatas[i].m_pIndices.Get()
        };
        UINT stride[4] = { 12,12,16,8 };
        UINT offset[4] = { 0,0,0,0 };
        context->IASetVertexBuffers(0, 4, buffer, stride, offset);
        context->IASetIndexBuffer(buffer[4], model->meshdatas[i].m_IndexCount > 65535 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT, 0);
        context->DrawIndexed(model->meshdatas[i].m_IndexCount, 0, 0);
    }
}
void Wiredraw(Model* model, ID3D11DeviceContext* context) {
    auto it = DirectX::XMMatrixTranspose(model->getTransform().GetLocalToWorldMatrixXM());
    effect.GetConstantBufferVariable("World")->SetFloatMatrix(4, 4, (float*)&it.r);
    for (int i = 0; i < model->meshdatas.size(); i++) {
        effect.GetEffectPass("Line")->Apply(context);
        ID3D11Buffer* buffer[5] = { model->meshdatas[i].m_pVertices.Get(),
            model->meshdatas[i].m_pNormals.Get(),
            model->meshdatas[i].m_pTangents.Get(),
            model->meshdatas[i].m_pTexcoordArrays.size() > 0 ? model->meshdatas[i].m_pTexcoordArrays[0].Get() : nullptr,
            model->meshdatas[i].m_pIndices.Get()
        };
        UINT stride[4] = { 12,12,16,8 };
        UINT offset[4] = { 0,0,0,0 };
        context->IASetVertexBuffers(0, 4, buffer, stride, offset);
        context->IASetIndexBuffer(buffer[4], model->meshdatas[i].m_IndexCount > 65535 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT, 0);
        context->DrawIndexed(model->meshdatas[i].m_IndexCount, 0, 0);
    }
}
void Depthdraw(Model* model, ID3D11DeviceContext* context) {
    auto it = DirectX::XMMatrixTranspose(model->getTransform().GetLocalToWorldMatrixXM());
    depth_effect.GetConstantBufferVariable("g_World")->SetFloatMatrix(4, 4, (float*)&it.r);
    depth_effect.GetEffectPass("Depth")->Apply(context);
    for (int i = 0; i < model->meshdatas.size(); i++) {
        ID3D11Buffer* buffer[5] = {
            model->meshdatas[i].m_pVertices.Get(),
            model->meshdatas[i].m_pNormals.Get(),
            model->meshdatas[i].m_pTangents.Get(),
            model->meshdatas[i].m_pTexcoordArrays.size() > 0 ? model->meshdatas[i].m_pTexcoordArrays[0].Get() : nullptr,
            model->meshdatas[i].m_pIndices.Get()
        };
        UINT stride[4] = { 12,12,16,8 };
        UINT offset[4] = { 0,0,0,0 };
        context->IASetVertexBuffers(0, 4, buffer, stride, offset);
        context->IASetIndexBuffer(buffer[4], model->meshdatas[i].m_IndexCount > 65535 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT, 0);
        context->DrawIndexed(model->meshdatas[i].m_IndexCount, 0, 0);
    }
}




GameApp::GameApp(HINSTANCE hInstance)
    : D3DApp(hInstance)
{

}
GameApp::~GameApp()
{
}
bool GameApp::Init()
{
    if (!D3DApp::Init())
        return false;

    RenderStates::InitAll(m_pd3dDevice.Get());
    textureManager.Init(m_pd3dDevice.Get());
    model_manager.Init(m_pd3dDevice.Get());
    if (!InitEffect())
        return false;
    if (!InitResource())
        return false;


    effect.SetSamplerStateByName("g_Sam", RenderStates::SSLinearClamp.Get());
    effect.GetEffectPass("Basic")->SetRasterizerState(RenderStates::RSNoCull.Get());
    effect.GetEffectPass("Line")->SetRasterizerState(RenderStates::RSWireframe.Get());
    //初始化相机和相机控制器
    controller.InitCamera(&g_Fcamera);
    g_Fcamera.SetViewPort(m_ScreenViewport);
    g_Fcamera.SetFrustum(45, this->AspectRatio(), 0.1, 1000.0);
    //controller.SetMouseSensitivity(0.01,0.01);
    return true;
}
void GameApp::OnResize()
{
    D3DApp::OnResize();
    g_Fcamera.SetViewPort(m_ScreenViewport);
    g_Fcamera.SetFrustum(45, this->AspectRatio(), 0.1, 1000.0);

}
void GameApp::UpdateScene(float dt)
{
    controller.Update(dt);
    auto it = DirectX::XMMatrixTranspose(g_Fcamera.GetViewMatrixXM());
    effect.GetConstantBufferVariable("View")->SetFloatMatrix(4, 4, (float*)it.r);
    it = DirectX::XMMatrixTranspose(g_Fcamera.GetProjMatrixXM());
    effect.GetConstantBufferVariable("Proj")->SetFloatMatrix(4, 4, (float*)it.r);
    it = DirectX::XMMatrixTranspose(g_Fcamera.GetViewProjMatrixXM());
    effect.GetConstantBufferVariable("g_ViewProj")->SetFloatMatrix(4, 4, (float*)it.r);

    it = DirectX::XMMatrixIdentity();
    effect.GetConstantBufferVariable("g_EyePosW")->Set(g_Fcamera.GetPosition());
    depth_effect.GetConstantBufferVariable("lightPos")->SetFloatVector(3, light);
    effect.GetConstantBufferVariable("lightPos")->SetFloatVector(3, light);
    model_manager.GetModel("light")->getTransform().SetPosition(SimpleMath::Vector3(light));
    ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
    ImGui::SliderFloat3("lightpos", light, -10, 10);

    ImGui::End();
}
void GameApp::DrawScene()
{
    assert(m_pd3dImmediateContext);
    assert(m_pSwapChain);
    static float black[4] = { 0.3f, 0.3f, 0.3f, 0.0f };

#pragma region DepthDraw
    static XMFLOAT3 ups[6] = {
            { 0.0f, 1.0f, 0.0f },	// +X
            { 0.0f, 1.0f, 0.0f },	// -X
            { 0.0f, 0.0f, -1.0f },	// +Y
            { 0.0f, 0.0f, 1.0f },	// -Y
            { 0.0f, 1.0f, 0.0f },	// +Z
            { 0.0f, 1.0f, 0.0f }	// -Z
    };

    static XMFLOAT3 looks[6] = {
        { 1.0f, 0.0f, 0.0f },	// +X
        { -1.0f, 0.0f, 0.0f },	// -X
        { 0.0f, 1.0f, 0.0f },	// +Y
        { 0.0f, -1.0f, 0.0f },	// -Y
        { 0.0f, 0.0f, 1.0f },	// +Z
        { 0.0f, 0.0f, -1.0f },	// -Z
    };
    D3D11_VIEWPORT Viewport = { 0,0,1000,1000,0,1.0 };
    m_pd3dImmediateContext->RSSetViewports(1, &Viewport);

    auto Proj = XMMatrixPerspectiveFovLH(XM_PIDIV2, 1.0f, 0.1f, 1000.0f);

    for (int i = 0; i < 6; i++) {
        m_pd3dImmediateContext->ClearDepthStencilView(depthtexture->GetDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        m_pd3dImmediateContext->OMSetRenderTargets(0, nullptr, depthtexture->GetDepthStencil());
        auto view = SimpleMath::Matrix::CreateLookAt(SimpleMath::Vector3(light), looks[i] + SimpleMath::Vector3(light), ups[i]);
        auto it = DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(view, Proj));
        depth_effect.GetConstantBufferVariable("g_ViewProj")->SetFloatMatrix(4, 4, (float*)it.r);
        Depthdraw(model_manager.GetModel("nanosuit"), m_pd3dImmediateContext.Get());
        Depthdraw(model_manager.GetModel("wall"), m_pd3dImmediateContext.Get());
        m_pd3dImmediateContext->CopySubresourceRegion(depthtexture_Cube->GetTexture(), D3D11CalcSubresource(0, i, 1), 0, 0, 0, depthtexture->GetTexture(), 0, nullptr);
    }
#pragma endregion








    m_pd3dImmediateContext->RSSetViewports(1, &m_ScreenViewport);
    m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), black);
    m_pd3dImmediateContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());
    Skydraw(m_pd3dImmediateContext.Get());


    Basicdraw(model_manager.GetModel("nanosuit"), m_pd3dImmediateContext.Get());
    Basicdraw(model_manager.GetModel("wall"), m_pd3dImmediateContext.Get());
    Wiredraw(model_manager.GetModel("light"), m_pd3dImmediateContext.Get());
#ifdef USE_IMGUI
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif
    m_pSwapChain->Present(0, 0);
}
LRESULT GameApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT res = D3DApp::MsgProc(hwnd, msg, wParam, lParam);
    return res;
}
bool GameApp::InitEffect()
{
    ComPtr<ID3DBlob> blob;
    effect.CreateShaderFromFile("LineV", L"LineV.hlsl", m_pd3dDevice.Get(), "main", "vs_5_0");

    effect.CreateShaderFromFile("LineP", L"LineP.hlsl", m_pd3dDevice.Get(), "main", "ps_5_0");

    effect.CreateShaderFromFile("SkyBoxV", L"SkyBoxV.hlsl", m_pd3dDevice.Get(), "main", "vs_5_0");

    effect.CreateShaderFromFile("SkyBoxP", L"SkyBoxP.hlsl", m_pd3dDevice.Get(), "main", "ps_5_0");

    effect.CreateShaderFromFile("VS", L"HLSL/VS.hlsl", m_pd3dDevice.Get(), "main", "vs_5_0", nullptr, blob.GetAddressOf());
    // 创建并绑定顶点布局
    HR(m_pd3dDevice->CreateInputLayout(VertexPosNormalTangentTex::GetInputLayout(), 4,
        blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout.GetAddressOf()));
    effect.CreateShaderFromFile("PS", L"HLSL/PS.hlsl", m_pd3dDevice.Get(), "main", "ps_5_0");


    EffectPassDesc pass;

    pass.nameVS = "LineV";
    pass.nameGS = "";
    pass.namePS = "LineP";
    pass.nameDS = "";
    pass.nameHS = "";
    effect.AddEffectPass("Line", m_pd3dDevice.Get(), &pass);
    pass.nameVS = "VS";
    pass.nameGS = "";
    pass.namePS = "PS";
    pass.nameDS = "";
    pass.nameHS = "";
    effect.AddEffectPass("Basic", m_pd3dDevice.Get(), &pass);
    pass.nameVS = "SkyBoxV";
    pass.nameGS = "";
    pass.namePS = "SkyBoxP";
    pass.nameDS = "";
    pass.nameHS = "";
    effect.AddEffectPass("SkyBox", m_pd3dDevice.Get(), &pass);


    depth_effect.CreateShaderFromFile("V", L"DepthV.hlsl", m_pd3dDevice.Get(), "main", "vs_5_0");
    depth_effect.CreateShaderFromFile("P", L"DepthP.hlsl", m_pd3dDevice.Get(), "main", "ps_5_0");
    pass.nameVS = "V";
    pass.nameGS = "";
    pass.namePS = "P";
    pass.nameDS = "";
    pass.nameHS = "";
    depth_effect.AddEffectPass("Depth", m_pd3dDevice.Get(), &pass);
    return true;
}
bool GameApp::InitResource()
{
    depthtexture = new Depth2D(m_pd3dDevice.Get(), 1000, 1000);
    depthtexture_Cube = new TextureCube(m_pd3dDevice.Get(), 1000, 1000, DXGI_FORMAT_R24_UNORM_X8_TYPELESS, 1, D3D11_BIND_SHADER_RESOURCE);
    depthtexture->SetDebugObjectName("depthture");
    depthtexture_Cube->SetDebugObjectName("depthture_Cube");
    effect.SetShaderResourceByName("depthtexture_Cube", depthtexture_Cube->GetShaderResource());

    model_manager.CreateFromFile("nanosuit", "resources/objects/nanosuit/nanosuit.obj");
    model_manager.GetModel("nanosuit")->getTransform().SetPosition(0, 1, 0);
    model_manager.GetModel("nanosuit")->getTransform().SetScale(0.1, 0.1, 0.1);
    model_manager.CreateFromGeometry("grid", Geometry::CreateGrid({ 25,25 }, { 25,25 }, { 1,1 }));
    model_manager.CreateFromGeometry("wall", Geometry::CreateBox());
    model_manager.GetModel("wall")->getTransform().SetScale(25, 12.5, 25);
    model_manager.GetModel("wall")->getTransform().SetPosition(0, -15, 0);
    model_manager.CreateFromGeometry("light", Geometry::CreateSphere(0.05, 40, 40));


    m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout.Get());
#pragma region light
    // ******************
  // 初始化不会变化的值
  //
  // 环境光
    DirectionalLight dirLight{};
    dirLight.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    dirLight.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    dirLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    dirLight.direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
    effect.GetConstantBufferVariable("g_DirLight")->SetRaw(&dirLight, (sizeof dirLight) * 0, sizeof dirLight);
    // 灯光
    PointLight pointLight{};
    pointLight.position = XMFLOAT3(0.0f, 10.0f, 0.0f);
    pointLight.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    pointLight.diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
    pointLight.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    pointLight.att = XMFLOAT3(0.0f, 0.1f, 0.0f);
    pointLight.range = 30.0f;
    for (int i = 0; i < 5; i++) {
        pointLight.position = lightpos[i];
        effect.GetConstantBufferVariable("g_PointLight")->SetRaw(&pointLight, (sizeof pointLight) * i, sizeof pointLight);
    }
#pragma endregion
    return true;
}





#endif // C1


