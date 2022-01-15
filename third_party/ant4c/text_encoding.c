/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
 *
 */

#include "text_encoding.h"
#ifndef NO_BUFFER_UNIT
#include "buffer.h"
#endif
#ifndef NO_COMMON_UNIT
//#include "common.h"
#endif

#include <string.h>

static const uint8_t UTF8_BOM[] = { 0xEF, 0xBB, 0xBF };
static const uint8_t UTF16BE_BOM[] = { 0xFE, 0xFF };
static const uint8_t UTF16LE_BOM[] = { 0xFF, 0xFE };
static const uint8_t UTF32BE_BOM[] = { 0x00, 0x00, 0xFE, 0xFF };
static const uint8_t UTF32LE_BOM[] = { 0xFF, 0xFE, 0x00, 0x00 };

static const uint8_t UTF8_UNKNOWN_CHAR[] = { 0xEF, 0xBF, 0xBD };
static const uint16_t UTF16BE_UNKNOWN_CHAR = 0xFDFF;
static const uint16_t UTF16LE_UNKNOWN_CHAR = 0xFFFD;

#ifndef NO_BUFFER_UNIT
static const uint8_t unknown_ASCII_char = 0x3F;
static const uint8_t max_ASCII_char = 0x7f;
static const uint8_t min_non_ASCII_char = 0x80;

static const uint16_t codes_874[] =
{
	0x20AC, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0x2026, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD
};

static const uint16_t codes_1250[] =
{
	0x20AC, 0xFFFD, 0x201A, 0xFFFD, 0x201E, 0x2026, 0x2020, 0x2021, 0xFFFD, 0x2030, 0x0160, 0x2039, 0x015A, 0x0164, 0x017D, 0x0179,
	0xFFFD, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0xFFFD, 0x2122, 0x0161, 0x203A, 0x015B, 0x0165, 0x017E, 0x017A,
	0x00A0, 0x02C7, 0x02D8, 0x0141, 0x00A4, 0x0104, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x015E, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x017B,
	0x00B0, 0x00B1, 0x02DB, 0x0142, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x0105, 0x015F, 0x00BB, 0x013D, 0x02DD, 0x013E, 0x017C,
	0x0154, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x0139, 0x0106, 0x00C7, 0x010C, 0x00C9, 0x0118, 0x00CB, 0x011A, 0x00CD, 0x00CE, 0x010E,
	0x0110, 0x0143, 0x0147, 0x00D3, 0x00D4, 0x0150, 0x00D6, 0x00D7, 0x0158, 0x016E, 0x00DA, 0x0170, 0x00DC, 0x00DD, 0x0162, 0x00DF,
	0x0155, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x013A, 0x0107, 0x00E7, 0x010D, 0x00E9, 0x0119, 0x00EB, 0x011B, 0x00ED, 0x00EE, 0x010F,
	0x0111, 0x0144, 0x0148, 0x00F3, 0x00F4, 0x0151, 0x00F6, 0x00F7, 0x0159, 0x016F, 0x00FA, 0x0171, 0x00FC, 0x00FD, 0x0163, 0x02D9
};

static const uint16_t codes_1251[] =
{
	0x0402, 0x0403, 0x201A, 0x0453, 0x201E, 0x2026, 0x2020, 0x2021, 0x20AC, 0x2030, 0x0409, 0x2039, 0x040A, 0x040C, 0x040B, 0x040F,
	0x0452, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0xFFFD, 0x2122, 0x0459, 0x203A, 0x045A, 0x045C, 0x045B, 0x045F,
	0x00A0, 0x040E, 0x045E, 0x0408, 0x00A4, 0x0490, 0x00A6, 0x00A7, 0x0401, 0x00A9, 0x0404, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x0407,
	0x00B0, 0x00B1, 0x0406, 0x0456, 0x0491, 0x00B5, 0x00B6, 0x00B7, 0x0451, 0x2116, 0x0454, 0x00BB, 0x0458, 0x0405, 0x0455, 0x0457,
	0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417, 0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
	0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
	0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437, 0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
	0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F
};

static const uint16_t codes_1252[] =
{
	0x20AC, 0xFFFD, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0xFFFD, 0x017D, 0xFFFD,
	0xFFFD, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0xFFFD, 0x017E, 0x0178,
	0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
	0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
	0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
	0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
	0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
	0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF
};

static const uint16_t codes_1253[] =
{
	0x20AC, 0xFFFD, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0xFFFD, 0x2030, 0xFFFD, 0x2039, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0xFFFD, 0x2122, 0xFFFD, 0x203A, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0x00A0, 0x0385, 0x0386, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0xFFFD, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x2015,
	0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x0384, 0x00B5, 0x00B6, 0x00B7, 0x0388, 0x0389, 0x038A, 0x00BB, 0x038C, 0x00BD, 0x038E, 0x038F,
	0x0390, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397, 0x0398, 0x0399, 0x039A, 0x039B, 0x039C, 0x039D, 0x039E, 0x039F,
	0x03A0, 0x03A1, 0xFFFD, 0x03A3, 0x03A4, 0x03A5, 0x03A6, 0x03A7, 0x03A8, 0x03A9, 0x03AA, 0x03AB, 0x03AC, 0x03AD, 0x03AE, 0x03AF,
	0x03B0, 0x03B1, 0x03B2, 0x03B3, 0x03B4, 0x03B5, 0x03B6, 0x03B7, 0x03B8, 0x03B9, 0x03BA, 0x03BB, 0x03BC, 0x03BD, 0x03BE, 0x03BF,
	0x03C0, 0x03C1, 0x03C2, 0x03C3, 0x03C4, 0x03C5, 0x03C6, 0x03C7, 0x03C8, 0x03C9, 0x03CA, 0x03CB, 0x03CC, 0x03CD, 0x03CE, 0xFFFD
};

static const uint16_t codes_1254[] =
{
	0x20AC, 0xFFFD, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0xFFFD, 0xFFFD, 0x0178,
	0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
	0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
	0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
	0x011E, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x0130, 0x015E, 0x00DF,
	0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
	0x011F, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x0131, 0x015F, 0x00FF
};

static const uint16_t codes_1255[] =
{
	0x20AC, 0xFFFD, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0xFFFD, 0x2039, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0xFFFD, 0x203A, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x20AA, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00D7, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
	0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00F7, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
	0x05B0, 0x05B1, 0x05B2, 0x05B3, 0x05B4, 0x05B5, 0x05B6, 0x05B7, 0x05B8, 0x05B9, 0xFFFD, 0x05BB, 0x05BC, 0x05BD, 0x05BE, 0x05BF,
	0x05C0, 0x05C1, 0x05C2, 0x05C3, 0x05F0, 0x05F1, 0x05F2, 0x05F3, 0x05F4, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
	0x05D0, 0x05D1, 0x05D2, 0x05D3, 0x05D4, 0x05D5, 0x05D6, 0x05D7, 0x05D8, 0x05D9, 0x05DA, 0x05DB, 0x05DC, 0x05DD, 0x05DE, 0x05DF,
	0x05E0, 0x05E1, 0x05E2, 0x05E3, 0x05E4, 0x05E5, 0x05E6, 0x05E7, 0x05E8, 0x05E9, 0x05EA, 0xFFFD, 0xFFFD, 0x200E, 0x200F, 0xFFFD
};

static const uint16_t codes_1256[] =
{
	0x20AC, 0x067E, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0679, 0x2039, 0x0152, 0x0686, 0x0698, 0x0688,
	0x06AF, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x06A9, 0x2122, 0x0691, 0x203A, 0x0153, 0x200C, 0x200D, 0x06BA,
	0x00A0, 0x060C, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x06BE, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
	0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x061B, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x061F,
	0x06C1, 0x0621, 0x0622, 0x0623, 0x0624, 0x0625, 0x0626, 0x0627, 0x0628, 0x0629, 0x062A, 0x062B, 0x062C, 0x062D, 0x062E, 0x062F,
	0x0630, 0x0631, 0x0632, 0x0633, 0x0634, 0x0635, 0x0636, 0x00D7, 0x0637, 0x0638, 0x0639, 0x063A, 0x0640, 0x0641, 0x0642, 0x0643,
	0x00E0, 0x0644, 0x00E2, 0x0645, 0x0646, 0x0647, 0x0648, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x0649, 0x064A, 0x00EE, 0x00EF,
	0x064B, 0x064C, 0x064D, 0x064E, 0x00F4, 0x064F, 0x0650, 0x00F7, 0x0651, 0x00F9, 0x0652, 0x00FB, 0x00FC, 0x200E, 0x200F, 0x06D2
};

static const uint16_t codes_1257[] =
{
	0x20AC, 0xFFFD, 0x201A, 0xFFFD, 0x201E, 0x2026, 0x2020, 0x2021, 0xFFFD, 0x2030, 0xFFFD, 0x2039, 0xFFFD, 0x00A8, 0x02C7, 0x00B8,
	0xFFFD, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0xFFFD, 0x2122, 0xFFFD, 0x203A, 0xFFFD, 0x00AF, 0x02DB, 0xFFFD,
	0x00A0, 0xFFFD, 0x00A2, 0x00A3, 0x00A4, 0xFFFD, 0x00A6, 0x00A7, 0x00D8, 0x00A9, 0x0156, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00C6,
	0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00F8, 0x00B9, 0x0157, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00E6,
	0x0104, 0x012E, 0x0100, 0x0106, 0x00C4, 0x00C5, 0x0118, 0x0112, 0x010C, 0x00C9, 0x0179, 0x0116, 0x0122, 0x0136, 0x012A, 0x013B,
	0x0160, 0x0143, 0x0145, 0x00D3, 0x014C, 0x00D5, 0x00D6, 0x00D7, 0x0172, 0x0141, 0x015A, 0x016A, 0x00DC, 0x017B, 0x017D, 0x00DF,
	0x0105, 0x012F, 0x0101, 0x0107, 0x00E4, 0x00E5, 0x0119, 0x0113, 0x010D, 0x00E9, 0x017A, 0x0117, 0x0123, 0x0137, 0x012B, 0x013C,
	0x0161, 0x0144, 0x0146, 0x00F3, 0x014D, 0x00F5, 0x00F6, 0x00F7, 0x0173, 0x0142, 0x015B, 0x016B, 0x00FC, 0x017C, 0x017E, 0x02D9
};

static const uint16_t codes_1258[] =
{
	0x20AC, 0xFFFD, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0xFFFD, 0x2039, 0x0152, 0xFFFD, 0xFFFD, 0xFFFD,
	0xFFFD, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0xFFFD, 0x203A, 0x0153, 0xFFFD, 0xFFFD, 0x0178,
	0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
	0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
	0x00C0, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x0300, 0x00CD, 0x00CE, 0x00CF,
	0x0110, 0x00D1, 0x0309, 0x00D3, 0x00D4, 0x01A0, 0x00D6, 0x00D7, 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x01AF, 0x0303, 0x00DF,
	0x00E0, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x0301, 0x00ED, 0x00EE, 0x00EF,
	0x0111, 0x00F1, 0x0323, 0x00F3, 0x00F4, 0x01A1, 0x00F6, 0x00F7, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x01B0, 0x20AB, 0x00FF
};

enum Endian { Little, Big };
#endif

void xchange_uint16_t_bytes(uint16_t* input)
{
	const uint8_t right = (uint8_t)((*input) & 0xFF);
	const uint8_t left = (uint8_t)((*input) >> 8);
	*input = (uint16_t)(right << 8);
	*input += left;
}

/*void xchange_uint32_t_bytes(uint32_t* input)
{
	const uint8_t right = (uint16_t)((*input) & 0xFFFF);
	const uint8_t left = (uint16_t)((*input) >> 16);
	*input = (uint32_t)(right << 16);
	*input += left;
}*/

#ifndef NO_BUFFER_UNIT
uint8_t text_encoding_get_BOM(
	uint8_t encoding, struct buffer* output)
{
	if (NULL == output)
	{
		return 0;
	}

	switch (encoding)
	{
		case UTF7:
			return 0; /*TODO: */

		case BigEndianUnicode:
		case UTF16BE:
			return buffer_append(output, UTF16BE_BOM, 2);

		case Unicode:
		case UTF16LE:
			return buffer_append(output, UTF16LE_BOM, 2);

		case UTF32BE:
			return buffer_append(output, UTF32BE_BOM, 4);

		case UTF32:
		case UTF32LE:
			return buffer_append(output, UTF32LE_BOM, 4);

		case Default:
		case ASCII:
		case UTF8:
			break;

		default:
			return 0;
	}

	return 1;
}
#endif

uint8_t text_encoding_get_one_of_data_by_BOM(const uint8_t* data, ptrdiff_t data_length)
{
	if (NULL == data ||
		data_length < 2)
	{
		return ASCII;
	}

	/*TODO: UTF7*/
	if (3 < data_length)
	{
		if (0 == memcmp(data, UTF32BE_BOM, 4))
		{
			return UTF32BE;
		}

		if (0 == memcmp(data, UTF32LE_BOM, 4))
		{
			return UTF32LE;
		}
	}

	if (2 < data_length)
	{
		if (0 == memcmp(data, UTF8_BOM, 3))
		{
			return UTF8;
		}
	}

	if (0 == memcmp(data, UTF16BE_BOM, 2))
	{
		return UTF16BE;
	}

	if (0 == memcmp(data, UTF16LE_BOM, 2))
	{
		return UTF16LE;
	}

	return ASCII;
}

uint8_t text_encoding_decode_UTF16BE_single(const uint16_t* input_start, const uint16_t* input_finish,
		uint32_t* output);
uint8_t text_encoding_decode_UTF16LE_single(const uint16_t* input_start, const uint16_t* input_finish,
		uint32_t* output);
uint8_t text_encoding_change_UTF32_endian(const uint32_t* input_start, const uint32_t* input_finish,
		uint32_t* output);
uint8_t text_encoding_decode_UTF32LE_single(const uint32_t* input_start, const uint32_t* input_finish,
		uint32_t* output);
#ifndef NO_BUFFER_UNIT
#define UTF_TO_ASCII(START, FINISH, SIZE, DECODE, TYPE, OUT, OUTPUT)			\
	while ((const TYPE*)(START) < (const TYPE*)(FINISH))						\
	{																			\
		(*(OUT)) = unknown_ASCII_char;											\
		(SIZE) = DECODE((const TYPE*)(START), (const TYPE*)(FINISH), (OUT));	\
		\
		if (!(SIZE))															\
		{																		\
			return 0;															\
		}																		\
		else if (1 < (SIZE) || max_ASCII_char < (*(OUT)))						\
		{																		\
			if (!buffer_push_back((OUTPUT), unknown_ASCII_char))				\
			{																	\
				return 0;														\
			}																	\
		}																		\
		else																	\
		{																		\
			if (!buffer_push_back((OUTPUT), (uint8_t)(*OUT)))					\
			{																	\
				return 0;														\
			}																	\
		}																		\
		\
		START += (SIZE) * sizeof(TYPE);											\
	}

uint8_t text_encoding_UTF_to_ASCII(
	const uint8_t* data_start, const uint8_t* data_finish,
	uint8_t encoding, struct buffer* output)
{
	if (NULL == data_start ||
		NULL == data_finish ||
		NULL == output ||
		data_finish < data_start)
	{
		return 0;
	}

	if (data_finish == data_start)
	{
		return 1;
	}

	ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, (data_finish - data_start) + sizeof(uint32_t)))
	{
		return 0;
	}

	uint32_t* out = (uint32_t*)buffer_data(output, buffer_size(output) - sizeof(uint32_t));

	if (!buffer_resize(output, size))
	{
		return 0;
	}

	switch (encoding)
	{
		case UTF8:
			UTF_TO_ASCII(data_start, data_finish, size, text_encoding_decode_UTF8_single, uint8_t, out, output);
			break;

		case BigEndianUnicode:
		case UTF16BE:
			UTF_TO_ASCII(data_start, data_finish, size, text_encoding_decode_UTF16BE_single, uint16_t, out, output);
			break;

		case Unicode:
		case UTF16LE:
			UTF_TO_ASCII(data_start, data_finish, size, text_encoding_decode_UTF16LE_single, uint16_t, out, output);
			break;

		case UTF32BE:
			UTF_TO_ASCII(data_start, data_finish, size, text_encoding_change_UTF32_endian, uint32_t, out, output);
			break;

		case UTF32:
		case UTF32LE:
			UTF_TO_ASCII(data_start, data_finish, size, text_encoding_decode_UTF32LE_single, uint32_t, out, output);
			break;

		default:
			return 0;
	}

	return 1;
}

#define ASCII_TO_UTF(START, FINISH, INPUT_TYPE, PRE, POST, OUTPUT)							\
	{																						\
		const ptrdiff_t size = buffer_size(OUTPUT);											\
		\
		if (!buffer_append((OUTPUT), NULL, sizeof(INPUT_TYPE) * ((FINISH) - (START))) ||	\
			!buffer_resize((OUTPUT), size))													\
		{																					\
			return 0;																		\
		}																					\
	}																						\
	while ((START) < (FINISH))																\
	{																						\
		static const uint8_t empty[] = { 0, 0, 0 };											\
		\
		const uint8_t* code = max_ASCII_char < (*(START)) ? &unknown_ASCII_char : (START);	\
		\
		if (!buffer_append(output, empty, (PRE)) ||											\
			!buffer_append(output, code, 1) ||												\
			!buffer_append(output, empty, (POST)))											\
		{																					\
			return 0;																		\
		}																					\
		\
		++(START);																			\
	}

uint8_t text_encoding_UTF_from_ASCII(
	const uint8_t* data_start, const uint8_t* data_finish,
	uint8_t encoding, struct buffer* output)
{
	if (NULL == data_start ||
		NULL == data_finish ||
		NULL == output ||
		data_finish < data_start)
	{
		return 0;
	}

	if (data_finish == data_start)
	{
		return 1;
	}

	switch (encoding)
	{
		case UTF8:
			ASCII_TO_UTF(data_start, data_finish, uint8_t, 0, 0, output);
			break;

		case BigEndianUnicode:
		case UTF16BE:
			ASCII_TO_UTF(data_start, data_finish, uint16_t, 1, 0, output);
			break;

		case Unicode:
		case UTF16LE:
			ASCII_TO_UTF(data_start, data_finish, uint16_t, 0, 1, output);
			break;

		case UTF32BE:
			ASCII_TO_UTF(data_start, data_finish, uint32_t, 3, 0, output);
			break;

		case UTF32:
		case UTF32LE:
			ASCII_TO_UTF(data_start, data_finish, uint32_t, 0, 3, output);
			break;

		default:
			return 0;
	}

	return 1;
}
#endif

uint8_t text_encoding_encode_UTF8_single(uint32_t input, uint8_t* output)
{
	if (NULL == output)
	{
		return 0;
	}

	if (input < 0x80)
	{
		output[0] = (uint8_t)input;
		return 1;
	}
	else if (input < 0x800)
	{
		output[1] = input & 0x3F;
		input = input >> 6;
		output[0] = input & 0x1F;
		/**/
		output[0] += 0xC0;
		output[1] += 0x80;
		/**/
		return 2;
	}
	else if (input < 0x10000 && (input < 0xD800 || 0xDFFF < input))
	{
		output[2] = input & 0x3F;
		input = input >> 6;
		output[1] = input & 0x3F;
		input = input >> 6;
		output[0] = input & 0x1F;
		/**/
		output[0] += 0xE0;
		output[1] += 0x80;
		output[2] += 0x80;
		/**/
		return 3;
	}

	for (input = 0; input < 3; ++input)
	{
		output[input] = UTF8_UNKNOWN_CHAR[input];
	}

	return 3;
}

#ifndef NO_BUFFER_UNIT
uint8_t text_encoding_encode_UTF8(
	const uint32_t* data_start, const uint32_t* data_finish,
	struct buffer* output)
{
	if (NULL == data_start ||
		NULL == data_finish ||
		NULL == output ||
		data_finish <= data_start)
	{
		return 0;
	}

	ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, 4 * (data_finish - data_start)) ||
		!buffer_resize(output, size))
	{
		return 0;
	}

	while (data_start < data_finish)
	{
		if (!buffer_append(output, NULL, 4))
		{
			return 0;
		}

		uint8_t* out = buffer_data(output, size);
		size += text_encoding_encode_UTF8_single(*data_start, out);

		if (!buffer_resize(output, size))
		{
			return 0;
		}

		++data_start;
	}

	return 1;
}
#endif

uint8_t text_encoding_is_valid_octet_(uint8_t input)
{
	return 0x9F < input && input < 0xC0;
}

uint8_t text_encoding_decode_UTF8_single(
	const uint8_t* input_start, const uint8_t* input_finish, uint32_t* output)
{
	if (NULL == input_start ||
		NULL == input_finish ||
		NULL == output ||
		input_finish <= input_start)
	{
		return 0;
	}

	(*output) = UTF16LE_UNKNOWN_CHAR;
	const uint8_t octet_1 = (*input_start);

	if (octet_1 < 0x80)
	{
		(*output) = octet_1;
		return 1;
	}
	else if (octet_1 < 0xC0)
	{
		return 1;
	}

	uint8_t octet_2 = 0;
	uint8_t octet_3 = 0;
	uint8_t* octets[2];
	octets[0] = &octet_2;
	octets[1] = &octet_3;
	uint8_t count = 0;

	while (++input_start < input_finish && count < 2)
	{
		const uint8_t input_code = (*input_start);

		if (input_code < 0x80 || 0xBF < input_code)
		{
			break;
		}

		*(octets[count++]) = input_code;
	}

	if (2 == count && (0xDF < octet_1 && octet_1 < 0xF0))
	{
		if (octet_1 < 0xE1 && !text_encoding_is_valid_octet_(octet_2))
		{
			return count;
		}

		(*output) = 0x1000 * (octet_1 & 0x1F);
		(*output) += 0x40 * (octet_2 & 0x3F);
		(*output) += octet_3 & 0x3F;
	}
	else if (1 == count && octet_1 < 0xE0)
	{
		(*output) = 0x40 * (octet_1 & 0x1F);
		(*output) += octet_2 & 0x3F;
	}

	return 1 + count;
}

#ifndef NO_BUFFER_UNIT
uint8_t text_encoding_decode_UTF8(
	const uint8_t* data_start, const uint8_t* data_finish,
	struct buffer* output)
{
	if (NULL == data_start ||
		NULL == data_finish ||
		NULL == output ||
		data_finish <= data_start)
	{
		return 0;
	}

	ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, data_finish - data_start) ||
		!buffer_resize(output, size))
	{
		return 0;
	}

	while (data_start < data_finish)
	{
		if (!buffer_push_back_uint32(output, 0))
		{
			return 0;
		}

		uint32_t* out = (uint32_t*)buffer_data(output, size);
		data_start += text_encoding_decode_UTF8_single(data_start, data_finish, out);
		size += sizeof(uint32_t);
	}

	return 1;
}
#endif

uint8_t text_encoding_encode_UTF16BE_single(uint32_t input_32LE, uint16_t* output)
{
	if (NULL == output)
	{
		return 0;
	}

	if ((0xD7FF < input_32LE && input_32LE < 0xE000) || 0x10FFFF < input_32LE)
	{
		output[0] = UTF16BE_UNKNOWN_CHAR;
		return 1;
	}
	else if (input_32LE < 0x10000)
	{
		output[0] = input_32LE & 0xFFFF;
		xchange_uint16_t_bytes(output);
		return 1;
	}

	input_32LE -= 0x10000;
	/**/
	output[1] = (input_32LE & 0x3FF) & 0xFFFF;
	output[0] = (input_32LE >> 10) & 0xFFFF;
	/**/
	output[0] += 0xD800;
	output[1] += 0xDC00;

	for (input_32LE = 0; input_32LE < 2; ++input_32LE)
	{
		xchange_uint16_t_bytes(output);
		++output;
	}

	return 2;
}

uint8_t text_encoding_encode_UTF16LE_single(uint32_t input, uint16_t* output)
{
	if (NULL == output)
	{
		return 0;
	}

	if ((0xD7FF < input && input < 0xE000) || 0x10FFFF < input)
	{
		output[0] = UTF16LE_UNKNOWN_CHAR;
		return 1;
	}
	else if (input < 0x10000)
	{
		output[0] = input & 0xFFFF;
		return 1;
	}

	input -= 0x10000;
	/**/
	output[1] = (input & 0x3FF) & 0xFFFF;
	output[0] = (input >> 10) & 0xFFFF;
	/**/
	output[0] += 0xD800;
	output[1] += 0xDC00;
	/**/
	return 2;
}

#ifndef NO_BUFFER_UNIT
uint8_t text_encoding_encode_UTF16(
	const uint32_t* data_start, const uint32_t* data_finish,
	uint8_t endian, struct buffer* output)
{
	if (NULL == data_start ||
		NULL == data_finish ||
		NULL == output ||
		data_finish <= data_start)
	{
		return 0;
	}

	uint8_t(*encode_function)(uint32_t, uint16_t*) =
		(Little == endian) ? &text_encoding_encode_UTF16LE_single : &text_encoding_encode_UTF16BE_single;
	ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, 2 * sizeof(uint16_t) * (data_finish - data_start)) ||
		!buffer_resize(output, size))
	{
		return 0;
	}

	while (data_start < data_finish)
	{
		if (!buffer_append(output, NULL, 2 * sizeof(uint16_t)))
		{
			return 0;
		}

		uint16_t* out = (uint16_t*)buffer_data(output, size);
		size += sizeof(uint16_t) * (encode_function)(*data_start, out);

		if (!buffer_resize(output, size))
		{
			return 0;
		}

		++data_start;
	}

	return 1;
}
#endif

uint8_t text_encoding_decode_UTF16BE_single(const uint16_t* input_start, const uint16_t* input_finish,
		uint32_t* output)
{
	if (NULL == input_start ||
		NULL == input_finish ||
		NULL == output ||
		input_finish <= input_start)
	{
		return 0;
	}

	uint16_t input_16LE = (*input_start);
	xchange_uint16_t_bytes(&input_16LE);

	if (input_16LE < 0xD800 || 0xDFFF < input_16LE)
	{
		(*output) = input_16LE;
		return 1;
	}
	else if (0xDBFF < input_16LE)
	{
		(*output) = UTF16LE_UNKNOWN_CHAR;
		return 1;
	}

	(*output) = input_16LE & 0x3FF;
	(*output) = (*output) << 10;
	++input_start;

	if (input_start == input_finish)
	{
		(*output) = UTF16LE_UNKNOWN_CHAR;
	}
	else
	{
		input_16LE = (*input_start);

		if (input_16LE < 0xDC00 || 0xDFFF < input_16LE)
		{
			(*output) = UTF16LE_UNKNOWN_CHAR;
		}
		else
		{
			(*output) += (input_16LE & 0x3FF);
			(*output) += 0x10000;
		}
	}

	return 2;
}

uint8_t text_encoding_decode_UTF16LE_single(const uint16_t* input_start, const uint16_t* input_finish,
		uint32_t* output)
{
	if (NULL == input_start ||
		NULL == input_finish ||
		NULL == output ||
		input_finish <= input_start)
	{
		return 0;
	}

	if ((*input_start) < 0xD800 || 0xDFFF < (*input_start))
	{
		(*output) = (*input_start);
		return 1;
	}
	else if (0xDBFF < (*input_start))
	{
		(*output) = UTF16LE_UNKNOWN_CHAR;
		return 1;
	}

	(*output) = (*input_start) & 0x3FF;
	(*output) = (*output) << 10;
	++input_start;

	if (input_start == input_finish)
	{
		(*output) = UTF16LE_UNKNOWN_CHAR;
	}
	else
	{
		if ((*input_start) < 0xDC00 || 0xDFFF < (*input_start))
		{
			(*output) = UTF16LE_UNKNOWN_CHAR;
		}
		else
		{
			(*output) += ((*input_start) & 0x3FF);
			(*output) += 0x10000;
		}
	}

	return 2;
}

#ifndef NO_BUFFER_UNIT
uint8_t text_encoding_decode_UTF16(
	const uint16_t* data_start, const uint16_t* data_finish,
	uint8_t endian, struct buffer* output)
{
	if (NULL == data_start ||
		NULL == data_finish ||
		NULL == output ||
		data_finish <= data_start)
	{
		return 0;
	}

	uint8_t(*decode_function)(const uint16_t*, const uint16_t*, uint32_t*) =
		(Little == endian) ? &text_encoding_decode_UTF16LE_single : &text_encoding_decode_UTF16BE_single;
	ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, data_finish - data_start) ||
		!buffer_resize(output, size))
	{
		return 0;
	}

	while (data_start < data_finish)
	{
		if (!buffer_push_back_uint32(output, 0))
		{
			return 0;
		}

		uint32_t* out = (uint32_t*)buffer_data(output, size);
		data_start += (decode_function)(data_start, data_finish, out);
		size += sizeof(uint32_t);
	}

	return 1;
}
#endif

uint8_t text_encoding_change_UTF32_endian(const uint32_t* input_start, const uint32_t* input_finish,
		uint32_t* output)
{
	if (NULL == input_start ||
		NULL == input_finish ||
		NULL == output ||
		input_finish <= input_start/* ||
		1 < input_finish - input_start*/)
	{
		return 0;
	}

	(*output) = (*input_start);
	/**/
	uint16_t right = (uint16_t)((*output) & 0xFFFF);
	uint16_t left = (uint16_t)((*output) >> 16);
	/**/
	xchange_uint16_t_bytes(&right);
	xchange_uint16_t_bytes(&left);
	/**/
	(*output) = (uint32_t)(right << 16);
	(*output) += left;
	/**/
	return 1;
}

uint8_t text_encoding_decode_UTF32LE_single(const uint32_t* input_start, const uint32_t* input_finish,
		uint32_t* output)
{
	if (NULL == input_start ||
		NULL == input_finish ||
		NULL == output ||
		input_finish <= input_start)
	{
		return 0;
	}

	(*output) = (*input_start);
	/**/
	return 1;
}

#ifndef NO_BUFFER_UNIT
uint8_t text_encoding_UTF8_to_code_page(
	const uint8_t* data_start, const uint8_t* data_finish,
	uint16_t code_page, struct buffer* output)
{
	if (NULL == data_start ||
		NULL == data_finish ||
		NULL == output ||
		data_finish <= data_start)
	{
		return 0;
	}

	const uint16_t* ptr = NULL;
	const uint8_t max_index = Windows_874 == code_page ? 32 : 128;

	switch (code_page)
	{
		case Windows_874:
			ptr = codes_874;
			break;

		case Windows_1250:
			ptr = codes_1250;
			break;

		case Windows_1251:
			ptr = codes_1251;
			break;

		case Windows_1252:
			ptr = codes_1252;
			break;

		case Windows_1253:
			ptr = codes_1253;
			break;

		case Windows_1254:
			ptr = codes_1254;
			break;

		case Windows_1255:
			ptr = codes_1255;
			break;

		case Windows_1256:
			ptr = codes_1256;
			break;

		case Windows_1257:
			ptr = codes_1257;
			break;

		case Windows_1258:
			ptr = codes_1258;
			break;

		default:
			return 0;
	}

	ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, (data_finish - data_start) + sizeof(uint32_t)) ||
		!buffer_resize(output, size))
	{
		return 0;
	}

	while (data_start < data_finish)
	{
		if (!buffer_push_back_uint32(output, 0))
		{
			return 0;
		}

		uint32_t* out = (uint32_t*)buffer_data(output, size);
		uint32_t code = text_encoding_decode_UTF8_single(data_start, data_finish, out);

		if (!code || !buffer_resize(output, size))
		{
			return 0;
		}

		data_start += code;
		code = *out;

		if (code < min_non_ASCII_char)
		{
			if (!buffer_push_back(output, (uint8_t)code))
			{
				return 0;
			}
		}
		else
		{
			for (uint8_t i = 0; i < max_index; ++i)
			{
				if (UTF16LE_UNKNOWN_CHAR == ptr[i])
				{
					continue;
				}

				if (!text_encoding_decode_UTF16LE_single(&(ptr[i]), &(ptr[i]) + 1, out))
				{
					return 0;
				}

				if ((*out) == code)
				{
					if (!buffer_push_back(output, min_non_ASCII_char + i))
					{
						return 0;
					}

					code = 0;
					break;
				}
			}

			if (0 != code)
			{
				if (!buffer_push_back(output, unknown_ASCII_char))
				{
					return 0;
				}
			}
		}

		++size;
	}

	return 1;
}

uint8_t text_encoding_UTF8_from_code_page(
	const uint8_t* data_start, const uint8_t* data_finish,
	uint16_t code_page, struct buffer* output)
{
	if (NULL == data_start ||
		NULL == data_finish ||
		NULL == output ||
		data_finish <= data_start)
	{
		return 0;
	}

	const uint16_t* ptr = NULL;
	const uint8_t max_index = Windows_874 == code_page ? 32 : 128;

	switch (code_page)
	{
		case Windows_874:
			ptr = codes_874;
			break;

		case Windows_1250:
			ptr = codes_1250;
			break;

		case Windows_1251:
			ptr = codes_1251;
			break;

		case Windows_1252:
			ptr = codes_1252;
			break;

		case Windows_1253:
			ptr = codes_1253;
			break;

		case Windows_1254:
			ptr = codes_1254;
			break;

		case Windows_1255:
			ptr = codes_1255;
			break;

		case Windows_1256:
			ptr = codes_1256;
			break;

		case Windows_1257:
			ptr = codes_1257;
			break;

		case Windows_1258:
			ptr = codes_1258;
			break;

		default:
			return 0;
	}

	ptrdiff_t size = buffer_size(output);
	uint32_t* out = NULL;

	if (!buffer_append(output, NULL, (data_finish - data_start) + sizeof(uint32_t)) ||
		NULL == (out = (uint32_t*)buffer_data(output, buffer_size(output) - sizeof(uint32_t))) ||
		!buffer_resize(output, size))
	{
		return 0;
	}

	while (data_start < data_finish)
	{
		uint8_t code = (*data_start);

		if (code < min_non_ASCII_char)
		{
			if (!buffer_push_back(output, code))
			{
				return 0;
			}
		}
		else
		{
			uint16_t table_code;

			if (code < min_non_ASCII_char + max_index &&
				UTF16LE_UNKNOWN_CHAR != (table_code = ptr[code - min_non_ASCII_char]))
			{
				if (!text_encoding_decode_UTF16LE_single(&table_code, &table_code + 1, out))
				{
					return 0;
				}

				size = text_encoding_encode_UTF8_single((*out), (uint8_t*)out);

				if (!size ||
					!buffer_append(output, (const uint8_t*)out, size))
				{
					return 0;
				}
			}
			else
			{
				if (!buffer_push_back(output, unknown_ASCII_char))
				{
					return 0;
				}
			}
		}

		data_start++;
	}

	return 1;
}

uint8_t text_encoding_UTF8_to_UTF16BE(const uint8_t* data_start, const uint8_t* data_finish,
									  struct buffer* output)
{
	if (NULL == data_start ||
		NULL == data_finish ||
		data_finish <= data_start ||
		NULL == output)
	{
		return 0;
	}

	ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, sizeof(uint16_t) * (data_finish - data_start) + sizeof(uint32_t)))
	{
		return 0;
	}

	uint32_t* decoded_UTF8 = (uint32_t*)buffer_data(output, buffer_size(output) - sizeof(uint32_t));

	if ((data_start <= (uint8_t*)decoded_UTF8 &&
		 (uint8_t*)decoded_UTF8 < data_finish) ||
		!buffer_resize(output, size))
	{
		return 0;
	}

	while (data_start < data_finish)
	{
		if (!buffer_append(output, NULL, 2 * sizeof(uint16_t)))
		{
			return 0;
		}

		uint16_t* out = (uint16_t*)buffer_data(output, size);
		data_start += text_encoding_decode_UTF8_single(data_start, data_finish, decoded_UTF8);
		size += sizeof(uint16_t) * text_encoding_encode_UTF16BE_single(*decoded_UTF8, out);

		if (!buffer_resize(output, size))
		{
			return 0;
		}
	}

	return 1;
}

uint8_t text_encoding_UTF8_to_UTF16LE(const uint8_t* data_start, const uint8_t* data_finish,
									  struct buffer* output)
{
	if (NULL == data_start ||
		NULL == data_finish ||
		data_finish <= data_start ||
		NULL == output)
	{
		return 0;
	}

	ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, sizeof(uint16_t) * (data_finish - data_start) + sizeof(uint32_t)))
	{
		return 0;
	}

	uint32_t* decoded_UTF8 = (uint32_t*)buffer_data(output, buffer_size(output) - sizeof(uint32_t));

	if ((data_start <= (uint8_t*)decoded_UTF8 &&
		 (uint8_t*)decoded_UTF8 < data_finish) ||
		!buffer_resize(output, size))
	{
		return 0;
	}

	while (data_start < data_finish)
	{
		if (!buffer_append(output, NULL, 2 * sizeof(uint16_t)))
		{
			return 0;
		}

		uint16_t* out = (uint16_t*)buffer_data(output, size);
		data_start += text_encoding_decode_UTF8_single(data_start, data_finish, decoded_UTF8);
		size += sizeof(uint16_t) * text_encoding_encode_UTF16LE_single(*decoded_UTF8, out);

		if (!buffer_resize(output, size))
		{
			return 0;
		}
	}

	return 1;
}

uint8_t text_encoding_UTF16BE_to_UTF8(const uint16_t* data_start, const uint16_t* data_finish,
									  struct buffer* output)
{
	if (NULL == data_start ||
		NULL == data_finish ||
		data_finish <= data_start ||
		NULL == output)
	{
		return 0;
	}

	ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, 4 * (data_finish - data_start) + sizeof(uint32_t)))
	{
		return 0;
	}

	uint32_t* decoded_UTF16BE = (uint32_t*)buffer_data(output, buffer_size(output) - sizeof(uint32_t));

	if ((data_start <= (uint16_t*)decoded_UTF16BE &&
		 (uint16_t*)decoded_UTF16BE < data_finish) ||
		!buffer_resize(output, size))
	{
		return 0;
	}

	while (data_start < data_finish)
	{
		if (!buffer_append(output, NULL, 4))
		{
			return 0;
		}

		uint8_t* out = buffer_data(output, size);
		data_start += text_encoding_decode_UTF16BE_single(data_start, data_finish, decoded_UTF16BE);
		size += text_encoding_encode_UTF8_single(*decoded_UTF16BE, out);

		if (!buffer_resize(output, size))
		{
			return 0;
		}
	}

	return 1;
}

uint8_t text_encoding_UTF16LE_to_UTF8(const uint16_t* data_start, const uint16_t* data_finish,
									  struct buffer* output)
{
	if (NULL == data_start ||
		NULL == data_finish ||
		data_finish <= data_start ||
		NULL == output)
	{
		return 0;
	}

	ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, 4 * (data_finish - data_start) + sizeof(uint32_t)))
	{
		return 0;
	}

	uint32_t* decoded_UTF16LE = (uint32_t*)buffer_data(output, buffer_size(output) - sizeof(uint32_t));

	if ((data_start <= (uint16_t*)decoded_UTF16LE &&
		 (uint16_t*)decoded_UTF16LE < data_finish) ||
		!buffer_resize(output, size))
	{
		return 0;
	}

	while (data_start < data_finish)
	{
		if (!buffer_append(output, NULL, 4))
		{
			return 0;
		}

		uint8_t* out = buffer_data(output, size);
		data_start += text_encoding_decode_UTF16LE_single(data_start, data_finish, decoded_UTF16LE);
		size += text_encoding_encode_UTF8_single(*decoded_UTF16LE, out);

		if (!buffer_resize(output, size))
		{
			return 0;
		}
	}

	return 1;
}

uint8_t text_encoding_UTF8_to_UTF32BE(const uint8_t* data_start, const uint8_t* data_finish,
									  struct buffer* output)
{
	if (NULL == data_start ||
		NULL == data_finish ||
		NULL == output ||
		data_finish <= data_start)
	{
		return 0;
	}

	ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, data_finish - data_start) ||
		!buffer_resize(output, size))
	{
		return 0;
	}

	while (data_start < data_finish)
	{
		if (!buffer_push_back_uint32(output, 0))
		{
			return 0;
		}

		uint32_t* out = (uint32_t*)buffer_data(output, size);
		data_start += text_encoding_decode_UTF8_single(data_start, data_finish, out);

		if (!text_encoding_change_UTF32_endian(out, out + 1, out))
		{
			return 0;
		}

		size += sizeof(uint32_t);
	}

	return 1;
}

uint8_t text_encoding_UTF32BE_to_UTF8(const uint32_t* data_start, const uint32_t* data_finish,
									  struct buffer* output)
{
	if (NULL == data_start ||
		NULL == data_finish ||
		NULL == output ||
		data_finish <= data_start)
	{
		return 0;
	}

	ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, 4 * (data_finish - data_start)) ||
		!buffer_resize(output, size))
	{
		return 0;
	}

	while (data_start < data_finish)
	{
		if (!buffer_append(output, NULL, 4))
		{
			return 0;
		}

		uint32_t data_start_32LE = *data_start;

		if (!text_encoding_change_UTF32_endian(&data_start_32LE, &data_start_32LE + 1, &data_start_32LE))
		{
			return 0;
		}

		uint8_t* out = buffer_data(output, size);
		size += text_encoding_encode_UTF8_single(data_start_32LE, out);

		if (!buffer_resize(output, size))
		{
			return 0;
		}

		++data_start;
	}

	return 1;
}
#endif
