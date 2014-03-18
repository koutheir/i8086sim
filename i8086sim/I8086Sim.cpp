
#include "i8086sim.h"


char *g_FileName=NULL;
bool	g_StepByStep=false;

void DisplayUsageHelp()
{
	puts(
		"I8086SIM - Intel 8086 Processor Simulator\n"
		"Created by Koutheir Attouchi, 2009\n"
		"Usage: I8086SIM [-h] [-s] filename\n"
		"   -h        Display usage help.\n"
		"   -s        Execute step by step.\n"
		"   filename  Executable file to simulate. Can be .EXE or .COM file.\n"
		);
}


bool ParseCmdLine(int argc, char *argv[])
{
	if (argc < 2) {DisplayUsageHelp(); return false;}

	bool ShouldDisplayUsageHelp=false;

	for (int i=1; i<argc; i++) {
		if (*argv[i] == '-' || *argv[i] == '/') {
			switch (argv[i][1]) {
			case 'h':
			case 'H':
				ShouldDisplayUsageHelp = true;
				break;

			case 's':
			case 'S':
				g_StepByStep = true;
				break;
			}
		} else {
			size_t len = strlen(argv[i])+1;
			g_FileName = new char[len];
			if (!g_FileName) return false;
			memmove(g_FileName, argv[i], len);
		}
	}

	if (ShouldDisplayUsageHelp) DisplayUsageHelp();
	return true;
}
