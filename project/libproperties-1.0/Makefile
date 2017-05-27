#------------------------------------------------------------ 
# Created By harite for mingw32 on win32 and GCC on other OS
# Mailto:Harite.K@gmail.com
# For libproperties v1.0
# http://code.google.com/p/libproperties
# Date:05/10/2007
#
# When gcc's ld links the objects, it has bugs.
# This trouble me when write the Makefile! In fact, i spend lots of time to learn it.
#
# test.c
# int f(){return 3;}
# %gcc -c test.c -o test.o
# %ar cfs libtest.a test.o
# main.c
# int main(){return f();}
# %gcc -c main.c -o main.o
# Not work command
# %gcc -o main -L. -ltest main.o
# main.o: In function 'main':
# main.o(.text+0x4): undefined reference to 'f'
# collect2: ld return 1 exit status
# The error message indicates that even though libtest.a contains a definition of f,the
# linker did not find it.That¡¯s because libtest.a was searched when it was first encoun-
# tered,and at that point the linker hadn¡¯t seen any references to f.
# On the other hand,if we use this line,no error messages are issued:
# %gcc -o app app.o -L. ¨Cltest
# The reason is that the reference to f in app.o causes the linker to include the test.o
# object file from the libtest.a archive.
# 
#
#------------------------------------------------------------

CPP = g++
RM = rm -rf
MD = mkdir
AR = ar

ARFLAGS = crs
DEBUGFLAGS = -g -O2
RELEASEFLAGS = -O3 -DNDEBUG -s

MINGW32_DLLWRAP = dllwrap
MINGW32_DLLWRAP_FLAGS = --no-export-all-symbols --add-stdcall-alias --enable-auto-import
MINGW32_BUILD_DYNAMIC_FLAGS = -DLIBPROPERTIES_BUILDING_DLL

BUILD_STATIC_DIR = static_lib
BUILD_DYNAMIC_DIR = mingw32_dll
BUILD_SHARED_DIR = shared_lib
BUILD_INCLUDE_DIR = include
BUILD_LIBPROPERTIES_SRC_DIR = src/libproperties
BUILD_TINYXML_SRC_DIR = src/tinyxml
TEST_SRC_DIR = src/test

TESTOUTFILE = testlibproperties

#gcc's bug
#if want to link rightly, properties.o MUST at the end of gcc's param
#but heigher version right.
LIBPROPERTIESOBJS = libproperties.o tinystr.o tinyxml.o libproperties_tinyxml_ext.o \
                   tinyxmlerror.o tinyxmlparser.o

LIBPROPERTIESSRCCPPS = $(BUILD_LIBPROPERTIES_SRC_DIR)/libproperties.cpp \
                   $(BUILD_TINYXML_SRC_DIR)/tinystr.cpp \
                   $(BUILD_TINYXML_SRC_DIR)/tinyxml.cpp \
		   $(BUILD_LIBPROPERTIES_SRC_DIR)/libproperties_tinyxml_ext.cpp \
                   $(BUILD_TINYXML_SRC_DIR)/tinyxmlerror.cpp \
		   $(BUILD_TINYXML_SRC_DIR)/tinyxmlparser.cpp

STATIC_LIB_NAME = libproperties.a
DYNAMIC_IMPLIB_MINGW32_NAME = libproperties.a
MINGW32_DLL_DEFFILE = libproperties.def
DYNAMIC_DLL_MINGW32_NAME = libproperties.dll
DYNAMIC_DLL_GCC_NAME = libproperties.so

CPPFLAGS = -Wall -DTIXML_USE_STL $(RELEASEFLAGS) -I$(BUILD_INCLUDE_DIR)

#.PHONY: all help readyforbuild test static

all: help

help:
	@echo =======================================================
	@echo *  Makefile for Mingw32 on win32 and GCC on other OS  *
	@echo *     libproperties v1.0 Made By harite               *
	@echo *     Mailto:Harite.K@gmail.com                       *
	@echo *     Project Homepage:                               *
	@echo *        http://code.google.com/p/libproperties       *
	@echo *   Build it:                                         *
	@echo *   Make Static lib:                                  *
	@echo *                   make static                       *
	@echo *   Make Shared lib in GCC:                           *
	@echo *                   make shared                       *
	@echo =======================================================

static: 
	-@$(RM) $(BUILD_STATIC_DIR)
	@echo [Clean workspace Done]
	@$(MD) $(BUILD_STATIC_DIR)
	@echo [Make workspace Done]
	@$(CPP) $(CPPFLAGS) -c $(LIBPROPERTIESSRCCPPS)
	@echo [Make libproperties objects Done]
	@$(AR) $(ARFLAGS) $(BUILD_STATIC_DIR)/$(STATIC_LIB_NAME) $(LIBPROPERTIESOBJS)
	@echo [Make static lib Done]
	@$(RM) $(LIBPROPERTIESOBJS)
	@echo [Clean libproperties objects Done]
	@$(CPP) $(CPPFLAGS) -o $(BUILD_STATIC_DIR)/$(TESTOUTFILE) $(TEST_SRC_DIR)/testproperties.cpp -L./$(BUILD_STATIC_DIR) -lproperties
	@echo [Make test program using static lib Done]

# want to build dll in win32 with mingw32, but fail.
# if you do it, mail me or find us website
dll:
	-@$(RM) $(BUILD_DYNAMIC_DIR)
	@echo [Clean workspace Done]
	@$(MD) $(BUILD_DYNAMIC_DIR)
	@echo [Make workspace Done]
	@$(CPP) $(CPPFLAGS) $(MINGW32_BUILD_DYNAMIC_FLAGS) -c $(LIBPROPERTIESSRCCPPS)
	@$(MINGW32_DLLWRAP) --output-def $(BUILD_DYNAMIC_DIR)/$(MINGW32_DLL_DEFFILE) --driver-name c++ --implib $(BUILD_DYNAMIC_DIR)/$(DYNAMIC_IMPLIB_MINGW32_NAME) $(LIBPROPERTIESOBJS) $(MINGW32_DLLWRAP_FLAGS) -o $(BUILD_DYNAMIC_DIR)/$(DYNAMIC_DLL_MINGW32_NAME)
	@echo [Make dll and its input lib Done]
	@$(RM) $(LIBPROPERTIESOBJS)
	@echo [Clean libproperties objects Done]
	@$(CPP) $(CPPFLAGS) -o $(BUILD_DYNAMIC_DIR)/$(TESTOUTFILE) $(TEST_SRC_DIR)/testproperties.cpp -L./$(BUILD_DYNAMIC_DIR) -lproperties
	@echo [Make test program using dll Done]

shared:
	-@$(RM) $(BUILD_SHARED_DIR)
	@echo [Clean workspace Done]
	@$(MD) $(BUILD_SHARED_DIR)
	@echo [Make workspace Done]
	@$(CPP) $(CPPFLAGS) -fPIC -c $(LIBPROPERTIESSRCCPPS)
	@$(CPP) -shared -fPIC -o $(BUILD_SHARED_DIR)/$(DYNAMIC_DLL_GCC_NAME) $(LIBPROPERTIESSRCCPPS)
	@echo [Make shared lib Done]
	@$(RM) $(LIBPROPERTIESOBJS)
	@echo [Clean libproperties objects Done]
	@$(CPP) $(CPPFLAGS) -o $(BUILD_SHARED_DIR)/$(TESTOUTFILE) $(TEST_SRC_DIR)/testproperties.cpp -L./$(BUILD_SHARED_DIR) -lproperties
	@echo [Make test program using shared lib Done]

clean: 
	-@$(RM) $(BUILD_STATIC_DIR) $(BUILD_DYNAMIC_DIR) $(BUILD_SHARED_DIR)
	@echo Clean Done

