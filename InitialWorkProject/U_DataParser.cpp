#include "P_HEADER.h"
#include "U_DataParser.h"

U_DataParser* U_DataParser::U_parser = nullptr;

U_DataParser* U_DataParser::DataParser()
{
	if (!U_parser)
		U_parser = new U_DataParser();
	return U_parser;
}

U_DataParser::U_DataParser()
{
	cwd = U_GetCWD();
}

const char* U_DataParser::GetLoginPage()
{
	const std::string data_file = std::string(cwd) + "\\data\\login.html";

	const std::string file_data_str = CoreToolkit::FileUtils::ReadFile(data_file);

	const char* file_data = new char[file_data_str.size()];
	strcpy((char*)file_data, file_data_str.c_str());

	return file_data;
}

const char* U_DataParser::GetMainPage()
{
	const std::string data_file = std::string(cwd) + "\\data\\main.html";

	const std::string file_data_str = CoreToolkit::FileUtils::ReadFile(data_file);

	const char* file_data = new char[file_data_str.size()];
	strcpy((char*)file_data, file_data_str.c_str());

	return file_data;
}

const char* U_DataParser::GetMainAdminPage()
{
	const std::string data_file = std::string(cwd) + "\\data\\main_admin.html";

	const std::string file_data_str = CoreToolkit::FileUtils::ReadFile(data_file);

	const char* file_data = new char[file_data_str.size()];
	strcpy((char*)file_data, file_data_str.c_str());

	return file_data;
}

#if WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

const char* U_GetCWD()
{
#ifdef _WIN32
	return _getcwd(0, 0);
#else
	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd)) != NULL) 
	{
		return cwd;
	}

	return "";
#endif
}
