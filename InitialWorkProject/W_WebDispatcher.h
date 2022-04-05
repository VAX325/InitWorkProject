#pragma once
#ifndef WEBDISPATCHER_H
#define WEBDISPATCHER_H

class W_WebDispatcher : public WebToolkit::URIDispatcher
{
public:
	virtual void Handle(WebToolkit::HttpServerContext* context);
};

#endif