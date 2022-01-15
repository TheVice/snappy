/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
 *
 */

#include "string_unit.h"
#include "buffer.h"
#include "range.h"
#include "text_encoding.h"

#include <ctype.h>
#include <string.h>

static const uint8_t* quote_symbols = (const uint8_t*)"\"'";

ptrdiff_t string_cmp_(
	const uint8_t* input_1_start, const uint8_t* input_1_finish,
	const uint8_t* input_2_start, const uint8_t* input_2_finish)
{
	uint32_t input_1_char_set, input_2_char_set;

	while (NULL != (input_1_start = string_enumerate(input_1_start, input_1_finish, &input_1_char_set)) &&
		   NULL != (input_2_start = string_enumerate(input_2_start, input_2_finish, &input_2_char_set)))
	{
		if (input_1_char_set != input_2_char_set)
		{
			return input_1_char_set < input_2_char_set ? -1 : 1;
		}
	}

	return (NULL == input_2_start || input_2_finish == input_2_start) ?
		   0 : (input_1_start < input_2_start ? -1 : 1);
}

ptrdiff_t string_index_of_value(
	const uint8_t* input_start, const uint8_t* input_finish,
	const uint8_t* value_start, const uint8_t* value_finish, int8_t step)
{
	if (range_in_parts_is_null_or_empty(input_start, input_finish) ||
		range_in_parts_is_null_or_empty(value_start, value_finish) ||
		(-1 != step && 1 != step))
	{
		if (input_start == input_finish &&
			value_start == value_finish)
		{
			return 0;
		}

		return -1;
	}

	uint32_t input_char_set, value_char_set;

	if (NULL == (value_start = string_enumerate(value_start, value_finish, &value_char_set)))
	{
		return 0;
	}

	const uint8_t value_empty = range_in_parts_is_null_or_empty(value_start, value_finish);
	ptrdiff_t index = -1, i = 0;

	while (NULL != (input_start = string_enumerate(input_start, input_finish, &input_char_set)))
	{
		if (value_char_set == input_char_set)
		{
			if (!value_empty &&
				!range_in_parts_is_null_or_empty(input_start, input_finish) &&
				string_cmp_(input_start, input_finish, value_start, value_finish))
			{
				++i;
				continue;
			}

			index = i;

			if (0 < step)
			{
				break;
			}
		}

		++i;
	}

	return index;
}

uint8_t string_contains(const uint8_t* input_start, const uint8_t* input_finish,
						const uint8_t* value_start, const uint8_t* value_finish)
{
	return -1 != string_index_of_value(input_start, input_finish, value_start, value_finish, 1);
}

uint8_t string_ends_with(const uint8_t* input_start, const uint8_t* input_finish,
						 const uint8_t* value_start, const uint8_t* value_finish)
{
	if (range_in_parts_is_null_or_empty(input_start, input_finish) ||
		range_in_parts_is_null_or_empty(value_start, value_finish))
	{
		if (input_start == input_finish &&
			value_start == value_finish)
		{
			return 1;
		}

		return 0;
	}

	const ptrdiff_t length = value_finish - value_start;

	if (input_finish - input_start < length)
	{
		return 0;
	}

	return 0 == memcmp(input_finish - length, value_start, length);
}

const uint8_t* string_enumerate(const uint8_t* input_start, const uint8_t* input_finish, uint32_t* char_set)
{
	static uint32_t internal_char_set;

	if (NULL == char_set)
	{
		char_set = &internal_char_set;
	}

	const uint8_t offset = text_encoding_decode_UTF8_single(input_start, input_finish, char_set);
	return offset ? input_start + offset : NULL;
}

#define LIKE_OR_NOT()																		\
	step = 0;																				\
	that_pos = that_start;																	\
	\
	while (NULL != (that_pos = string_enumerate(that_pos, that_finish, &that_char_set)))	\
	{																						\
		step = (input_char_set == that_char_set);											\
		\
		if (step)																			\
		{																					\
			break;																			\
		}																					\
	}

const uint8_t* string_find_any_symbol_like_or_not_like_that(
	const uint8_t* start, const uint8_t* finish,
	const uint8_t* that_start, const uint8_t* that_finish,
	uint8_t like, int8_t step)
{
	if (NULL == start || NULL == finish ||
		range_in_parts_is_null_or_empty(that_start, that_finish) ||
		(0 != like && 1 != like) ||
		(-1 != step && 1 != step) ||
		((0 < step) ? (finish < start) : (finish > start)))
	{
		return finish;
	}

	uint32_t input_char_set, that_char_set;
	const uint8_t* input_pos;
	const uint8_t* that_pos;

	if (0 < step)
	{
		input_pos = start;

		while (NULL != (input_pos = string_enumerate(input_pos, finish, &input_char_set)))
		{
			LIKE_OR_NOT();

			if (like == step)
			{
				return start;
			}

			start = input_pos;
		}
	}
	else
	{
		const uint8_t* result = finish;
		input_pos = finish;

		while (NULL != (input_pos = string_enumerate(input_pos, start, &input_char_set)))
		{
			LIKE_OR_NOT();

			if (like == step)
			{
				result = finish;
			}

			finish = input_pos;
		}

		start = result;
	}

	return start;
}

ptrdiff_t string_get_length(const uint8_t* input_start, const uint8_t* input_finish)
{
	if (input_start == input_finish)
	{
		return 0;
	}

	if (NULL == input_start || NULL == input_finish || input_finish < input_start)
	{
		return -1;
	}

	ptrdiff_t length = 0;

	while (NULL != (input_start = string_enumerate(input_start, input_finish, NULL)))
	{
		++length;
	}

	return length;
}

ptrdiff_t string_index_of(const uint8_t* input_start, const uint8_t* input_finish,
						  const uint8_t* value_start, const uint8_t* value_finish)
{
	return string_index_of_value(input_start, input_finish, value_start, value_finish, 1);
}

ptrdiff_t string_last_index_of(const uint8_t* input_start, const uint8_t* input_finish,
							   const uint8_t* value_start, const uint8_t* value_finish)
{
	return string_index_of_value(input_start, input_finish, value_start, value_finish, -1);
}

ptrdiff_t string_index_of_any(const uint8_t* input_start, const uint8_t* input_finish,
							  const uint8_t* value_start, const uint8_t* value_finish)
{
	if (range_in_parts_is_null_or_empty(value_start, value_finish))
	{
		return -1;
	}

	ptrdiff_t index = -1, i = 0;
	uint32_t input_char_set, value_char_set;

	while (NULL != (input_start = string_enumerate(input_start, input_finish, &input_char_set)))
	{
		const uint8_t* value_pos = value_start;

		while (NULL != (value_pos = string_enumerate(value_pos, value_finish, &value_char_set)))
		{
			if (input_char_set == value_char_set)
			{
				index = i;
				input_start = NULL;
				break;
			}
		}

		++i;
	}

	return index;
}

ptrdiff_t string_last_index_of_any(const uint8_t* input_start, const uint8_t* input_finish,
								   const uint8_t* value_start, const uint8_t* value_finish)
{
	if (range_in_parts_is_null_or_empty(value_start, value_finish))
	{
		return -1;
	}

	ptrdiff_t index = -1, i = 0;
	uint32_t input_char_set, value_char_set;

	while (NULL != (input_start = string_enumerate(input_start, input_finish, &input_char_set)))
	{
		const uint8_t* value_pos = value_start;

		while (NULL != (value_pos = string_enumerate(value_pos, value_finish, &value_char_set)))
		{
			if (input_char_set == value_char_set)
			{
				index = i;
				break;
			}
		}

		++i;
	}

	return index;
}

enum string_pad_side { string_pad_left_function = 0, string_pad_right_function };

uint8_t string_pad(const uint8_t* input_start, const uint8_t* input_finish,
				   const uint8_t* value_start, const uint8_t* value_finish,
				   ptrdiff_t result_length, struct buffer* output, enum string_pad_side side)
{
	if (input_finish < input_start ||
		range_in_parts_is_null_or_empty(value_start, value_finish) ||
		result_length < 0 ||
		NULL == output)
	{
		return 0;
	}

	const ptrdiff_t length = string_get_length(input_start, input_finish);

	if (result_length < length + 1)
	{
		return buffer_append(output, input_start, input_finish - input_start);
	}

	result_length -= length;
	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL,
					   input_finish - input_start + 4 * result_length + sizeof(uint32_t)) ||
		!buffer_resize(output, size + sizeof(uint32_t)))
	{
		return 0;
	}

	const uint8_t offset = text_encoding_decode_UTF8_single(
							   value_start, value_finish,
							   (uint32_t*)buffer_data(output, size));

	if (!offset ||
		!buffer_resize(output, size))
	{
		return 0;
	}

	if (string_pad_left_function == side)
	{
		while (result_length)
		{
			if (!buffer_append(output, value_start, offset))
			{
				return 0;
			}

			--result_length;
		}
	}

	if (!buffer_append(output, input_start, input_finish - input_start))
	{
		return 0;
	}

	if (string_pad_right_function == side)
	{
		while (result_length)
		{
			if (!buffer_append(output, value_start, offset))
			{
				return 0;
			}

			--result_length;
		}
	}

	return 1;
}

uint8_t string_pad_left(const uint8_t* input_start, const uint8_t* input_finish,
						const uint8_t* value_start, const uint8_t* value_finish,
						ptrdiff_t result_length, struct buffer* output)
{
	return string_pad(input_start, input_finish, value_start, value_finish,
					  result_length, output, string_pad_left_function);
}

uint8_t string_pad_right(const uint8_t* input_start, const uint8_t* input_finish,
						 const uint8_t* value_start, const uint8_t* value_finish,
						 ptrdiff_t result_length, struct buffer* output)
{
	return string_pad(input_start, input_finish, value_start, value_finish,
					  result_length, output, string_pad_right_function);
}

uint8_t string_replace(const uint8_t* input_start, const uint8_t* input_finish,
					   const uint8_t* to_be_replaced_start, const uint8_t* to_be_replaced_finish,
					   const uint8_t* by_replacement_start, const uint8_t* by_replacement_finish,
					   struct buffer* output)
{
	if (NULL == input_start || NULL == input_finish ||
		range_in_parts_is_null_or_empty(to_be_replaced_start, to_be_replaced_finish) ||
		NULL == output || input_finish < input_start)
	{
		return 0;
	}

	const ptrdiff_t input_length = input_finish - input_start;

	if (!input_length)
	{
		return 1;
	}

	if (string_equal(to_be_replaced_start, to_be_replaced_finish, by_replacement_start, by_replacement_finish))
	{
		return buffer_append(output, input_start, input_length);
	}

	const ptrdiff_t to_be_replaced_length = to_be_replaced_finish - to_be_replaced_start;
	const ptrdiff_t by_replacement_length = (NULL == by_replacement_start || NULL == by_replacement_finish ||
											by_replacement_finish < by_replacement_start) ? -1 : (by_replacement_finish - by_replacement_start);
	const uint8_t* start = input_start;

	while (NULL != input_start && to_be_replaced_length <= input_finish - input_start)
	{
		if (memcmp(input_start, to_be_replaced_start, to_be_replaced_length))
		{
			input_start = string_enumerate(input_start, input_finish, NULL);
			continue;
		}

		if (!buffer_append(output, start, input_start - start))
		{
			return 0;
		}

		if (-1 != by_replacement_length &&
			!buffer_append(output, by_replacement_start, by_replacement_length))
		{
			return 0;
		}

		input_start += to_be_replaced_length;
		start = input_start;
	}

	return buffer_append(output, start, input_finish - start);
}

#define MEM_CPY(DST, SRC, LENGTH)								\
	for (ptrdiff_t counter = 0; counter < LENGTH; ++counter)	\
	{															\
		(*(DST)) = (*(SRC));									\
		++(DST);												\
		++(SRC);												\
	}

uint8_t string_replace_double_char_with_single(
	uint8_t* input, ptrdiff_t* size,
	const uint8_t* to_be_replaced_start,
	const uint8_t* to_be_replaced_finish)
{
	uint32_t char_set;

	if (NULL == input || NULL == size || 0 == *size ||
		NULL == (string_enumerate(
					 to_be_replaced_start, to_be_replaced_finish, &char_set)))
	{
		return 0;
	}

	uint8_t* start = NULL;
	ptrdiff_t match = 0;
	uint32_t input_char_set;
	const uint8_t* pos;
	const uint8_t* prev_pos = input;
	const uint8_t* finish = input + *size;

	while (NULL != (pos = string_enumerate(prev_pos, finish, &input_char_set)))
	{
		if (input_char_set == char_set)
		{
			if (!match)
			{
				start = input + (pos - input);
			}

			match++;
		}
		else
		{
			if (1 < match)
			{
				const ptrdiff_t length = finish - prev_pos;
				const uint8_t* src = prev_pos;
				MEM_CPY(start, src, length);
				finish -= match - 1;
			}

			match = 0;
		}

		prev_pos = pos;
	}

	if (1 < match)
	{
		finish -= match - 1;
	}

	*size = finish - input;
	return 1;
}

uint8_t string_starts_with(const uint8_t* input_start, const uint8_t* input_finish,
						   const uint8_t* value_start, const uint8_t* value_finish)
{
	if (range_in_parts_is_null_or_empty(input_start, input_finish) ||
		range_in_parts_is_null_or_empty(value_start, value_finish))
	{
		if (input_start == input_finish &&
			value_start == value_finish)
		{
			return 1;
		}

		return 0;
	}

	const ptrdiff_t length = value_finish - value_start;

	if (input_finish - input_start < length)
	{
		return 0;
	}

	return 0 == memcmp(input_start, value_start, length);
}

uint8_t string_substring(const uint8_t* input_start, const uint8_t* input_finish,
						 ptrdiff_t index, ptrdiff_t length, struct range* output)
{
	if (range_in_parts_is_null_or_empty(input_start, input_finish) ||
		index < 0 ||
		NULL == output)
	{
		return 0;
	}

	while (index)
	{
		if (NULL == (input_start =
						 string_enumerate(input_start, input_finish, NULL)))
		{
			output->start = output->finish = NULL;
			return 0;
		}

		--index;
	}

	output->start = input_start;

	if (length < 0)
	{
		output->finish = input_finish;
	}
	else
	{
		while (length)
		{
			if (NULL == (input_start =
							 string_enumerate(input_start, input_finish, NULL)))
			{
				output->start = output->finish = NULL;
				return 0;
			}

			--length;
		}

		output->finish = input_start;
	}

	return 1;
}

#define CHANGE_CASE_A(I, C, D)	\
	if (string_get_id_of_to_lower_function() == (C))		\
	{							\
		if (!((I) % 2))			\
		{						\
			return (I) + (D);	\
		}						\
	}							\
	else						\
	{							\
		if ((I) % 2)			\
		{						\
			return (I) - (D);	\
		}						\
	}

#define CHANGE_CASE_B(I, C, D)	\
	if (string_get_id_of_to_lower_function() == (C))		\
	{							\
		if ((I) % 2)			\
		{						\
			return (I) + (D);	\
		}						\
	}							\
	else						\
	{							\
		if (!((I) % 2))			\
		{						\
			return (I) - (D);	\
		}						\
	}

uint8_t string_quote(const uint8_t* input_start, const uint8_t* input_finish,
					 struct buffer* output)
{
	if (NULL == output)
	{
		return 0;
	}

	if (!buffer_push_back(output, quote_symbols[0]))
	{
		return 0;
	}

	if (!range_in_parts_is_null_or_empty(input_start, input_finish) &&
		!buffer_append(output, input_start, input_finish - input_start))
	{
		return 0;
	}

	return buffer_push_back(output, quote_symbols[0]);
}

uint8_t string_un_quote(struct range* input_output)
{
	if (NULL == input_output ||
		NULL == input_output->start ||
		NULL == input_output->finish ||
		input_output->finish < input_output->start)
	{
		return 0;
	}

	input_output->start = string_find_any_symbol_like_or_not_like_that(
							  input_output->start, input_output->finish,
							  quote_symbols, quote_symbols + 2, 0, 1);

	if (input_output->finish == input_output->start)
	{
		return 1;
	}

	const uint8_t* pos = string_find_any_symbol_like_or_not_like_that(
							 input_output->finish, input_output->start,
							 quote_symbols, quote_symbols + 2, 0, -1);
	input_output->finish = string_find_any_symbol_like_or_not_like_that(
							   pos, input_output->finish,
							   quote_symbols, quote_symbols + 2, 1, 1);
	return 1;
}

uint8_t string_equal(const uint8_t* input_1_start, const uint8_t* input_1_finish,
					 const uint8_t* input_2_start, const uint8_t* input_2_finish)
{
	if (NULL == input_1_start || NULL == input_1_finish ||
		NULL == input_2_start || NULL == input_2_finish ||
		input_1_finish < input_1_start || input_2_finish < input_2_start ||
		input_1_finish - input_1_start != input_2_finish - input_2_start)
	{
		return 0;
	}

	if (0 == input_1_finish - input_1_start)
	{
		return 1;
	}

	if (input_1_start == input_2_start)
	{
		return 1;
	}

	return 0 == memcmp(input_1_start, input_2_start, input_1_finish - input_1_start);
}
