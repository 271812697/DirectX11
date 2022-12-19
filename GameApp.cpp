#define C11
#ifdef C11
#include"ui.h"
#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"
#include"asset/shader.h"
#include"std_image_write.h"
#include"Vertex.h"
#include<filesystem>
#include"RenderState.h"
#include"TextureManager.h"
#include"Camera.h"
#include"CameraController.h"
#include"Texture2D.h"
#include"ModelManager.h"
#include"util/global.h"
#include"component/Material.h"
using namespace DirectX;
//特效助理

asset::Shader effect;
TextureManager textureManager;

FirstPersonCamera g_Fcamera;
FirstPersonCameraController controller;

TextureCube* skybox = nullptr;
TextureCube* irradiance = nullptr;
TextureCube* prefilter_map = nullptr;
Texture2D* BRDF_LUT = nullptr;
using EffectCallBack=std::function<void(component::Material*)>;
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
    static component::Mesh mesh;
    if (!initflag) {
        initflag = true;
        mesh.CreateSphere();
        effect.setSV("g_TexCube", skybox->GetShaderResource());
    }
    effect.Apply();
    mesh.Draw();
}
void PBRDraw(ID3D11DeviceContext* context,EffectCallBack call) {
    static bool initflag = false;
    static asset::Shader* effect=nullptr;
    static component::Material mat;
    static component::Mesh mesh;
    if (!initflag) {
        mesh.CreateSphere();
        initflag = true;
        ID3D11Device* m_pd3dDevice = nullptr;
        context->GetDevice(&m_pd3dDevice);
        effect = new asset::Shader();
        effect->Init("PBR.hlsl"); 
        mat.SetShader(std::shared_ptr<asset::Shader>( effect) );
        uint32_t shader_model[2] = {1,0};
        mat.SetVal("model",shader_model,2);
        float dl_direction[4] = { 1.0,1.0,1.0,1.0 };
        mat.SetVal("dl_direction",dl_direction,4);
        mat.SetVal("ao", 1.0f);;
        mat.SetVal("specular",0.5f);
        mat.SetSample("g_SamLinear", RenderStates::SSAnistropicWrap16x.Get());

        mat.setSV("irradiance_map", irradiance->GetShaderResource());
        mat.setSV("prefilter_map", prefilter_map->GetShaderResource());
        mat.setSV("BRDF_LUT", BRDF_LUT->GetShaderResource());
        auto normal_texture = textureManager.CreateFromFile("resources\\textures\\pbr\\wall\\normal.jpg",true,true);;
        auto albedo_texture = textureManager.CreateFromFile("resources\\textures\\pbr\\wall\\albedo.jpg", true, true);;
        auto metallic_texture = textureManager.CreateFromFile("resources\\textures\\pbr\\wall\\metallic.png", true, true);
        auto roughness_texture = textureManager.CreateFromFile("resources\\textures\\pbr\\wall\\roughness.jpg", true, true);;
        auto ao_texture = textureManager.CreateFromFile("resources\\textures\\pbr\\wall\\ao.png", true, true);

        mat.setSV("albedo_map",albedo_texture);
        mat.setSV("normal_map", normal_texture);
        mat.setSV("metallic_map", metallic_texture);
        mat.setSV("roughness_map", roughness_texture);
        mat.setSV("ao_map", ao_texture);
    }
    call(&mat);
    mat.Bind();
    mesh.Draw();
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

    global::InitGraphicI(m_pd3dDevice, m_pd3dImmediateContext);
    textureManager.Init(m_pd3dDevice.Get());
    
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
    effect.SetVal("View",it);
    it = DirectX::XMMatrixTranspose(g_Fcamera.GetProjMatrixXM());
    effect.SetVal("Proj", it);
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
    auto callback = [] (component::Material* e) {
        auto it = DirectX::XMMatrixTranspose(g_Fcamera.GetViewMatrixXM());
        e->SetVal("g_View", it);
        it = DirectX::XMMatrixTranspose(g_Fcamera.GetProjMatrixXM());
        e->SetVal("g_Proj", it);
        it = DirectX::XMMatrixIdentity();
        e->SetVal("g_World",it);
        e->SetVal("position",g_Fcamera.GetPosition());
        Ui::NewInspector();
        static float metalic = 0.1;
        static float roughness = 0.1;
        static bool sample_albedo = 0;
        static bool sample_normal = 0;
        static bool sample_metallic = 0;
        static bool sample_roughness = 0;
        static bool sample_ao = 0;
        static float albedo[4] = { 1.0,0.0,0.1,1.0 };
        static float uv_scale[2] = { 2,2 };
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
        Ui::EndInspector();
        //e->BindVal("roughness",&roughness);
        e->SetVal("roughness", roughness);
        e->SetVal("metalness", metalic);
        e->SetVal("albedo", albedo, 4);
        e->SetVal("sample_albedo", sample_albedo);
        e->SetVal("sample_normal", sample_normal);
        e->SetVal("sample_metallic", sample_metallic);
        e->SetVal("sample_roughness", sample_roughness);
        e->SetVal("sample_ao", sample_ao);
        e->SetVal("uv_scale", uv_scale, 2);
    };
    PBRDraw(m_pd3dImmediateContext.Get(),callback);
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
    effect.Init("SkyBoxV.hlsl");
    return true;
}
bool GameApp::InitResource()
{
    m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pd3dImmediateContext->IASetInputLayout(global::m_pVertexLayout.Get());
    //天空盒设置渲染状态
    effect.SetSample("g_SamLinear", RenderStates::SSAnistropicWrap16x.Get());
    effect.SetRenderState(RenderStates::RSNoCull.Get(), RenderStates::DSSLessEqual.Get(), 0);
    PreComputeIBL(m_pd3dImmediateContext.Get());
    return true;
}
#endif
#ifdef C12
#include"ui.h"
#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"
#include"EffectHelper.h"
#include"std_image_write.h"
#include"Vertex.h"
#include<filesystem>
#include"TextureManager.h"
#include"Camera.h"
#include"CameraController.h"
#include"Texture2D.h"
#include"ModelManager.h"
#include"component/mesh.h"
#include"../util/global.h"
#include "../asset/shader.h"
using namespace DirectX;
asset::Shader shader;
FirstPersonCamera g_Fcamera;
FirstPersonCameraController controller;
component::Mesh mesh;
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

   
    //初始化全局设置
    global::InitGraphicI(m_pd3dDevice.Get(),m_pd3dImmediateContext.Get());   
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
    auto effect = shader.GetEffectHelper();
    auto it = DirectX::XMMatrixTranspose(g_Fcamera.GetViewMatrixXM());
    effect->GetConstantBufferVariable("V")->SetFloatMatrix(4, 4, (float*)it.r);
    it = DirectX::XMMatrixTranspose(g_Fcamera.GetProjMatrixXM());
    effect->GetConstantBufferVariable("P")->SetFloatMatrix(4, 4, (float*)it.r);
    it = DirectX::XMMatrixIdentity();
    effect->GetConstantBufferVariable("W")->SetFloatMatrix(4, 4, (float*)it.r);

   
}
void GameApp::DrawScene()
{
    assert(m_pd3dImmediateContext);
    assert(m_pSwapChain);
    static float black[4] = { 0.3f, 0.3f, 0.3f, 0.0f };
    m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), black);
    m_pd3dImmediateContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());
    
    shader.Apply();
    mesh.Draw();


  
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
    shader.Init("HLSL/VS.hlsl");

    return true;
}
bool GameApp::InitResource()
{
    m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pd3dImmediateContext->IASetInputLayout(global::m_pVertexLayout.Get());
    shader.GetEffectHelper()->GetEffectPass("Shader")->SetDepthStencilState(RenderStates::DSSLessEqual.Get(), 0);
   
    mesh.CreateSphere();

    return true;
}
#endif // C12




