#ifndef GAMEAPP_H
#define GAMEAPP_H
#include "d3dApp.h"
#include "Geometry.h"
#include"scene/scene.h"
class GameApp : public D3DApp
{

public:
	GameApp(HINSTANCE hInstance);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();
	std::shared_ptr<scene::Scene> cur;
	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


};
#endif