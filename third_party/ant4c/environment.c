/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
 *
 */

#if !defined(_WIN32)
/*#define _POSIX_SOURCE 1
#define _POSIX_C_SOURCE 200112L*/
#include "stdc_secure_api.h"
#endif

#include "environment.h"
#include "buffer.h"
#include "range.h"
#include "text_encoding.h"

#include <string.h>

#if defined(_WIN32)
#include <stdio.h>
#include <wchar.h>

#include <windows.h>

#else

#include "string_unit.h"

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <sys/param.h>
#endif

#include <pwd.h>
#include <unistd.h>
#if defined(BSD)
#include <sys/sysctl.h>
#else
#include <sys/sysinfo.h>
#endif
#include <sys/utsname.h>

#include <stdlib.h>

#endif

#define LOCAL_OR_ARGUMENT(L, A) (NULL != (A) ? (A) : (L))

#if defined(_WIN32)

uint8_t environment_get_variable(
	const uint8_t* variable_name_start, const uint8_t* variable_name_finish, struct buffer* variable)
{
	if (range_in_parts_is_null_or_empty(variable_name_start, variable_name_finish))
	{
		return 0;
	}

	uint8_t local_variable_was_used = 0;
	struct buffer l_variable;
	SET_NULL_TO_BUFFER(l_variable);

	if (!variable)
	{
		local_variable_was_used = 1;
		variable = &l_variable;
	}

	const ptrdiff_t size = buffer_size(variable);

	if (!text_encoding_UTF8_to_UTF16LE(
			variable_name_start, variable_name_finish, variable) ||
		!buffer_push_back_uint16(variable, 0))
	{
		buffer_release(&l_variable);
		return 0;
	}

	const ptrdiff_t size_ = buffer_size(variable);

	if (!buffer_push_back_uint16(variable, 0))
	{
		buffer_release(&l_variable);
		return 0;
	}

	const wchar_t* variable_name = (const wchar_t*)buffer_data(variable, size);
	wchar_t* variable_value = (wchar_t*)buffer_data(variable, size_);
	/**/
	DWORD variable_value_size = GetEnvironmentVariableW(
									variable_name, variable_value, 1);
	/**/
	buffer_release(&l_variable);

	if (variable_value_size < 2)
	{
		return 0;
	}

	if (local_variable_was_used)
	{
		return 1;
	}

	if (!buffer_append(variable, NULL, sizeof(uint32_t) + (ptrdiff_t)6 * (variable_value_size + (ptrdiff_t)1)))
	{
		return 0;
	}

	variable_name = (const wchar_t*)buffer_data(variable, size);
	variable_value = (wchar_t*)buffer_data(variable,
										   buffer_size(variable) - sizeof(uint32_t) - sizeof(wchar_t) * variable_value_size - sizeof(wchar_t));
	/**/
	variable_value_size = GetEnvironmentVariableW(
							  variable_name, variable_value, variable_value_size);

	if (variable_value_size < 2)
	{
		return 0;
	}

	return buffer_resize(variable, size) &&
		   text_encoding_UTF16LE_to_UTF8(variable_value, variable_value + variable_value_size, variable);
}

#else

uint8_t environment_get_variable(const uint8_t* variable_name_start, const uint8_t* variable_name_finish,
								 struct buffer* variable)
{
	if (range_in_parts_is_null_or_empty(variable_name_start, variable_name_finish))
	{
		return 0;
	}

	struct buffer l_variable;

	SET_NULL_TO_BUFFER(l_variable);

	const ptrdiff_t size = buffer_size(variable);

	if (!buffer_append(LOCAL_OR_ARGUMENT(&l_variable, variable), variable_name_start,
					   variable_name_finish - variable_name_start) ||
		!buffer_push_back(LOCAL_OR_ARGUMENT(&l_variable, variable), 0))
	{
		buffer_release(&l_variable);
		return 0;
	}

	const char* value = getenv((const char*)buffer_data(LOCAL_OR_ARGUMENT(&l_variable, variable), size));
	buffer_release(&l_variable);

	if (NULL == value)
	{
		return 0;
	}

	if (NULL == variable)
	{
		return 1;
	}

	ptrdiff_t length = strlen(value);
	return buffer_resize(variable, size) && buffer_append(variable, (const uint8_t*)value, length);
}

#endif

uint8_t environment_variable_exists(const uint8_t* variable_name_start, const uint8_t* variable_name_finish)
{
	return environment_get_variable(variable_name_start, variable_name_finish, NULL);
}
