/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
 *
 */

/*#if !defined(_WIN32)
#if defined(__linux)
#define _POSIX_SOURCE 1
#define _DEFAULT_SOURCE 1
#else
#define _BSD_SOURCE 1
#endif
#endif*/

#include "stdc_secure_api.h"

#include "file_system.h"
#include "buffer.h"
#include "path.h"
#include "range.h"
#include "string_unit.h"
#include "text_encoding.h"

#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <utime.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

static const uint8_t zero = '\0';
#if defined(_WIN32)
static const wchar_t zeroW = L'\0';
#endif

#if defined(_WIN32)

#define FIND_FILE_OBJECT_DATA(PATHW, DATA, RETURN, CLOSE_RESULT)\
	const HANDLE file_handle = FindFirstFileW((PATHW), (DATA));	\
	\
	if (INVALID_HANDLE_VALUE == file_handle)					\
	{															\
		return (RETURN);										\
	}															\
	\
	(CLOSE_RESULT) = FindClose(file_handle);

#define FIND_FILE_OBJECT_DATA_FROM_BUFFER(PATH)												\
	struct buffer pathW;																	\
	SET_NULL_TO_BUFFER(pathW);																\
	\
	if (!file_system_path_to_pathW((PATH), &pathW))											\
	{																						\
		buffer_release(&pathW);																\
		return 0;																			\
	}																						\
	\
	WIN32_FIND_DATAW file_data;																\
	const HANDLE file_handle = FindFirstFileW(buffer_wchar_t_data(&pathW, 0), &file_data);	\
	buffer_release(&pathW);																	\
	\
	if (INVALID_HANDLE_VALUE == file_handle)												\
	{																						\
		return 0;																			\
	}																						\
	\
	if (!FindClose(file_handle))															\
	{																						\
		return 0;																			\
	}

static const uint8_t* pre_root_path = (const uint8_t*)"\\\\?\\";
static const wchar_t* pre_root_path_wchar_t = L"\\\\?\\";
static const uint8_t pre_root_path_length = 4;


uint8_t file_system_get_position_after_pre_root(
	const uint8_t** path_start, const uint8_t* path_finish)
{
	if (!path_start ||
		range_in_parts_is_null_or_empty(*path_start, path_finish))
	{
		return 0;
	}

	const uint8_t* pos = *path_start;

	while (NULL != pos && pre_root_path_length <= path_finish - pos)
	{
		if (memcmp(pos, pre_root_path, pre_root_path_length))
		{
			pos = string_enumerate(pos, path_finish, NULL);
			continue;
		}

		*path_start = pos + pre_root_path_length;
		return 2;
	}

	return 1;
}

uint8_t directory_exists_wchar_t(const wchar_t* path)
{
	if (NULL == path)
	{
		return 0;
	}

	WIN32_FIND_DATAW file_data;
	BOOL close_return = 0;
	FIND_FILE_OBJECT_DATA(path, &file_data, 0, close_return)
	const uint8_t is_directory = (0 != (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
	return close_return && is_directory;
}

uint8_t file_system_path_in_range_to_pathW(
	const uint8_t* path_start, const uint8_t* path_finish, struct buffer* pathW)
{
	if (range_in_parts_is_null_or_empty(path_start, path_finish) ||
		NULL == pathW)
	{
		return 0;
	}

	uint8_t is_path_rooted;

	if (!path_is_path_rooted(path_start, path_finish, &is_path_rooted))
	{
		return 0;
	}

	if (is_path_rooted &&
		!string_starts_with(path_start, path_finish, pre_root_path, pre_root_path + pre_root_path_length) &&
		!buffer_append_wchar_t(pathW, pre_root_path_wchar_t, pre_root_path_length))
	{
		return 0;
	}

	return text_encoding_UTF8_to_UTF16LE(path_start, path_finish, pathW) && buffer_push_back_uint16(pathW, 0);
}

uint8_t file_system_path_to_pathW(const uint8_t* path, struct buffer* pathW)
{
	if (NULL == path ||
		NULL == pathW)
	{
		return 0;
	}

	const ptrdiff_t length = strlen((const char*)path);
	return file_system_path_in_range_to_pathW(path, path + length, pathW);
}

#endif

uint8_t directory_exists(const uint8_t* path)
{
	if (NULL == path)
	{
		return 0;
	}

#if defined(_WIN32)
	struct buffer pathW;
	SET_NULL_TO_BUFFER(pathW);

	if (!file_system_path_to_pathW(path, &pathW))
	{
		buffer_release(&pathW);
		return 0;
	}

	const uint8_t returned = directory_exists_wchar_t(buffer_wchar_t_data(&pathW, 0));
	buffer_release(&pathW);
	return returned;
#else
	DIR* dir = opendir((const char*)path);

	if (NULL == dir)
	{
		return 0;
	}

	closedir(dir);
	return 1;
#endif
}
