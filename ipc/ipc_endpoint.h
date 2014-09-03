#pragma once
#include "ipc/ipc_thread.h"
#include "ipc/ipc_channel.h"
#include "ipc/ipc_listener.h"

namespace IPC
{
	class Endpoint : public Sender, public Listener
	{
	public:
		Endpoint(const std::string& name, Listener* listener, bool start_now = true);
		~Endpoint();

		void Start();

		virtual bool Send(Message* message) override;

		virtual bool OnMessageReceived(Message* message) override;

		virtual void OnChannelConnected(int32 peer_pid) override;

		virtual void OnChannelError() override;

	private:
		void CreateChannel();
		void CloseChannel(HANDLE wait_event);
		std::string name_;
		Thread thread_;

		Channel* channel_;
		Listener* listener_;
	};
}