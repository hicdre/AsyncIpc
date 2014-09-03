#pragma once

#include "ipc/ipc_channel.h"
#include "ipc/ipc_thread.h"
#include "ipc/ipc_listener.h"

#include "common.h"

#include <iostream>

class SampleClient : public IPC::Listener
{
public:
	void Init(IPC::Sender* sender)
	{
		sender_ = sender;
	}
	virtual bool OnMessageReceived(IPC::Message* msg)
	{
		std::cout << "Listener::OnChannelConnected(): Message Received" << std::endl;
		return true;
	}

	virtual void OnChannelConnected(int32 peer_pid)
	{
		std::cout << "Listener::OnChannelConnected(): Channel Connected" << std::endl;

		IPC::Message* m = new IPC::Message;
		m->WriteString("hello, this is client");
		sender_->Send(m);
	}

	virtual void OnChannelError()
	{
		std::cout << "Listener::OnChannelConnected(): Channel Error" << std::endl;
		exit(0);
	}
protected:
	IPC::Sender* sender_{ NULL };
};

int ClientMain()
{
	IPC::Thread ipc_thread;
	SampleClient clientListener;

	std::cout << "Client: Creating IPC channel " << kChannelName << std::endl;
	IPC::Channel clientChannel(kChannelName, IPC::Channel::MODE_CLIENT, &clientListener, &ipc_thread);
	clientListener.Init(&clientChannel);

	if (!clientChannel.Connect()){
		std::cout << "Client: Error in connecting to the channel " << kChannelName << std::endl;
		return 1;
	}
	std::cout << "Client: Connected to IPC channel " << kChannelName << std::endl;

	std::cout << "Initializing listener" << std::endl;

	ipc_thread.Start();
	ipc_thread.Wait(INFINITE);

	return 0;
}