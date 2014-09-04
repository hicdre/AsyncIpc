#pragma once
#include "ipc/ipc_interface.h"
#include "ipc/ipc_endpoint.h"
#include <string>

namespace IPC
{
	class EndpointImpl : public IEndpoint, public Listener
	{
	public:
		EndpointImpl(const std::string& name, IListener* listener);
		~EndpointImpl();
		virtual void      AddRef() const override;
		virtual void      Release() const override;
		virtual ErrorCode Send(const char* message, size_t len) override;
		virtual void      SetListener(IListener* listener) override;
		virtual bool      HasListener() const override;
	private:
		virtual bool OnMessageReceived(Message* message) override;
		virtual void OnChannelConnected(int32 peer_pid) override;
		virtual void OnChannelError() override;
	private:
		Endpoint* endpoint_;
		IListener* listener_;

		mutable Lock lock_;
		mutable LONG ref_count_;
	};
}