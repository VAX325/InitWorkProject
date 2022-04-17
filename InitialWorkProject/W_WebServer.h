#pragma once
#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "W_WebDispatcher.h"

enum class W_PAGES : int
{
	LOGIN,
	INDEX,
	INDEX_ADMIN,
};

class W_WebServer final : public WebToolkit::HttpHandler
{
public:
	W_WebServer();
	~W_WebServer();

	void Run();
	void Stop();

	void Handle(WebToolkit::HttpServerContext* context);

private:
	WebToolkit::Server* server;
	W_WebDispatcher dispatcher;

	void Login(WebToolkit::HttpServerContext* context);
	void Index(WebToolkit::HttpServerContext* context);
	void LogOut(WebToolkit::HttpServerContext* context);
	void ChangeData(WebToolkit::HttpServerContext* context);

	void ConstructFullRest(WebToolkit::HttpServerContext* context);
	void ConstructLiteRest(WebToolkit::HttpServerContext* context);
	void ConstructJobQuit(WebToolkit::HttpServerContext* context);

	void GetRequest(WebToolkit::HttpServerContext* context);

	void PushUser(WebToolkit::HttpServerContext* context);
};

#endif