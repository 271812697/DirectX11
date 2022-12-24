#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"
#include<filesystem>
#include"RenderState.h"
#include"TextureManager.h"

#include"util/global.h"
#include"example/scene01.h"
using namespace DirectX;

TextureManager textureManager;

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
    global::InitGraphicI(m_pd3dDevice, m_pd3dImmediateContext);
    textureManager.Init(m_pd3dDevice.Get());
    m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pd3dImmediateContext->IASetInputLayout(global::m_pVertexLayout.Get());
    cur= std::make_shared < scene::Scene01>("01");
   
    cur->Init();
    return true;
}
void GameApp::OnResize()
{
    D3DApp::OnResize();
  
}
void GameApp::UpdateScene(float dt)
{
    cur->OnImGuiRender();
}
void GameApp::DrawScene()
{
    assert(m_pd3dImmediateContext);
    assert(m_pSwapChain);
    static float black[4] = { 0.3f, 0.3f, 0.3f, 0.0f };
    m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), black);
    m_pd3dImmediateContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());
    cur->OnSceneRender();
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




