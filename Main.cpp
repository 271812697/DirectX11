#include "GameApp.h"


HANDLE hStdin;

HANDLE hStdout;
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE prevInstance,
	_In_ LPSTR cmdLine, _In_ int showCmd)
{
	// 这些参数不使用
	UNREFERENCED_PARAMETER(prevInstance);
	UNREFERENCED_PARAMETER(cmdLine);
	UNREFERENCED_PARAMETER(showCmd);
	// 允许在Debug版本进行运行时内存分配和泄漏检测
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif


	AllocConsole();
	hStdin = GetStdHandle(STD_INPUT_HANDLE);
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	char buffer[100] = "hello world";
	DWORD length = strlen(buffer);
	WriteFile(hStdout, buffer, length, &length, 0);

	GameApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}




