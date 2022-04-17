#include "P_HEADER.h"
#include "W_WebServer.h"
#include "W_WebSession.h"
#include "U_DataParser.h"
#include "D_DataBase.h"

//#include <DuckX/duckx.hpp>

using namespace CoreToolkit;
using namespace WebToolkit;

W_WebServer::W_WebServer()
{
	server = new WebToolkit::Server();

	dispatcher.AddMapping("/login", HttpGet, new HttpHandlerConnector<W_WebServer>(this, &W_WebServer::Login), true);
	dispatcher.AddMapping("/index", HttpGet, new HttpHandlerConnector<W_WebServer>(this, &W_WebServer::Index), true);
	dispatcher.AddMapping("/logout", HttpGet, new HttpHandlerConnector<W_WebServer>(this, &W_WebServer::LogOut), true);
	dispatcher.AddMapping("/change_data", HttpPost, new HttpHandlerConnector<W_WebServer>(this, &W_WebServer::ChangeData), true);

	dispatcher.AddMapping("/construct_full_rest", HttpPost, new HttpHandlerConnector<W_WebServer>(this, &W_WebServer::ConstructFullRest), true);
	dispatcher.AddMapping("/construct_lite_rest", HttpPost, new HttpHandlerConnector<W_WebServer>(this, &W_WebServer::ConstructLiteRest), true);
	dispatcher.AddMapping("/construct_job_quit", HttpPost, new HttpHandlerConnector<W_WebServer>(this, &W_WebServer::ConstructJobQuit), true);

	dispatcher.AddMapping("/get_request", HttpGet, new HttpHandlerConnector<W_WebServer>(this, &W_WebServer::GetRequest), true);
	dispatcher.AddMapping("/redirect", HttpGet, new Redirector("/index"), true);
	dispatcher.SetDefaultHandler("/redirect");
	server->RegisterHandler(&dispatcher);

	D_DataBase::DataBase()->D_Open("Test.sqlitedb");
}

W_WebServer::~W_WebServer()
{
	delete server;
}

void W_WebServer::Run()
{
	server->Run();
}

void W_WebServer::Stop()
{
	server->Terminate();
	D_DataBase::DataBase()->D_Close();
}

void W_WebServer::Handle(WebToolkit::HttpServerContext* context)
{
	//Empty :( 'cause it don't works (custom dispatcher)
}

const char* U_GetPageByEnum(W_PAGES page)
{
	switch (page)
	{
	case W_PAGES::LOGIN:
		return U_DataParser::DataParser()->GetLoginPage();
		break;
	case W_PAGES::INDEX:
		return U_DataParser::DataParser()->GetMainPage();
		break;
	case W_PAGES::INDEX_ADMIN:
		return U_DataParser::DataParser()->GetMainAdminPage();
		break;
	default:
		return U_DataParser::DataParser()->GetLoginPage();
		break;
	}
}

constexpr auto MAX_TIME = 120; //2 minutes;

inline bool W_EachPage(WebToolkit::HttpServerContext* cntx)
{
	cntx->responseHeader = HttpResponseHeader();

	bool BrandNew = false;
	if (cntx->sessionObject == 0)
	{
		cntx->StartSession(new W_WebSession());
		BrandNew = true;
	}

	W_WebSession* sessionObj = static_cast<W_WebSession*>(cntx->sessionObject);
	clock_t currTime = clock();

	if (BrandNew)
	{
		sessionObj->W_lastAccessedTime = clock();
	}

#ifdef MAKE_DISCONNECT
	const clock_t max_time = CLOCKS_PER_SEC * MAX_TIME;
	if (currTime - sessionObj->W_lastAccessedTime > max_time)
	{
		sessionObj->last_page = sessionObj->curr_page;
		sessionObj->curr_page = (int)W_PAGES::LOGIN;
		sessionObj->is_logged_in = false;
		sessionObj->W_lastAccessedTime = clock();
	}
	else
	{
		sessionObj->W_lastAccessedTime = clock();
	}
#endif

	return sessionObj->is_logged_in;
}

void W_WebServer::Login(WebToolkit::HttpServerContext* context)
{
	W_EachPage(context);
	W_WebSession* sessionObj = static_cast<W_WebSession*>(context->sessionObject);
	
	if(sessionObj->is_logged_in)
	{
		context->Redirect("/index");
	}
	else
	{
		if (context->parameters.size() > 0)
		{
			if (sessionObj->curr_page == (int)W_PAGES::LOGIN)
			{
				std::string username = context->parameters["username"];
				std::string password = context->parameters["password"];
				LOG(LogLevel::LogVerbose) << username;
				LOG(LogLevel::LogVerbose) << password;

				int result = D_DataBase::DataBase()->D_UserAuthNew(username.c_str(), password.c_str(), &sessionObj->user_id);

				if (result > 0)
				{
					if (sessionObj->last_page != sessionObj->curr_page)
					{
						sessionObj->curr_page = sessionObj->last_page;
						sessionObj->last_page = (int)W_PAGES::LOGIN;
					}
					else
					{
						sessionObj->last_page = sessionObj->curr_page;
						if (result == 2)
							sessionObj->curr_page = (int)W_PAGES::INDEX_ADMIN;
						else
							sessionObj->curr_page = (int)W_PAGES::INDEX;
					}
					sessionObj->is_logged_in = true;
					if(result == 2)
						sessionObj->is_admin = true;
				}
				else
				{
					sessionObj->is_logged_in = false;
				}
			}
		}

		if (sessionObj->is_logged_in)
			context->Redirect("/index");
		else
			context->responseBody << U_GetPageByEnum(W_PAGES::LOGIN);
	}
}

extern const char* format(const char* format, ...);

void W_WebServer::Index(WebToolkit::HttpServerContext* context)
{
	if (W_EachPage(context))
	{
		W_WebSession* sessionObj = static_cast<W_WebSession*>(context->sessionObject);

		std::string return_page = sessionObj->is_admin ? U_GetPageByEnum(W_PAGES::INDEX_ADMIN) : U_GetPageByEnum(W_PAGES::INDEX);

		const char* LKOpen = "display:none;";
		const char* RequestsOpen = "display:none;";
		const char* DBOpen = "display:none;";

		switch (sessionObj->open_page)
		{
		case 0:
			LKOpen = "display:block;";
			RequestsOpen = "display:none;";
			DBOpen = "display:none;";
			break;
		case 1:
			LKOpen = "display:none;";
			RequestsOpen = "display:block;";
			DBOpen = "display:none;";
			break;
		case 2:
			LKOpen = "display:none;";
			RequestsOpen = "display:none;";
			DBOpen = "display:block;";
			break;
		default:
			LKOpen = "display:none;";
			RequestsOpen = "display:none;";
			DBOpen = "display:none;";
			break;
		}

		sessionObj->open_page = -1;

		return_page = format(return_page.c_str(), LKOpen, RequestsOpen, DBOpen);

		if(sessionObj->is_admin)
		{
			//Get all user data
			{
				auto it = context->parameters.find("GTUSRDATA");
				if (it != context->parameters.end())
				{
					LOG(LogLevel::LogVerbose) << "Adm is requesting users data!";
					D_UserData* users = nullptr;
					size_t users_count = D_DataBase::DataBase()->D_GetAllUsers(*&users);

					std::string TheGreatBuffer = "{\"users\": [\n";
					for (int i = 0; i != users_count; i++)
					{
						if (i > 0)
						{
							TheGreatBuffer += ",\n";
						}

						std::ostringstream out_json_data;
						struct_mapping::map_struct_to_json(users[i], out_json_data, "  ");

						TheGreatBuffer += out_json_data.str();
					}
					TheGreatBuffer += "\n]}";

					context->responseBody << TheGreatBuffer;

					delete[] users;
					return;
				}

			}
			//Get count and names
			{
				auto it = context->parameters.find("UPDUSRS");
				if (it != context->parameters.end())
				{
					D_UserSelectData* users = nullptr;
					size_t users_count = D_DataBase::DataBase()->D_GetUsersForList(*&users);

					std::string TheGreatBuffer = "{\"users\": [\n";
					for (int i = 0; i != users_count; i++)
					{
						if (i > 0)
						{
							TheGreatBuffer += ",\n";
						}

						std::ostringstream out_json_data;
						struct_mapping::map_struct_to_json(users[i], out_json_data, "  ");

						TheGreatBuffer += out_json_data.str();
					}
					TheGreatBuffer += "\n]}";

					context->responseBody << TheGreatBuffer;

					delete[] users;
					return;
				}

			}
		}

		//Get concrete user data
		{
			auto it = context->parameters.find("GTCONUSRDATA");
			if (it != context->parameters.end())
			{
				//LOG(LogLevel::LogVerbose) << "Adm is requesting concrete user data!";

				int user_id = -1;
				if (it->second == "user")
					user_id = sessionObj->user_id;
				else
					user_id = atoi(it->second.c_str());

				if (!sessionObj->is_admin && sessionObj->user_id != user_id)
				{
					return;
				}

				auto user = D_DataBase::DataBase()->D_GetUserByID(user_id);

				if (sessionObj->is_admin)
				{
					sessionObj->admin_change_data_id = user_id;
				}

				std::string TheGreatBuffer = "{\"users\": [\n";
				if (user.Login != "")
				{
					std::ostringstream out_json_data;
					struct_mapping::map_struct_to_json(user, out_json_data, "  ");

					TheGreatBuffer += out_json_data.str();
					TheGreatBuffer += "\n]}";

					context->responseBody << TheGreatBuffer;
				}
				return;
			}
		}

		context->responseBody << return_page;
	}
	else
	{
		context->Redirect("/login");
	}
}

void W_WebServer::LogOut(WebToolkit::HttpServerContext* context)
{
	W_WebSession* sessionObj = static_cast<W_WebSession*>(context->sessionObject);

	if (W_EachPage(context))
	{
		sessionObj->W_lastAccessedTime = 0;

		sessionObj->is_logged_in = false;
		sessionObj->is_admin = false;

		sessionObj->curr_page = 0;
		sessionObj->last_page = 0;

		sessionObj->open_page = -1;

		sessionObj->user_id = -1;

		sessionObj->admin_change_data_id = 0;

		context->Redirect("/login");
	}
	else
	{
		context->Redirect("/login");
	}
}

//Push on LK
void W_WebServer::ChangeData(WebToolkit::HttpServerContext* context)
{
	W_WebSession* sessionObj = static_cast<W_WebSession*>(context->sessionObject);

	if (W_EachPage(context))
	{
		if(sessionObj->is_admin)
		{
			LOG(LogVerbose) << "Admin is requesting change of user data";

			std::vector<std::string> user_changable_data;
			user_changable_data.push_back(context->parameters["user_birthdate"]);
			user_changable_data.push_back(context->parameters["user_workname"]);
			user_changable_data.push_back(context->parameters["user_position"]); 
			user_changable_data.push_back(context->parameters["user_names"]);
			user_changable_data.push_back(context->parameters["user_email"]);
			user_changable_data.push_back(context->parameters["user_login"]);
			user_changable_data.push_back(context->parameters["user_password"]);

			int id = sessionObj->admin_change_data_id;

			D_DataBase::DataBase()->D_ChangeUserDataById(id, user_changable_data);
		}

		sessionObj->open_page = 0; //For lk
		context->Redirect("/index");
	}
	else
	{
		context->Redirect("/login");
	}
}

inline void U_ReplaceWith(std::string& format, std::string placeholder, std::string to_replace)
{
	size_t res = format.find(placeholder, 0);
	if (res != std::string::npos)
	{
		while (res != std::string::npos)
		{
			format.replace(res, placeholder.size(), to_replace);
			res = format.find(placeholder, res + to_replace.size());
		}
	}
}

#include <chrono>
#include <fstream>

void W_WebServer::ConstructFullRest(WebToolkit::HttpServerContext* context)
{
	W_WebSession* sessionObj = static_cast<W_WebSession*>(context->sessionObject);

	if (W_EachPage(context))
	{
		const std::string path = std::string(U_GetCWD()) + "\\data\\requests\\REQUEST_FULL\\request_full.html";
		std::string data = CoreToolkit::FileUtils::ReadFile(path);
		
		std::string pos = context->parameters["position"];
		std::string fullnames = context->parameters["fullnames"];
		std::string start_date = context->parameters["start_date"];
		std::string end_date = context->parameters["end_date"];
		std::string full_count = context->parameters["full_count"];
		std::string full_calendar = context->parameters["full_calendar"];
		std::string cur_date = context->parameters["cur_date"];
		std::string short_names = context->parameters["short_names"];

		U_ReplaceWith(data, "{position}", pos);
		U_ReplaceWith(data, "{names}", fullnames);
		U_ReplaceWith(data, "{date_start}", start_date);
		U_ReplaceWith(data, "{date_end}", end_date);
		U_ReplaceWith(data, "{count}", full_count);
		U_ReplaceWith(data, "{year}", full_calendar);
		U_ReplaceWith(data, "{current_date}", cur_date);
		U_ReplaceWith(data, "{short_names}", short_names);

		auto end = std::chrono::system_clock::now();
		std::time_t end_time = std::chrono::system_clock::to_time_t(end);

		std::string name = "request_full_" + fullnames + std::ctime(&end_time) + ".html";
		std::replace(name.begin(), name.end(), ':', '_');
		std::replace(name.begin(), name.end(), ' ', '_');
		std::replace(name.begin(), name.end(), '\n', '_');

		std::string _path = std::string(U_GetCWD()) + "\\data\\requests\\" + name;

		std::ofstream outfile(_path, std::fstream::out);
		outfile.write(" ", 1);
		outfile.close();

		CoreToolkit::File* file = new CoreToolkit::File(_path, true);
		file->Write(data);
		delete file;

		sessionObj->open_page = 1;
		context->responseBody << name;//data;
		//context->Redirect("/index");
	}
	else
	{
		context->Redirect("/login");
	}
}

void W_WebServer::ConstructLiteRest(WebToolkit::HttpServerContext* context)
{
	W_WebSession* sessionObj = static_cast<W_WebSession*>(context->sessionObject);

	if (W_EachPage(context))
	{
		const std::string path = std::string(U_GetCWD()) + "\\data\\requests\\REQUEST_LITE\\request_lite.html";
		std::string data = CoreToolkit::FileUtils::ReadFile(path);

		std::string pos = context->parameters["position"];
		std::string fullnames = context->parameters["fullnames"];
		std::string start_date = context->parameters["start_date"];
		std::string end_date = context->parameters["end_date"];
		std::string full_count = context->parameters["full_count"];
		std::string cur_date = context->parameters["cur_date"];
		std::string short_names = context->parameters["short_names"];

		U_ReplaceWith(data, "{position}", pos);
		U_ReplaceWith(data, "{names}", fullnames);
		U_ReplaceWith(data, "{date_start}", start_date);
		U_ReplaceWith(data, "{date_end}", end_date);
		U_ReplaceWith(data, "{count}", full_count);
		U_ReplaceWith(data, "{current_date}", cur_date);
		U_ReplaceWith(data, "{short_names}", short_names);

		auto end = std::chrono::system_clock::now();
		std::time_t end_time = std::chrono::system_clock::to_time_t(end);

		std::string name = "request_lite_" + fullnames + std::ctime(&end_time) + ".html";
		std::replace(name.begin(), name.end(), ':', '_');
		std::replace(name.begin(), name.end(), ' ', '_');
		std::replace(name.begin(), name.end(), '\n', '_');

		std::string _path = std::string(U_GetCWD()) + "\\data\\requests\\" + name;

		std::ofstream outfile(_path, std::fstream::out);
		outfile.write(" ", 1);
		outfile.close();

		CoreToolkit::File* file = new CoreToolkit::File(_path, true);
		file->Write(data);
		delete file;

		sessionObj->open_page = 1;
		context->responseBody << name;//data;
		//context->Redirect("/index");
	}
	else
	{
		context->Redirect("/login");
	}
}

void W_WebServer::ConstructJobQuit(WebToolkit::HttpServerContext* context)
{
	W_WebSession* sessionObj = static_cast<W_WebSession*>(context->sessionObject);

	if (W_EachPage(context))
	{
		const std::string path = std::string(U_GetCWD()) + "\\data\\requests\\REQUEST_JOBQUIT\\request_job_quit.html";
		std::string data = CoreToolkit::FileUtils::ReadFile(path);

		std::string pos = context->parameters["position"];
		std::string fullnames = context->parameters["fullnames"];
		std::string date = context->parameters["date"];
		std::string cur_date = context->parameters["cur_date"];
		std::string short_names = context->parameters["short_names"];

		U_ReplaceWith(data, "{position}", pos);
		U_ReplaceWith(data, "{names}", fullnames);
		U_ReplaceWith(data, "{date}", date);
		U_ReplaceWith(data, "{current_date}", cur_date);
		U_ReplaceWith(data, "{short_names}", short_names);

		auto end = std::chrono::system_clock::now();
		std::time_t end_time = std::chrono::system_clock::to_time_t(end);

		std::string name = "request_lite_" + fullnames + std::ctime(&end_time) + ".html";
		std::replace(name.begin(), name.end(), ':', '_');
		std::replace(name.begin(), name.end(), ' ', '_');
		std::replace(name.begin(), name.end(), '\n', '_');

		std::string _path = std::string(U_GetCWD()) + "\\data\\requests\\" + name;

		std::ofstream outfile(_path, std::fstream::out);
		outfile.write(" ", 1);
		outfile.close();

		CoreToolkit::File* file = new CoreToolkit::File(_path, true);
		file->Write(data);
		delete file;

		sessionObj->open_page = 1;
		context->responseBody << name;//data;
		//context->Redirect("/index");
	}
	else
	{
		context->Redirect("/login");
	}
}

void W_WebServer::GetRequest(WebToolkit::HttpServerContext* context)
{
	if (W_EachPage(context))
	{
		if(context->parameters.size() != 1)
		{
			context->Redirect("/index");
		}

		std::string RequestFileToGet = context->parameters["GTREQUEST"];
		if (RequestFileToGet == "")
		{
			context->Redirect("/index");
		}

		const std::string path = std::string(U_GetCWD()) + "\\data\\requests\\" + RequestFileToGet;
		std::string data = CoreToolkit::FileUtils::ReadFile(path);

		context->responseBody << data;
	}
	else
	{
		context->Redirect("/login");
	}
}
