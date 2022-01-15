/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 TheVice
 *
 */

#include "path_wrapper.h"

extern "C"
{
#include "buffer.h"
#include "path.h"
};

std::string PathGetTempFileName()
{
	buffer path_;
	SET_NULL_TO_BUFFER(path_);

	if (!path_get_temp_file_name(&path_))
	{
		buffer_release(&path_);
		return "";
	}

	std::string path(reinterpret_cast<const char*>(buffer_data(&path_, 0)), buffer_size(&path_));
	buffer_release(&path_);
	return path;
}
