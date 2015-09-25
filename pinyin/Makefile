CC=gcc
CFLAGS= -g -Wall -std=c99
CPP=g++
CPPFLAGS= -g3 -Wall
LINKFLAGS= -lpthread
PINYINIME_TEST=pinyinime_test

PINYINIME_SRC= \
	share/dictbuilder.cpp \
	share/dictlist.cpp \
	share/dicttrie.cpp \
	share/lpicache.cpp \
	share/matrixsearch.cpp \
	share/mystdlib.cpp \
	share/ngram.cpp \
	share/pinyinime.cpp \
	share/searchutility.cpp \
	share/spellingtable.cpp \
	share/spellingtrie.cpp \
	share/splparser.cpp \
	share/userdict.cpp \
	share/utf16char.cpp \
	share/utf16reader.cpp \
	share/sync.cpp

all: test

test: $(PINYINIME_TEST)

$(PINYINIME_TEST): $(PINYINIME_SRC) pinyinime_test.cpp ConvertUTF.cpp
	@$(CPP) $(CPPFLAGS) -o $@ $? $(LINKFLAGS) -std=c++0x

clean:
	-rm -rf $(PINYINIME_TEST)

.PHONY: clean