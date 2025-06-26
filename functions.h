#pragma once
#include <windows.h>
#include <amsi.h>

extern void RetStub(void);

// The proxy function
typedef HRESULT(WINAPI* PFN_AMSI_SCAN_BUFFER)(
	HAMSICONTEXT,          /* RCX */
	PVOID  Buffer,         /* RDX */
	ULONG  Length,         /* R8  */
	LPCWSTR ContentName,   /* R9  */
	HAMSISESSION,          /* [rsp+28h]  5-th */
	AMSI_RESULT* Result);  /* [rsp+30h]  6-th */

static PFN_AMSI_SCAN_BUFFER  g_pOriginalAmsiFunc = NULL;

BOOL TrampolineX64(IN PVOID pHookedFunction, IN PVOID pDetourFunction, OUT PVOID* pOriginalFunc, OUT DWORD* dwOldProt);
BOOL RemoveTrampoline(IN PVOID pHookedFunc, IN PVOID pOriginalFunction, IN DWORD dwOldProtect);
VOID BlockExecutionFlow(PCONTEXT pCtx);
BOOL GetEtwpEventWriteFull(OUT PVOID* ppFunction);
HRESULT WINAPI ProxyAmsi(HAMSICONTEXT ctx, PVOID buf, ULONG len, LPCWSTR name, HAMSISESSION sess, AMSI_RESULT* pRes);