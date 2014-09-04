// ipc_dll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ipc_dll.h"
#include "ipc_factory_impl.h"


IPC::FactoryImpl* gFactory = NULL;

bool GetIPCEndPoint(void** instance, const char* name, void* listener /*= 0*/)
{
	if (!gFactory)
		return false;
	IPC::IEndpoint* endpoint = gFactory->GetEndPoint(name, (IPC::IListener*)listener);
	*instance = endpoint;
	return !!endpoint;
}


BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		gFactory = new IPC::FactoryImpl;
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		if (gFactory)
		{
			delete gFactory;
			gFactory = NULL;
		}
		break;
	}
	return TRUE;
}
