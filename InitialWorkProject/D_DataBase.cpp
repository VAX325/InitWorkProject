#include "P_HEADER.h"
#include "D_DataBase.h"
#include <SQLite/sqlite3.h>

D_DataBase* D_DataBase::D_database = nullptr;

struct D_DataBase_Internal
{
	sqlite3* D_db;
	char* err;
};

//Gets internal data
#define GetInternalData static_cast<D_DataBase_Internal*>(D_InnerData)

struct SQL_RETURN_STRUCT
{
	int colums;
	std::vector<std::string> columns_data;
	std::vector<std::string> columns_result;
};

D_DataBase::D_DataBase()
{
	D_InnerData = new D_DataBase_Internal();

	struct_mapping::reg(&D_UserData::ID, "id");
	struct_mapping::reg(&D_UserData::Adm, "adm");
	struct_mapping::reg(&D_UserData::Names, "names");
	struct_mapping::reg(&D_UserData::DateOfBirth, "dob");
	struct_mapping::reg(&D_UserData::OrgName, "orgname");
	struct_mapping::reg(&D_UserData::Position, "pos");
	struct_mapping::reg(&D_UserData::E_Mail, "email");
	struct_mapping::reg(&D_UserData::Login, "login");
	struct_mapping::reg(&D_UserData::Password, "pass");
}

D_DataBase::~D_DataBase()
{
	D_Close();
	delete (D_DataBase_Internal*)D_InnerData;
}

bool D_DataBase::D_Open(const char* db_name)
{
	int result = sqlite3_open(db_name, &GetInternalData->D_db);
	if(result != SQLITE_OK)
	{
		LOG(CoreToolkit::LogError) << "Can't open data base! Reason: " << sqlite3_errmsg(GetInternalData->D_db);
		return false;
	}

	SQL_RETURN_STRUCT* obj = (SQL_RETURN_STRUCT*)D_Exec("SELECT name FROM sqlite_master WHERE type='table' AND name='RootDB'");
	if(obj->colums != 1)
	{
		LOG(CoreToolkit::LogWarning) << "Can't find DB. Creating new.";
	}

	D_Exec(
		"CREATE TABLE IF NOT EXISTS RootDB(\
		ID INTEGER PRIMARY KEY UNIQUE,\
		ADM INTEGER NOT NULL,\
		NAMES TEXT NOT NULL,\
		DOB TEXT NOT NULL,\
		ORGNAME TEXT NOT NULL,\
		POS TEXT NOT NULL,\
		MAIL TEXT NOT NULL UNIQUE,\
		LOG TEXT NOT NULL UNIQUE,\
		PASS TEXT NOT NULL\
		)"
	);

	if(obj->colums != 1)
	{
		//Create admin
		D_Exec(
			"INSERT INTO RootDB (ID, ADM, NAMES, DOB, ORGNAME, POS, MAIL, LOG, PASS)\
			VALUES (0, 1, 'ADMIN ADMIN ADMIN', 'ADMIN', 'Satelite Softlabs', 'SYS ADM', 'ADMIN', 'admin', 'admin')\
			"
		);

		//Create test
		D_Exec(
			"INSERT INTO RootDB (ID, ADM, NAMES, DOB, ORGNAME, POS, MAIL, LOG, PASS)\
			VALUES (1, 0, 'TEST TEST TEST', 'TEST', 'Satelite Softlabs', 'SYS TST', 'TEST', 'test', 'test')\
			"
		);
	}

	delete obj;
	return true;
}

bool D_DataBase::D_Close()
{
	int result = sqlite3_close(GetInternalData->D_db);
	if(result != SQLITE_OK)
	{
		LOG(CoreToolkit::LogError) << "Can't close data base! Reason: " << sqlite3_errmsg(GetInternalData->D_db);
		return false;
	}

	return true;
}

//Return non-zero for invoke error
int callback(void* sql_obj, int columns, char** columns_data, char** columns_result)
{
	//printf("Executed!\n");
	static_cast<SQL_RETURN_STRUCT*>(sql_obj)->colums = columns;
	static_cast<SQL_RETURN_STRUCT*>(sql_obj)->columns_data.reserve(columns);
	static_cast<SQL_RETURN_STRUCT*>(sql_obj)->columns_result.reserve(columns);

	for (int i = 0; i != columns; i++)
	{
		static_cast<SQL_RETURN_STRUCT*>(sql_obj)->columns_data.push_back(std::string(columns_data[i]));
		static_cast<SQL_RETURN_STRUCT*>(sql_obj)->columns_result.push_back(std::string(columns_result[i]));
	}

	return 0;
}

void* D_DataBase::D_Exec(const char* REQUEST)
{
	SQL_RETURN_STRUCT* obj = new SQL_RETURN_STRUCT;
	int result = sqlite3_exec(GetInternalData->D_db, REQUEST, callback, *&obj, &GetInternalData->err);
	if(result != SQLITE_OK)
	{
		LOG(CoreToolkit::LogError) << "Can't execute request! Reason: " << (GetInternalData->err != nullptr ? GetInternalData->err : sqlite3_errmsg(GetInternalData->D_db));
		sqlite3_free(GetInternalData->err);
		return nullptr;
	}

	return obj;
}

#include <cstdarg>
extern const char* format(const char* format, ...)
{
	size_t size = strlen(format) + 512;

	char* Buff = (char*)malloc(size);
	va_list va;
	va_start(va, format);
	vsnprintf(Buff, size, format, va);
	va_end(va);

	return Buff;
}

int D_DataBase::D_UserAuthNew(const char* login, const char* pass)
{
	const char* buff = format(
		"SELECT LOG, PASS, ADM FROM RootDB\
		WHERE LOG = '%s' AND PASS = '%s'",
		login, pass
		);

	SQL_RETURN_STRUCT* sql_return = (SQL_RETURN_STRUCT*)D_Exec(buff);
	if(sql_return)
	{
		if(sql_return->colums <= 0)
		{
			return 0;
		}
		else
		{
			int adm = atoi(sql_return->columns_data[2].c_str());
			return 1 + adm;
		}
	}
	else
	{
		LOG(CoreToolkit::LogError) << "BLYAT";
	}

	free((void*)buff);
	return -1;
}

size_t D_DataBase::D_GetAllUsers(D_UserData*& users)
{
	const char* buff = "SELECT * FROM RootDB";
	const int colums_count = 8;

	SQL_RETURN_STRUCT* sql_return = (SQL_RETURN_STRUCT*)D_Exec(buff);
	if (sql_return)
	{
		if (sql_return->colums <= 0)
		{
			return 0;
		}
		else
		{
			users = new D_UserData[sql_return->colums];
			using size = std::vector<std::string, std::allocator<std::string>>::size_type;

			size curr_user_shift = 0;
			const int user_count = static_cast<size>((sql_return->colums / colums_count) + 1);
			for(size i = 0; i != user_count; i++)
			{
				users[i] = { 0 };
				users[i].ID = atoi(sql_return->columns_data[curr_user_shift].c_str());
				users[i].Adm = atoi(sql_return->columns_data[curr_user_shift + 1].c_str());
				users[i].Names = sql_return->columns_data[curr_user_shift + 2];
				users[i].DateOfBirth = sql_return->columns_data[curr_user_shift + 3];
				users[i].OrgName = sql_return->columns_data[curr_user_shift + 4];
				users[i].Position = sql_return->columns_data[curr_user_shift + 5];
				users[i].E_Mail = sql_return->columns_data[curr_user_shift + 6];
				users[i].Login = sql_return->columns_data[curr_user_shift + 7];
				users[i].Password = sql_return->columns_data[curr_user_shift + 8];
				curr_user_shift += 9;
			}

			return user_count;
		}
	}
	else
	{
		LOG(CoreToolkit::LogError) << "ERROR WHILE SEARCHG ALL USERS";
	}

	return 0;
}
