#include "stdafx.h"

#include "App.h"

// Request to use discrete GPU
extern "C"
{
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

int main()
{
	try
	{
		dx::App app;
		return app.Run();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what();
		exit(EXIT_FAILURE);
	}
}