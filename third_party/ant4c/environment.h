/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
 *
 */

#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include <stddef.h>
#include <stdint.h>

struct buffer;

static const uint8_t environment_posix_delimiter = ':';
static const uint8_t environment_windows_delimiter = ';';

#if defined(_WIN32)
#define ENVIRONMENT_DELIMITER environment_windows_delimiter
#else
#define ENVIRONMENT_DELIMITER environment_posix_delimiter
#endif

uint8_t environment_get_variable(const uint8_t* variable_name_start, const uint8_t* variable_name_finish,
								 struct buffer* variable);
uint8_t environment_variable_exists(const uint8_t* variable_name_start, const uint8_t* variable_name_finish);

#endif
