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

#include "path.h"
#include "buffer.h"
#if !defined(_WIN32)
#include "environment.h"
#endif
#include "file_system.h"
#include "range.h"
#include "string_unit.h"
#include "text_encoding.h"

#if defined(_WIN32)
#include <wchar.h>

#include <windows.h>
#else

#include <unistd.h>
#include <stdlib.h>

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <sys/param.h>
#endif

#if defined(__APPLE__) && defined(__MACH__)
#include <mach-o/dyld.h>
#endif

#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#include <sys/sysctl.h>
#include <errno.h>
#endif

#endif

#include <stdio.h>
#include <string.h>

#ifndef L_tmpnam_s
#define L_tmpnam_s L_tmpnam
#endif

static const uint8_t point = '.';
#if defined(_WIN32)
static const uint8_t colon_mark = ':';
#else
static const uint8_t tilde = '~';
#endif
static const uint8_t* upper_level = (const uint8_t*)"..";


uint8_t path_combine_in_place(
	struct buffer* path1, const ptrdiff_t size,
	const uint8_t* path2_start, const uint8_t* path2_finish)
{
	if (NULL == path1 ||
		buffer_size(path1) < size ||
		path2_finish < path2_start)
	{
		return 0;
	}

	if (size < buffer_size(path1) &&
		path2_start < path2_finish &&
		!buffer_push_back(path1, PATH_DELIMITER))
	{
		return 0;
	}

	if (!buffer_append(path1, path2_start, path2_finish - path2_start))
	{
		return 0;
	}

	ptrdiff_t new_size = buffer_size(path1) - size;

	if (new_size)
	{
#if defined(_WIN32)

		if (!cygpath_get_windows_path(buffer_data(path1, size), buffer_data(path1, size) + new_size))
#else
		if (!cygpath_get_unix_path(buffer_data(path1, size), buffer_data(path1, size) + new_size))
#endif
		{
			return 0;
		}

		if (!string_replace_double_char_with_single(
				buffer_data(path1, size),
				&new_size, &PATH_DELIMITER,
				&PATH_DELIMITER + 1))
		{
			return 0;
		}
	}

	return new_size ? buffer_resize(path1, size + new_size) : 1;
}

uint8_t path_combine(
	const uint8_t* path1_start, const uint8_t* path1_finish,
	const uint8_t* path2_start, const uint8_t* path2_finish,
	struct buffer* output)
{
	if (path1_finish < path1_start ||
		path2_finish < path2_start ||
		NULL == output)
	{
		return 0;
	}

	ptrdiff_t new_size = path1_finish - path1_start + path2_finish - path2_start;

	if (path1_start < path1_finish && path2_start < path2_finish)
	{
		++new_size;
	}

	if (!new_size)
	{
		return 1;
	}

	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, new_size))
	{
		return 0;
	}

	if (!buffer_resize(output, size))
	{
		return 0;
	}

	if (!buffer_append(output, path1_start, path1_finish - path1_start))
	{
		return 0;
	}

	return path_combine_in_place(output, size, path2_start, path2_finish);
}

uint8_t path_get_directory_name(
	const uint8_t* path_start, const uint8_t** path_finish)
{
	if (NULL == path_finish ||
		range_in_parts_is_null_or_empty(path_start, *path_finish))
	{
		return 0;
	}

	const uint8_t* pos = string_find_any_symbol_like_or_not_like_that(
							 *path_finish, path_start,
							 &PATH_DELIMITER, &PATH_DELIMITER + 1, 1, -1);
	uint32_t char_set;
	const uint8_t* pos_ = string_enumerate(pos, *path_finish, &char_set);

	if (PATH_DELIMITER == char_set && path_start == pos)
	{
		*path_finish = pos_;
	}
	else
	{
		*path_finish = pos;
	}

	return path_start != *path_finish;
}

uint8_t path_get_temp_file_name(struct buffer* temp_file_name)
{
	if (NULL == temp_file_name)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(temp_file_name);
#if !defined(_WIN32)

	if (!path_get_temp_path(temp_file_name) ||
		!buffer_append_char(temp_file_name, "/fileXXXXXX\0", 12))
	{
		return 0;
	}

	char* temp_file_path = (char*)buffer_data(temp_file_name, size);
	int fd;

	if (-1 == (fd = mkstemp(temp_file_path)))
	{
		return 0;
	}

	close(fd);
	return buffer_resize(temp_file_name, buffer_size(temp_file_name) - 1);
#else

	if (!buffer_append_char(temp_file_name, NULL, L_tmpnam_s))
	{
		return 0;
	}

	uint8_t* temp_file_path = buffer_data(temp_file_name, size);
#if __STDC_LIB_EXT1__

	if (0 != tmpnam_s((char*)temp_file_path, L_tmpnam_s))
	{
		return 0;
	}

#else
	tmpnam((char*)temp_file_path);
#endif
	ptrdiff_t length = strlen((const char*)temp_file_path);
	const uint8_t* temp_file_path_finish = temp_file_path + length;

	if (length < 1)
	{
		return 0;
	}

	if (string_ends_with(
			temp_file_path, temp_file_path_finish,
			&point, &point + 1))
	{
		uint32_t char_set;
		const uint8_t* pos = string_find_any_symbol_like_or_not_like_that(
								 temp_file_path_finish, temp_file_path,
								 &point, &point + 1, 0, -1);

		if (pos == temp_file_path)
		{
			return 0;
		}

		if (!string_enumerate(pos, temp_file_path_finish, &char_set))
		{
			return 0;
		}

		if (PATH_DELIMITER == char_set)
		{
			return 0;
		}

		pos = string_enumerate(pos, temp_file_path_finish, NULL);

		if (NULL == pos)
		{
			return 0;
		}

		length = pos - temp_file_path;
	}

	uint8_t is_path_rooted;

	if (!path_is_path_rooted(temp_file_path, temp_file_path + length, &is_path_rooted))
	{
		return 0;
	}

	if (is_path_rooted)
	{
		if (!buffer_resize(temp_file_name, size + length) ||
			!buffer_push_back(temp_file_name, 0))
		{
			return 0;
		}

		return buffer_resize(temp_file_name, size + length);
	}

	const ptrdiff_t temp_path_start = buffer_size(temp_file_name);

	if (!path_get_temp_path(temp_file_name))
	{
		return 0;
	}

	const ptrdiff_t temp_path_finish = buffer_size(temp_file_name);

	if (!buffer_append(temp_file_name, NULL, (ptrdiff_t)2 + temp_path_finish - size))
	{
		return 0;
	}

	const uint8_t* temp_path = buffer_data(temp_file_name, temp_path_start);
	const uint8_t* temp_path_ = buffer_data(temp_file_name, temp_path_finish);
	temp_file_path = buffer_data(temp_file_name, size);

	if (!buffer_resize(temp_file_name, temp_path_finish) ||
		!path_combine(
			temp_path, temp_path_,
			temp_file_path, temp_file_path + length,
			temp_file_name))
	{
		return 0;
	}

	length = buffer_size(temp_file_name) - temp_path_finish;
#if __STDC_LIB_EXT1__

	if (0 != memcpy_s(temp_file_path, length, temp_path_, length))
	{
		return 0;
	}

#else
	memcpy(temp_file_path, temp_path_, length);
#endif

	if (!buffer_resize(temp_file_name, size + length) ||
		!buffer_push_back(temp_file_name, 0))
	{
		return 0;
	}

	return buffer_resize(temp_file_name, size + length);
#endif
}

uint8_t path_get_temp_path(struct buffer* temp_path)
{
	if (NULL == temp_path)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(temp_path);
#if defined(_WIN32)

	if (!buffer_append(temp_path, NULL, 6 * FILENAME_MAX + sizeof(uint32_t)))
	{
		return 0;
	}

	wchar_t* pathW = (wchar_t*)buffer_data(temp_path, size);
	DWORD length = GetTempPathW(sizeof(wchar_t), pathW);

	if (length < sizeof(wchar_t) + 1)
	{
		return 0;
	}

	if (!buffer_resize(temp_path, size) ||
		!buffer_append(temp_path, NULL, (ptrdiff_t)6 * length + sizeof(uint32_t)))
	{
		return 0;
	}

	pathW = (wchar_t*)buffer_data(temp_path,
								  buffer_size(temp_path) - sizeof(uint32_t) - sizeof(uint16_t) * length - sizeof(uint16_t));
	length = GetTempPathW(length, pathW);

	if (!buffer_resize(temp_path, size) ||
		!text_encoding_UTF16LE_to_UTF8(pathW, pathW + length, temp_path))
	{
		return 0;
	}

#else
	static const uint8_t* tmp_dir = (const uint8_t*)"TMPDIR";

	if (environment_get_variable(tmp_dir, tmp_dir + 6, temp_path))
	{
#endif
	const uint8_t* path_start = buffer_data(temp_path, size);
	const uint8_t* path_finish = buffer_data(temp_path, 0) + buffer_size(temp_path);

	if (!path_get_directory_name(path_start, &path_finish))
	{
		return 0;
	}

	return buffer_resize(temp_path, size + (path_finish - path_start));
#if !defined(_WIN32)
}

if (!buffer_resize(temp_path, size))
{
	return 0;
}

#if defined(__ANDROID__)
static const uint8_t* temp_path_ = (const uint8_t*)"/data/local/tmp";
#define TEMP_PATH_SIZE 15
#else
static const uint8_t* temp_path_ = (const uint8_t*)"/tmp";
#define TEMP_PATH_SIZE 4
#endif
return directory_exists(temp_path_) &&
	   buffer_append(temp_path, temp_path_, TEMP_PATH_SIZE);
#endif
}

uint8_t path_is_path_rooted(
	const uint8_t* path_start, const uint8_t* path_finish,
	uint8_t* is_path_rooted)
{
	if (range_in_parts_is_null_or_empty(path_start, path_finish) ||
		!is_path_rooted)
	{
		return 0;
	}

	struct range out;

	uint32_t char_set;

#if defined(_WIN32)
	ptrdiff_t length = string_get_length(path_start, path_finish);

	if (2 < length)
	{
		length = file_system_get_position_after_pre_root(&path_start, path_finish);

		if (!length)
		{
			return 0;
		}
		else if (2 == length)
		{
			return path_is_path_rooted(path_start, path_finish, is_path_rooted);
		}

		length = 2;
	}

	if (1 < length)
	{
		if (!string_substring(path_start, path_finish, 1, 1, &out))
		{
			return 0;
		}

		if (!string_enumerate(out.start, out.finish, &char_set))
		{
			return 0;
		}

		*is_path_rooted = colon_mark == char_set;
		return 1;
	}

#endif

	if (!string_substring(path_start, path_finish, 0, 1, &out))
	{
		return 0;
	}

	if (!string_enumerate(out.start, out.finish, &char_set))
	{
		return 0;
	}

	*is_path_rooted =
		path_posix_delimiter == char_set ||
		PATH_DELIMITER == char_set;
	return 1;
}

uint8_t cygpath_get_unix_path(uint8_t* path_start, uint8_t* path_finish)
{
	if (range_in_parts_is_null_or_empty(path_start, path_finish))
	{
		return 0;
	}

	uint32_t char_set;
	const uint8_t* pos = path_start;

	while (NULL != (pos = string_enumerate(pos, path_finish, &char_set)))
	{
		if (path_windows_delimiter == char_set &&
			1 == pos - path_start)
		{
			*path_start = path_posix_delimiter;
		}

		path_start += pos - path_start;
	}

	return 1;
}

uint8_t cygpath_get_windows_path(uint8_t* path_start, uint8_t* path_finish)
{
	if (range_in_parts_is_null_or_empty(path_start, path_finish))
	{
		return 0;
	}

	uint32_t char_set;
	const uint8_t* pos = path_start;

	while (NULL != (pos = string_enumerate(pos, path_finish, &char_set)))
	{
		if (path_posix_delimiter == char_set &&
			1 == pos - path_start)
		{
			*path_start = path_windows_delimiter;
		}

		path_start += pos - path_start;
	}

	return 1;
}
