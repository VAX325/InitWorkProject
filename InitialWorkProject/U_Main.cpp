#include "P_HEADER.h"
#include "W_WebServer.h"

using namespace std;
using namespace CoreToolkit;
using namespace WebToolkit;

#ifdef WIN32
BOOL CALLBACK U_ConsoleCloseHandler(DWORD ctrlType)
{
	switch (ctrlType)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
		Environment::terminated = true;
		Sleep(10000); //Wait 10 sec for app close correct
		return TRUE;
		break;
	default:
		break;
	}

	return FALSE;
}

#endif

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

	W_WebServer* srv = new W_WebServer();
	srv->Run();

	while(!Environment::CheckForTermination())
	{
		/*std::string data;
		std::cin >> data;

		if(data == "exit")
		{
			LOG(LogLevel::LogVerbose) << "Exiting...";
			break;
		}
		else
		{
			LOG(LogLevel::LogError) << "Can't find command named \"" + data + "\"!";
		}*/
	}

	srv->Stop();

	return 0;
}