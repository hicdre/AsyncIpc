#include "ipc_endpoint.h"
#include <cassert>

namespace IPC
{

	Endpoint::Endpoint(const std::string& name, Listener* listener, bool start_now)
		: name_(name)
		, listener_(listener)
	{
		thread_.Start();
		if (start_now)
			Start();
	}

	Endpoint::~Endpoint()
	{
		if (channel_) 
		{
			HANDLE wait_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
			thread_.PostTask(std::bind(&Endpoint::CloseChannel, this, wait_event));
			DWORD ret = ::WaitForSingleObject(wait_event, 2000);
			assert(ret == WAIT_OBJECT_0);
			CloseHandle(wait_event);
			delete channel_;
			channel_ = NULL;
		}
		thread_.Stop();
		thread_.Wait(2000);
	}

	void Endpoint::Start()
	{
		if (channel_ == NULL)
			thread_.PostTask(std::bind(&Endpoint::CreateChannel, this));
	}

	void Endpoint::CreateChannel()
	{
		if (channel_)
			return;

		channel_ = new Channel(name_, this, &thread_);
		channel_->Connect();
	}

	bool Endpoint::Send(Message* message)
	{
		if (channel_ == NULL)
			return false;
		return channel_->Send(message);
	}

	bool Endpoint::OnMessageReceived(Message* message)
	{
		return listener_->OnMessageReceived(message);
	}

	void Endpoint::OnChannelConnected(int32 peer_pid)
	{
		listener_->OnChannelConnected(peer_pid);
	}

	void Endpoint::OnChannelError()
	{
		listener_->OnChannelError();
		Channel* ch = channel_;
		channel_ = NULL;
		delete ch;
		Start();
	}

	void Endpoint::CloseChannel(HANDLE wait_event)
	{
		Channel* ch = channel_;
		channel_ = NULL;
		delete ch;
		SetEvent(wait_event);
	}


}