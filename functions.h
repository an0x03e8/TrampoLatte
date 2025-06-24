#pragma once
#include <windows.h>

extern void RetStub(void);
extern void GenericStub(void);

BOOL TrampolineX64(IN PVOID pHookedFunction, IN PVOID pDetourFunction, OUT PVOID* pOriginalFunc, OUT DWORD* dwOldProt);
BOOL RemoveTrampoline(IN PVOID pHookedFunc, IN PVOID pOriginalFunction, IN DWORD dwOldProtect);
VOID BlockExecutionFlow(PCONTEXT pCtx);
BOOL GetEtwpEventWriteFull(OUT PVOID* ppFunction);
BOOL GetContent(OUT PBYTE* pPayload, OUT SIZE_T* sSizeOfPayload);