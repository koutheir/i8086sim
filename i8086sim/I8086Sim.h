
#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>

#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>
#include <deque>

#include <stdlib.h>
#include <errno.h>
#include <conio.h>

using namespace std;

typedef unsigned char	byte;
typedef unsigned short	ushort;
typedef unsigned long	ulong;
typedef unsigned int		uint;


#define LogErr(s)		fprintf(stderr, "Error: %s\n   (%s)\n", s, strerror(errno))


extern char *g_FileName;
extern bool	g_StepByStep;

extern bool ParseCmdLine(int argc, char *argv[]);

#include "KConsoleLock.h"
#include "KVMIO.h"
#include "K8086VM.h"
