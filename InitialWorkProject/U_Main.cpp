#include "P_HEADER.h"
#include "W_WebServer.h"

using namespace std;
using namespace CoreToolkit;
using namespace WebToolkit;

static bool U_Work = true;

#ifdef WIN32
BOOL CALLBACK U_ConsoleCloseHandler(DWORD ctrlType)
{
	switch (ctrlType)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
		U_Work = false;
		Sleep(10000); //Wait 10 sec for app close correct
		return TRUE;
		break;
	default:
		break;
	}

	return FALSE;
}

#endif

static void C_CommandParser(void)
{
	while (!Environment::CheckForTermination() && U_Work)
	{
		std::string data;
		std::cin >> data;

		if(data == "exit")
		{
			LOG(LogLevel::LogVerbose) << "Exiting...";
			U_Work = false;
			break;
		}
		else
		{
			LOG(LogLevel::LogError) << "Can't find command named \"" + data + "\"!";
		}
	}
}

int main(int argc, char** argv)
{
#ifdef WIN32
	//Making this for correct exit
	if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)U_ConsoleCloseHandler, TRUE) == FALSE)
	{
		LOG(LogLevel::LogVerbose) << "Can't install handler...";
		return -1;
	}
#endif

	setlocale(LC_ALL, ".UTF8");

	W_WebServer* srv = new W_WebServer();
	srv->Run();

	Thread::StartThread((ThreadProc)C_CommandParser, nullptr);
	while(U_Work){}

	srv->Stop();

	return 0;
}