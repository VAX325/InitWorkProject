#pragma once
#ifndef W_WEBSESSION_H
#define W_WEBSESSION_H

class W_WebSession final : public WebToolkit::HttpSessionObject
{
public:
	W_WebSession();
	virtual ~W_WebSession() = default;

	clock_t W_lastAccessedTime;

	bool is_logged_in;
	bool is_admin;

	int curr_page;
	int last_page;
};

#endif