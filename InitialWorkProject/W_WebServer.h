#pragma once
#ifndef WEBSERVER_H
#define WEBSERVER_H

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

	void Handle(WebToolkit::HttpServerContext* cntx) {}

private:
	WebToolkit::Server* server;
	WebToolkit::URIDispatcher dispatcher;

	void Login(WebToolkit::HttpServerContext* context);
	void Index(WebToolkit::HttpServerContext* context);
	void ChangeData(WebToolkit::HttpServerContext* context);
};

#endif