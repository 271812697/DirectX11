#include"fbo.h"
#include"../d3dApp.h"
#include"../util/global.h"
#include"shader.h"
#include"../component/mesh.h"
namespace asset {
	FBO::FBO(UINT width, UINT height):width(width),height(height)
	{
		auto& Graphic = global::GetGraphicI();
		DXGI_SAMPLE_DESC sampleDesc{ 4,0 };
		depth_Texture = std::make_shared<Depth2DMS>(Graphic.m_pDevice.Get(), width, height, sampleDesc);

	}
	FBO::~FBO()
	{
	}
	void FBO::AddColorTexture(UINT count)
	{
		auto& Graphic=global::GetGraphicI();
		DXGI_SAMPLE_DESC sampleDesc{ 4,0 };
		for (int i = 0; i < count; i++) {
			this->color_attachments.push_back(std::make_shared<Texture2DMS>(Graphic.m_pDevice.Get(),width,height, DXGI_FORMAT_R32G32B32A32_FLOAT,sampleDesc));
		}
	}
	void FBO::Draw(int index)
	{
		if(index!=-1)
		CORE_ASERT(index<this->color_attachments.size(),"the index is out of range");
		
		static asset::Shader shader("HLSL//framebuffer.hlsl");
		auto& Graphic = global::GetGraphicI();
		UINT choice = 0;

		if(index==-1){
			
		    shader.setSV("depth_texture", depth_Texture->GetShaderResource());
			choice = 1;


		}else{
		Texture2D screen(Graphic.m_pDevice.Get(), width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);
		Graphic.m_pDeviceContext->ResolveSubresource(screen.GetTexture(),0,color_attachments[index]->GetTexture() ,0, DXGI_FORMAT_R32G32B32A32_FLOAT);

	
		shader.setSV("color_texture", screen.GetShaderResource());
		}
        shader.SetVal("index",choice);
		shader.SetSample("g_SamLinear",RenderStates::SSLinearClamp.Get());
		shader.Apply();
		component::Mesh::DrawQuad();
	}
	void FBO::Draw(Texture2D)
	{
      //
	}
	void FBO::Bind()
	{
		static float black[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		int n = this->color_attachments.size();
		CORE_ASERT(n>0,"you must firstly Add ColorTexture");

		auto& Graphic = global::GetGraphicI();
		// 设置视口变换
		D3D11_VIEWPORT viewport{0,0,width,height,0.0,1.0};
		Graphic.m_pDeviceContext->RSSetViewports(1, &viewport);
		ID3D11RenderTargetView** ppRenderTargetViews = new ID3D11RenderTargetView * [n];
		for (int i = 0; i < n; i++) {

			ppRenderTargetViews[i] = this->color_attachments[i]->GetRenderTarget();
			Graphic.m_pDeviceContext->ClearRenderTargetView(ppRenderTargetViews[i],black);
		}
		Graphic.m_pDeviceContext->ClearDepthStencilView(depth_Texture->GetDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		Graphic.m_pDeviceContext->OMSetRenderTargets(n, ppRenderTargetViews, depth_Texture->GetDepthStencil());
		delete ppRenderTargetViews;
	}
	void FBO::UnBind()
	{
		auto& Graphic = global::GetGraphicI();
		auto instance = D3DApp::Get();
		Graphic.m_pDeviceContext->RSSetViewports(1, &instance->m_ScreenViewport);
		Graphic.m_pDeviceContext->OMSetRenderTargets(1,instance->m_pRenderTargetView.GetAddressOf(), instance->m_pDepthStencilView.Get());
	}
    Texture2D FBO::GetColorTexture(UINT index) 
	{
		CORE_ASERT(index < this->color_attachments.size(), "the index is out of range");
		auto& Graphic = global::GetGraphicI();
		Texture2D screen(Graphic.m_pDevice.Get(), width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);
		Graphic.m_pDeviceContext->ResolveSubresource(screen.GetTexture(), 0, color_attachments[index]->GetTexture(), 0, DXGI_FORMAT_R32G32B32A32_FLOAT);
		return screen;
	}
}