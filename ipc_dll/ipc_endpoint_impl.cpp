#include "stdafx.h"
#include "ipc_endpoint_impl.h"
#include "ipc/ipc_message.h"
#include <cassert>

namespace IPC
{


	EndpointImpl::EndpointImpl(const std::string& name, IListener* listener)
		: endpoint_(new Endpoint(name, this))
	{
		listener_ = listener;
		if (listener_)
			listener_->AddRef();
	}

	EndpointImpl::~EndpointImpl()
	{
		assert(endpoint_);
		delete endpoint_;
		endpoint_ = NULL;

		if (listener_)
			listener_->Release();
	}

	void EndpointImpl::AddRef() const
	{
		InterlockedIncrement(&ref_count_);
	}

	void EndpointImpl::Release() const
	{
		if (InterlockedDecrement(&ref_count_) == 0)
		{
			delete this;
		}
	}

	IPC::ErrorCode EndpointImpl::Send(const char* message, size_t len)
	{
		scoped_refptr<Message> m(new Message);
		m->WriteString(std::string(message, len));
		return endpoint_->Send(m.get()) ? ERROR_OK : ERROR_DEST_DISCONNECTED;
	}

	void EndpointImpl::SetListener(IListener* listener)
	{
		AutoLock lock(lock_);
		if (listener_)
			listener_->Release();
		listener_ = listener;
		if (listener_)
			listener_->AddRef();
	}


	bool EndpointImpl::HasListener() const
	{
		AutoLock lock(lock_);
		return listener_ != NULL;
	}


	bool EndpointImpl::OnMessageReceived(Message* message)
	{
		std::string s;
		MessageReader reader(message);
		if (!reader.ReadString(&s))
			return true;
		AutoLock lock(lock_);
		if (listener_)
			listener_->OnMessageReceived(s.c_str(), s.size());
		return true;
	}

	void EndpointImpl::OnChannelConnected(int32 peer_pid)
	{
		AutoLock lock(lock_);
		if (listener_)
			listener_->OnDestConnected(peer_pid);
	}

	void EndpointImpl::OnChannelError()
	{
		AutoLock lock(lock_);
		if (listener_)
			listener_->OnDestDisconnected();
	}

}