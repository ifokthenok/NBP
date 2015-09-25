//============================================================================
// Name        : GooglePinyin.cpp
// Author      : Yu Hao
// Version     : 0.1
// Copyright   : Your copyright notice
// Description : Just a test for Google Pinyin
//============================================================================

#include <cstring>
#include <cstdio>
#include <string>
#include <iostream>
#include "./include/pinyinime.h"
#include "ConvertUTF.h"

using namespace ime_pinyin;

UTF8 g_utf8_buf[1024] = { 0 };

const char* toUTF8(const char16* src, size_t length) {
	UTF16 *utf16Start = (UTF16 *) src;
	UTF16 *utf16End = (UTF16 *) (src + length);
	UTF8* utf8Start = g_utf8_buf;
	ConvertUTF16toUTF8((const UTF16 **) &utf16Start, utf16End, &utf8Start,
	  &(g_utf8_buf[1024]), strictConversion);
	return (const char*) g_utf8_buf;
}

int main(int argc, char* argv[])
{
	bool success = im_open_decoder("./data/dict_pinyin.dat", "./data/dict_usr.txt");
	std::string py;
	int select = 0;
	std::cin >> py;
	size_t hzCount = im_search(py.c_str(), py.size());
	size_t pyDecode;
	char16 hzUtf16[32] = {0};
	UTF8 hz[64] = {0};
	const char* pyStr = im_get_sps_str(&pyDecode);
	size_t pyFixed = im_get_fixed_len();
	const uint16* startPos = 0;
	size_t len = im_get_spl_start_pos(startPos);
	std::cout << "PyStr=" << pyStr << " DecodeLen=" << pyDecode
			<< " FixedLen=" << pyFixed << " StartPos=[";
	for(int i=0; i <= len; i++) {
		std::cout << "  " << startPos[i];
	}
	std::cout << "  ]" << std::endl;
	for (int i=0; i < hzCount; i++) {
		im_get_candidate(i, hzUtf16, 32);
		std::cout << toUTF8(hzUtf16,32) << " ";
	}
	std::cout << std::endl;
	std::cin >> select;
	while (select != -1) {
		hzCount = im_choose(select);
		pyStr = im_get_sps_str(&pyDecode);
		pyFixed = im_get_fixed_len();
		len = im_get_spl_start_pos(startPos);
		std::cout << "PyStr=" << pyStr << " DecodeLen=" << pyDecode
				<< " FixedLen=" << pyFixed << " StartPos=[";
		for(int i=0; i <= len; i++) {
			std::cout << "  " << startPos[i];
		}
		std::cout << "  ]" << std::endl;
		for (int i=0; i < hzCount; i++) {
			im_get_candidate(i, hzUtf16, 32);
			std::cout << toUTF8(hzUtf16,32) << " ";
		}
		std::cout << std::endl;
		std::cin >> select;
	}
	im_close_decoder();
	return 0;
}
