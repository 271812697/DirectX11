#ifndef GAMEAPP_H
#define GAMEAPP_H
#include "d3dApp.h"
#include "Geometry.h"
class GameApp : public D3DApp
{

public:
	GameApp(HINSTANCE hInstance);
	~GameApp();
    enum  CameraMode{FP,TP};
	enum DrawSceneMode{VIEW,MARK};
	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();
	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
private:
	
	bool InitEffect();
	bool InitResource();

private:

	ComPtr<ID3D11InputLayout> m_pVertexLayout;	    // 顶点输入布局
	ComPtr<ID3D11Buffer> m_pVertexBuffer;			// 顶点缓冲区
	ComPtr<ID3D11Buffer> m_pIndexBuffer;			// 索引缓冲区
	UINT m_IndexCount;							    // 绘制物体的索引数组大小

	
	ComPtr<ID3D11RasterizerState> m_pRSWireframe;	// 光栅化状态: 线框模式
	ComPtr<ID3D11RasterizerState> m_pRSnoCull;
	bool m_IsWireframeMode;							// 当前是否为线框模式
};
#endif