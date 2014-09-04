// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the IPC_DLL_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// IPC_DLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

__declspec(dllexport) bool GetIPCEndPoint(void** instance, const char* name, void* listener = 0);
