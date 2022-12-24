#pragma once
#include <d3d11_1.h>
#include <wrl/client.h>
#include <vector>
#include<memory>
#include"Texture2D.h"
namespace asset {	
	template <typename  T>
    using ComPtr= Microsoft::WRL::ComPtr<T>;
	class FBO {
	private:
		UINT width, height;
		std::vector<std::shared_ptr<Texture2DMS>>color_attachments;// 渲染目标视图
		std::shared_ptr<Depth2DMS> depth_Texture;   // 深度模板视图
		
	public:
		FBO() = default;
		FBO(UINT width, UINT height);
		~FBO();
		FBO(const FBO&) = delete;
		FBO& operator=(const FBO&) = delete;
		FBO(FBO&& other) noexcept = default;
		FBO& operator=(FBO&& other) noexcept = default;
	public:
		void AddColorTexture(UINT count);
		void Draw(int index);
		void Bind();
		void UnBind();
		const Texture2D& GetColorTexture(UINT index) const;
		//void SetColorTexture();


		


	};

}