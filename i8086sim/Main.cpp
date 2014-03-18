
#include "i8086sim.h"



int main(int argc, char *argv[])
{
	SetConsoleTitle("I8086SIM - Simulator");

	if (!ParseCmdLine(argc, argv)) return -1;

	K8086VM *kvm;
	K8086VM::CreateVMLoader(&kvm, g_FileName);
	kvm->LoadFile(g_FileName);
	kvm->Execute((g_StepByStep ? KEF_StepByStep : KEF_None));
	delete kvm;

	_getch();
	return 0;
}
