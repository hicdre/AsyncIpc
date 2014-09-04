#include "ipc_endpoint.h"
#include "ipc/ipc_message.h"
#include <cassert>

namespace IPC
{

	Endpoint::Endpoint(const std::string& name, Listener* listener, bool start_now)
		: name_(name)
		, listener_(listener)
		, is_connected_(false)
	{
		thread_.Start();
		if (start_now)
			Start();
	}

	Endpoint::~Endpoint()
	{
		SetConnected(false);
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


	bool Endpoint::IsConnected() const
	{
		AutoLock lock(lock_);
		return is_connected_;
	}


	void Endpoint::SetConnected(bool c)
	{
		AutoLock lock(lock_);
		is_connected_ = c;
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
		scoped_refptr<Message> m(message);
		if (channel_ == NULL || !IsConnected()) {
			return false;
		}
		thread_.PostTask(std::bind(&Endpoint::OnSendMessage, this, m));
		return true;
	}


	void Endpoint::OnSendMessage(scoped_refptr<Message> message)
	{
		if (channel_ == NULL)
			return;

		channel_->Send(message.get());
	}


	bool Endpoint::OnMessageReceived(Message* message)
	{
		return listener_->OnMessageReceived(message);
	}

	void Endpoint::OnChannelConnected(int32 peer_pid)
	{
		SetConnected(true);
		listener_->OnChannelConnected(peer_pid);
	}

	void Endpoint::OnChannelError()
	{
		Channel* ch = channel_;
		channel_ = NULL;
		delete ch;
		SetConnected(false);
		listener_->OnChannelError();
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