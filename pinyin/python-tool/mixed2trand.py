#!/usr/bin/env python2.7

import os

utf16_dict_file ="../data/rawdict_utf16_65105_freq.txt"
utf16_freq_file_tw ="../data/rawdict_utf16_65105_freq_tw.txt"
opencc_home = "/home/hyu/opencc-0.4.3"
t2s_char_file = "%s/data/trad_to_simp/characters.txt" % (opencc_home)
s2t_char_file = "%s/data/simp_to_trad/characters.txt" % (opencc_home)

if __name__ == "__main__":
	
	# Covert raw dirct from UTF16 to UTF8
	cmd = "iconv -f UTF16 -t UTF8 %s -o ./utf8_dict.txt" % (utf16_dict_file)
	if 0 != os.system(cmd):
		print "iconv error: can't convert file %s from UTF16 to UTF8" % (utf16_dict_file)
	
	# Get all trad chars
	t2s_file = open(t2s_char_file, 'r')
	trad_char_set = set()
	for line in t2s_file:
		words = line.split()
		if words != []:
			trad_char_set.add(words[0])
	t2s_file.close()

	# Get all simple chars
	s2t_file = open(s2t_char_file, 'r')
	simple_char_set = set()
	for line in s2t_file:
		words = line.split()
		if words != []:
			simple_char_set.add(words[0])
	s2t_file.close()

	# Filter trad chars
	dict_filter_file = open("./utf8_dict_filter.txt", "w")
	dict_phrase_file = open("./utf8_dict_phrase.txt", "w")
	char_file = open("./utf8_dict.txt", "r")
	for line in char_file:
		words = line.split()
		if len(words[0].decode("utf-8")) > 1:
			dict_phrase_file.write(line)
		elif (words[0] in trad_char_set): 
			dict_filter_file.write(line)
		elif not(words[0] in simple_char_set):
			trad_char_set.add(words[0])
			dict_filter_file.write(line)
	char_file.close()
	dict_phrase_file.close()

	# Covert phrase from Simple to Trad		
	cmd = "opencc -i utf8_dict_phrase.txt -o utf8_dict_trad_phrase.txt -c %s/data/config/zhs2zht.ini" % (opencc_home)
	if 0 != os.system(cmd):
		print "opencc error: can't translate simple to trad"

	# Fiter trad phrase
	dict_trad_phrase_file = open("utf8_dict_trad_phrase.txt")
	for line in dict_trad_phrase_file:
		words = line.split()
		hzs = words[0].decode("utf-8")
		for hz in hzs:
			if hz.encode("utf-8") in trad_char_set:
				dict_filter_file.write(line)
				break
	dict_trad_phrase_file.close()
	dict_filter_file.close()
	
	# Covert dirct from UTF8 to UTF16
	cmd = "iconv -f UTF8 -t UTF16 ./utf8_dict_filter.txt -o %s" % (utf16_freq_file_tw)
	if 0 != os.system(cmd):
		print "iconv error: can't generate utf16 file %s" % (utf16_freq_file_tw)

	# Remove generated intermediate files
	cmd = "rm -rf ./utf8_dict.txt ./utf8_dict_filter.txt ./utf8_dict_phrase.txt ./utf8_dict_trad_phrase.txt"
	os.system(cmd)
