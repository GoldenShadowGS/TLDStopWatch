/*
Application Author: Michael Lundgren  (GoldenshadowGS)
*/

#include "PCH.h"
#include "Clock.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	std::wstring arg = lpCmdLine;
	int clientsize = 240;
	if (arg.size())
	{
		try
		{
			clientsize = std::stoi(arg);
		}
		catch (std::exception& e)
		{
			clientsize = 240;
			UNREFERENCED_PARAMETER(e);
		}
	}
	if (clientsize < 100)
		clientsize = 100;
	else if (clientsize > 800)
		clientsize = 800;
	std::unique_ptr<ClockApp> clockapp = std::make_unique<ClockApp>();
	if (!clockapp->Init(hInstance, nCmdShow, clientsize))
		return FALSE;

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

