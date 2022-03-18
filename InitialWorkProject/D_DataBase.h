#pragma once
#ifndef D_DATABASE_H
#define D_DATABASE_H

struct D_UserData final
{
	int ID;
	int Adm;

	std::string DateOfBirth;
	std::string OrgName;
	std::string Position;
	std::string E_Mail;

	std::string Login;
	std::string Password;
};

class D_DataBase final
{
public:
	static D_DataBase* DataBase()
	{
		if (!D_database)
			D_database = new D_DataBase();
		return D_database;
	}
	~D_DataBase();

	bool D_Open(const char* db_name);
	bool D_Close();

	//For unexpected use. Pls, if your task has function, do not use this
	void* D_Exec(const char* REQUEST);

	/*
	Returns -1 if internal error
	Returns 0 if invalid data
	Returns 1 if user is passed
	Retruns 2 if user is admin
	*/
	int D_UserAuthNew(const char* login, const char* pass);
	//Do not use, only as history
	int D_UserAuth(const char* login, const char* pass);

	//Only admin access
	size_t D_GetAllUsers(D_UserData*& users);

private:
	static D_DataBase* D_database;
	
	D_DataBase();
	D_DataBase(const D_DataBase&) = delete;
	D_DataBase& operator=(D_DataBase&) = delete;

	void* D_InnerData;
};

#endif