#include "stdafx.h"

#include "sample_dll_client.h"


void SampleDllClient::OnDestDisconnected()
{
	std::cout << "Process [" << id_ << "] Disconnected" << std::endl;
}

void SampleDllClient::OnDestConnected(int peer_pid)
{
	id_ = peer_pid;
	std::cout << "Process [" << peer_pid << "] Connected" << std::endl;
}

void SampleDllClient::OnMessageReceived(const char* message, size_t len)
{
	std::string s(message, len);
	std::cout << "Process [" << id_ << "]: " << s << std::endl;
}

void SampleDllClient::AddRef() const
{
	InterlockedIncrement(&ref_count_);
}

void SampleDllClient::Release() const
{
	if (InterlockedDecrement(&ref_count_) == 0)
	{
		delete this;
	}
}

SampleDllClient::SampleDllClient()
	: ref_count_(0)
	, id_(0)
	, module_(NULL)
	, endpoint_(NULL)
{
	Init();
}


SampleDllClient::~SampleDllClient()
{
	if (endpoint_)
	{
		endpoint_->Release();
		endpoint_ = NULL;
	}
	if (module_)
	{
		FreeLibrary(module_);
		module_ = NULL;
	}
}


void SampleDllClient::Send(const std::string& cmd)
{
	if (endpoint_)
		endpoint_->Send(cmd.c_str(), cmd.size());
}

void SampleDllClient::Init()
{
	module_ = LoadLibrary(L"ipc_dll.dll");
	if (module_ == NULL)
		return;

	typedef bool (*FuncGetIPCEndPoint)(void** instance, const char* name, void* listener);
	FuncGetIPCEndPoint pfn = (FuncGetIPCEndPoint)GetProcAddress(module_, "GetIPCEndPoint");
	if (!pfn)
		return;

	if (pfn((void**)&endpoint_, kChannelName, this))
	{
		endpoint_->AddRef();
	}
}


int _tmain(int argc, _TCHAR* argv[])
{
	SampleDllClient* client = new SampleDllClient;
	client->AddRef();

	std::string cmd;
	while (true)
	{
		std::cout << ">>";
		std::cin >> cmd;
		if (cmd == "exit")
		{
			break;
		}
		else
		{
			client->Send(cmd);
		}
	}

	client->Release();

	return 0;
}
