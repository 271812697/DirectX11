#include "GameApp.h"
#include"core/log.h"
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE prevInstance,
	_In_ LPSTR cmdLine, _In_ int showCmd)
{
	// ��Щ������ʹ��
	UNREFERENCED_PARAMETER(prevInstance);
	UNREFERENCED_PARAMETER(cmdLine);
	UNREFERENCED_PARAMETER(showCmd);
	// ������Debug�汾��������ʱ�ڴ�����й©���
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif


	AllocConsole();
	::core::Log::Init();
	CORE_INFO("start the log");
	GameApp theApp(hInstance);

	if (!theApp.Init())
		return 0;
	int res=theApp.Run();
	//::core::Log::Shutdown();
	return res;
}




