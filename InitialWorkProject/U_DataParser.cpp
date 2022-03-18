#include "P_HEADER.h"
#include "U_DataParser.h"

#ifdef _WIN32
#include <direct.h>
#endif

U_DataParser* U_DataParser::U_parser = nullptr;

U_DataParser* U_DataParser::DataParser()
{
	if (!U_parser)
		U_parser = new U_DataParser();
	return U_parser;
}

U_DataParser::U_DataParser()
{
#ifdef _WIN32
	cwd = _getcwd(0, 0);
#else
	cwd = "none"; //Get current work dir on linux/os x
#endif
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
