/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 TheVice
 *
 */

#ifndef _SQLITE_WRAPPER_H_
#define _SQLITE_WRAPPER_H_

#include <map>
#include <string>

struct sqlite3;

class SQLite
{
	sqlite3* db;

public:
	SQLite(const char* path);
	~SQLite();

	SQLite() = delete;
	SQLite(const SQLite&) = delete;
	SQLite& operator=(const SQLite&) = delete;

	SQLite(SQLite&&) = default;
	SQLite& operator=(SQLite&&) = default;

	bool exec(
		 const char* statement,
		 std::map<std::string, std::string>* response);
};

bool IsTableExists(SQLite& sql, const char* tableName);

void CreateGetUncompressedLengthTable(SQLite& sql);
bool GetFromGetUncompressedLength(SQLite& sql, const std::string& start, bool& return_, size_t& result_);
void SetToGetUncompressedLength(SQLite& sql, const std::string& start, bool return_, size_t result_);

void CreateCompressTable(SQLite& sql);
bool GetFromCompress(SQLite& sql, const std::string& input, std::string& output);
void SetToCompress(SQLite& sql, const std::string& input, const std::string& output);

void CreateUncompressTable(SQLite& sql);
bool GetFromUncompress(SQLite& sql, const std::string& input, std::string& output);
void SetToUncompress(SQLite& sql, const std::string& input, const std::string& output);

void CreateIsValidCompressedBufferTable(SQLite& sql);
bool GetFromIsValidCompressedBuffer(SQLite& sql, const std::string& input, bool& return_);
void SetToIsValidCompressedBuffer(SQLite& sql, const std::string& input, bool return_);

#endif
