#pragma once
#ifndef U_DATAPARSER_H
#define U_DATAPARSER_H

class U_DataParser final
{
public:
	static U_DataParser* DataParser();

	const char* GetLoginPage();
	const char* GetMainPage();
	const char* GetMainAdminPage();

private:
	static U_DataParser* U_parser;

	U_DataParser();
	U_DataParser(const U_DataParser&) = delete;
	U_DataParser& operator=(U_DataParser&) = delete;

	const char* cwd;
};

#endif