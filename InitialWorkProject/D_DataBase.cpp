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
	int colums = 0;
	std::vector<std::string> columns_data;
	std::vector<std::string> columns_result;
};

static D_UserData NullPtrUser = D_UserData();

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

	struct_mapping::reg(&D_UserSelectData::ID, "id");
	struct_mapping::reg(&D_UserSelectData::Names, "names");

	NullPtrUser.ID = -228;
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
			VALUES (0, 1, 'ADMIN ADMIN ADMIN', '2022-04-18', 'Satelite Softlabs', 'SYS ADM', 'ADMIN', 'admin', 'admin')\
			"
		);

		//Create test
		D_Exec(
			"INSERT INTO RootDB (ID, ADM, NAMES, DOB, ORGNAME, POS, MAIL, LOG, PASS)\
			VALUES (1, 0, 'TEST TEST TEST', '2022-04-18', 'Satelite Softlabs', 'SYS TST', 'TEST', 'test', 'test')\
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

#include <assert.h>
#include <cstdarg>
extern const char* format(const char* format, ...)
{
	size_t size = strlen(format) + 512;

	char* Buff = (char*)malloc(size);
	va_list va;
	va_start(va, format);
	vsnprintf(Buff, size, format, va);
	va_end(va);

	assert(Buff && "MALLOC ERROR!");

	const size_t str_size = strlen(Buff);
	//Buff = (char*)realloc(Buff, str_size);
	//Buff[str_size] = '\0';
	void* buff_ptr = realloc(Buff, str_size);
	assert(buff_ptr && "REALOC ERROR!");

	Buff = (char*)buff_ptr;
	Buff[str_size] = '\0';

	return Buff;
}

int D_DataBase::D_UserAuthNew(const char* login, const char* pass, int * user_id_ptr)
{
	const char* buff = format(
		"SELECT LOG, PASS, ADM, ID FROM RootDB\
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
			*user_id_ptr = atoi(sql_return->columns_data[3].c_str());

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
	const char* buff = "SELECT ID FROM RootDB WHERE ID = (SELECT MAX(ID) FROM RootDB)";

	SQL_RETURN_STRUCT* sql_return = (SQL_RETURN_STRUCT*)D_Exec(buff);

	if (!sql_return->colums)
		return 0;

	int id = atoi(sql_return->columns_data[0].c_str()) + 1;

	users = new D_UserData[id];

	for (int i = 0; i != id; i++)
	{
		auto user = D_GetUserByID(i);
		users[i] = user;
	}

	return id;
}

size_t D_DataBase::D_GetUsersForList(D_UserSelectData*& users)
{
	const char* buff = "SELECT ID FROM RootDB WHERE ID = (SELECT MAX(ID) FROM RootDB)";

	SQL_RETURN_STRUCT* sql_return = (SQL_RETURN_STRUCT*)D_Exec(buff);

	if (!sql_return->colums)
		return 0;

	int id = atoi(sql_return->columns_data[0].c_str()) + 1;

	users = new D_UserSelectData[id];

	for (int i = 0; i != id; i++)
	{
		auto user = D_GetUserByID(i);
		if (user.ID == NullPtrUser.ID)
			continue;

		users[i].ID = user.ID;
		users[i].Names = user.Names;
	}

	return id;
}

D_UserData D_DataBase::D_GetUserByID(int id)
{
	const char* buff = format("SELECT * FROM RootDB\
						WHERE ID = %i", id);
	const int colums_count = 8;

	SQL_RETURN_STRUCT* sql_return = (SQL_RETURN_STRUCT*)D_Exec(buff);
	if (sql_return)
	{
		if (sql_return->colums <= 0)
		{
			return NullPtrUser;
		}
		else
		{
			D_UserData user{};
			user.ID = atoi(sql_return->columns_data[0].c_str());
			user.Adm = atoi(sql_return->columns_data[1].c_str());
			user.Names = sql_return->columns_data[2];
			user.DateOfBirth = sql_return->columns_data[3];
			user.OrgName = sql_return->columns_data[4];
			user.Position = sql_return->columns_data[5];
			user.E_Mail = sql_return->columns_data[6];
			user.Login = sql_return->columns_data[7];
			user.Password = sql_return->columns_data[8];

			return user;
		}
	}

	return NullPtrUser;
}

void D_DataBase::D_ChangeUserDataById(int id, std::vector<std::string> user_data)
{
	const char* buff = format("UPDATE RootDB\
						SET DOB = \"%s\", \
							ORGNAME = \"%s\",\
							POS = \"%s\",\
							NAMES = \"%s\",\
							MAIL = \"%s\",\
							LOG = \"%s\",\
							PASS = \"%s\"\
						WHERE ID = %i", user_data[0].c_str(), 
		user_data[1].c_str(), 
		user_data[2].c_str(), 
		user_data[3].c_str(), 
		user_data[4].c_str(), 
		user_data[5].c_str(), 
		user_data[6].c_str(), 
		id);

	SQL_RETURN_STRUCT* sql_return = (SQL_RETURN_STRUCT*)D_Exec(buff);
	if (sql_return)
	{

	}
}

void D_DataBase::D_AddUser(D_UserData data)
{
	const char* buff = "SELECT ID FROM RootDB WHERE ID = (SELECT MAX(ID) FROM RootDB)";

	SQL_RETURN_STRUCT* sql_return = (SQL_RETURN_STRUCT*)D_Exec(buff);

	if (!sql_return->colums)
		return;

	int id = atoi(sql_return->columns_data[0].c_str()) + 1;

	const char* buff2 = "INSERT INTO RootDB (ID, ADM, NAMES, DOB, ORGNAME, POS, MAIL, LOG, PASS)\
			VALUES (%i, %i, '%s', '%s', '%s', '%s', '%s', '%s', '%s')\
			";

	const char* res = format(buff2, 
		id, 
		data.Adm ? 1 : 0, 
		data.Names.c_str(), 
		data.DateOfBirth.c_str(), 
		data.OrgName.c_str(), 
		data.Position.c_str(), 
		data.E_Mail.c_str(), 
		data.Login.c_str(), 
		data.Password.c_str());

	D_Exec(res);
}

void D_DataBase::D_DeleteUser(int id)
{
	D_Exec(format("DELETE FROM RootDB WHERE ID = %i", id));
}
