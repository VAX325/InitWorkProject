#include "P_HEADER.h"
#include "W_WebServer.h"
#include "W_WebSession.h"
#include "U_DataParser.h"
#include "D_DataBase.h"

using namespace CoreToolkit;
using namespace WebToolkit;

W_WebServer::W_WebServer()
{
	server = new WebToolkit::Server();

	dispatcher.AddMapping("/login", HttpGet, new HttpHandlerConnector<W_WebServer>(this, &W_WebServer::Login), true);
	dispatcher.AddMapping("/index", HttpGet, new HttpHandlerConnector<W_WebServer>(this, &W_WebServer::Index), true);
	dispatcher.AddMapping("/change_data", HttpPost, new HttpHandlerConnector<W_WebServer>(this, &W_WebServer::ChangeData), true);
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

				int result = D_DataBase::DataBase()->D_UserAuthNew(username.c_str(), password.c_str());

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

		auto it = context->parameters.find("OPENLK");
		if (it != context->parameters.end())
		{
			LKOpen = "display:block;";
			RequestsOpen = "display:none;";
			DBOpen = "display:none;";
		}

		return_page = format(return_page.c_str(), LKOpen, RequestsOpen, DBOpen);

		if (!sessionObj->is_admin)
		{
			//context->responseBody << U_GetPageByEnum(W_PAGES::INDEX);
		}
		else
		{
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
			{
				auto it = context->parameters.find("GTCONUSRDATA");
				if (it != context->parameters.end())
				{
					LOG(LogLevel::LogVerbose) << "Adm is requesting concrete user data!";
					D_UserData* users = nullptr;
					size_t users_count = D_DataBase::DataBase()->D_GetAllUsers(*&users);

					int user_id = atoi(it->second.c_str());

					std::string TheGreatBuffer = "{\"users\": [\n";
					for (int i = 0; i != users_count; i++)
					{
						if (users[i].ID != user_id)
						{
							continue;
						}

						std::ostringstream out_json_data;
						struct_mapping::map_struct_to_json(users[i], out_json_data, "  ");

						TheGreatBuffer += out_json_data.str();

						break;
					}
					TheGreatBuffer += "\n]}";

					context->responseBody << TheGreatBuffer;

					delete[] users;
					return;
				}
			}
		}

		context->responseBody << return_page;
	}
	else
	{
		context->Redirect("/login");
	}
}

void W_WebServer::ChangeData(WebToolkit::HttpServerContext* context)
{
	W_WebSession* sessionObj = static_cast<W_WebSession*>(context->sessionObject);

	if (W_EachPage(context))
	{
		if(sessionObj->is_admin)
		{
			LOG(LogVerbose) << "Admin is requesting change of user data";
		}

		context->Redirect("/index?OPENLK=true");
	}
	else
	{
		context->Redirect("/login");
	}
}