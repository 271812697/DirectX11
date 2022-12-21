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


};
#endif