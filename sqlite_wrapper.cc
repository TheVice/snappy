/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 TheVice
 *
 */

#include "sqlite_wrapper.h"

#include "sqlite3.h"

#include <sstream>
#include <utility>
//#include <exception>
//#include <stdexcept>
#include <iostream>

SQLite::SQLite(const char* path) : db(nullptr)
{
	const auto rc = sqlite3_open(path, &db);

	if (SQLITE_OK != rc)
	{
		std::string message = "Can't open database: ";
		message += sqlite3_errmsg(db);
		message += ".";
		//
		sqlite3_close(db);
		db = nullptr;
		//
		//throw new std::runtime_error(message);
		std::cerr << message << std::endl;
	}
}

SQLite::~SQLite()
{
	if (nullptr != db)
	{
		sqlite3_close(db);
		db = nullptr;
	}
}

std::map<std::string, std::string>* response_ = nullptr;

static int callback(void*, int argc, char** argv, char** colName)
{
	if (response_)
	{
		for (int i = 0; i < argc; ++i)
		{
			response_->insert(
				std::make_pair<std::string, std::string>(
					colName[i], argv[i] ? argv[i] : ""));
		}
	}

	return 0;
}

bool SQLite::exec(
	const char* statement,
	std::map<std::string, std::string>* response)
{
	char* errMsg = nullptr;
	response_ = response;
	const auto rc = sqlite3_exec(db, statement, callback, 0, &errMsg);
	response_ = nullptr;

	if (SQLITE_OK != rc)
	{
		std::string message = "SQL error: ";
		message += errMsg;
		message += ".";
		//
		sqlite3_free(errMsg);
		errMsg = nullptr;
		//
		sqlite3_close(db);
		db = nullptr;
		//
		//throw new std::runtime_error(message);
		std::cerr << message << std::endl;
		return false;
	}

	return true;
}

std::map<std::string, std::string> response;

bool IsTableExists(SQLite& sql, const char* tableName)
{
	std::string statement =
		"SELECT name FROM sqlite_master" \
		" WHERE type LIKE \"table\"" \
		" AND name LIKE \"";
	statement += tableName;
	statement += "\"";
	response.clear();
	sql.exec(statement.c_str(), &response);
	return !response.empty();
}

static const std::string get_uncompressed_length = "GetUncompressedLength";

void CreateGetUncompressedLengthTable(SQLite& sql)
{
	std::string statement = "CREATE TABLE \"";
	statement += get_uncompressed_length;
	statement +="\" (" \
		" \"start\" TEXT NOT NULL UNIQUE," \
		" \"result\" INTEGER," \
		" \"return\" INTEGER NOT NULL" \
		");";
	sql.exec(statement.c_str(), nullptr);
}

bool GetFromGetUncompressedLength(SQLite& sql, const std::string& start, bool& return_, size_t& result_)
{
	std::string statement = "SELECT return, result FROM \"";
	statement += get_uncompressed_length;
	statement += "\" WHERE(start==\"";
	statement += start;
	statement += "\");";
	response.clear();
	sql.exec(statement.c_str(), &response);

	if (2 == response.size())
	{
		return_ = "1" == response["return"];

		if (return_)
		{
			const auto result_str = response["result"];
			std::stringstream sstream(result_str);
			sstream >> result_;
		}

		return true;
	}

	return false;
}

void SetToGetUncompressedLength(SQLite& sql, const std::string& start, bool return_, size_t result_)
{
	std::string statement = "INSERT INTO \"";
	statement += get_uncompressed_length;
	statement += "\"";
	statement += "(start, return, result) VALUES(\"";
	statement += start;
	statement += "\", ";
	statement += std::to_string(return_ ? 1 : 0);
	statement += ", ";
	statement += std::to_string(result_);
	statement += ");";
	sql.exec(statement.c_str(), nullptr);
}

static const std::string compress = "Compress";

void CreateCompressTable(SQLite& sql)
{
	std::string statement = "CREATE TABLE \"";
	statement += compress;
	statement += "\" (" \
		" \"input\" TEXT NOT NULL UNIQUE," \
		" \"output\" TEXT" \
		");";
	sql.exec(statement.c_str(), nullptr);
}

bool GetFromCompress(SQLite& sql, const std::string& input, std::string& output)
{
	std::string statement = "SELECT output FROM \"";
	statement += compress;
	statement += "\" WHERE(input==\"";
	statement += input;
	statement += "\");";
	response.clear();
	sql.exec(statement.c_str(), &response);

	if (1 == response.size())
	{
		output = response["output"];
		return true;
	}

	return false;
}

void SetToCompress(SQLite& sql, const std::string& input, const std::string& output)
{
	std::string statement = "INSERT INTO \"";
	statement += compress;
	statement += "\"";
	statement += "(input, output) VALUES(\"";
	statement += input;
	statement += "\", \"";
	statement += output;
	statement += "\");";
	sql.exec(statement.c_str(), nullptr);
}
