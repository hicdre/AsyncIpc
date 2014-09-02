#pragma once

#include "ipc/ipc_channel.h"
#include "ipc/ipc_thread.h"
#include "ipc/ipc_listener.h"

#include "common.h"

#include <iostream>

class SampleServer : public IPC::Listener
{
public:
	virtual bool OnMessageReceived(const IPC::Message& msg)
	{
		std::string s;
		IPC::MessageReader reader(msg);
		reader.ReadString(&s);
		std::cout << "Listener::OnChannelConnected(): Message Received: " << s << std::endl;
		return true;
	}

	virtual void OnChannelConnected(int32 peer_pid)
	{
		std::cout << "Listener::OnChannelConnected(): Channel Connected" << std::endl;
	}

	virtual void OnChannelError()
	{
		std::cout << "Listener::OnChannelConnected(): Channel Error" << std::endl;
		exit(0);
	}
};

int ServerMain()
{
	IPC::Thread ipc_thread;
	SampleServer serverListener;

	std::cout << "Server: Creating IPC channel " << kChannelName << std::endl;
	IPC::Channel serverChannel(kChannelName, IPC::Channel::MODE_SERVER, &serverListener, &ipc_thread);

	if (!serverChannel.Connect()){
		std::cout << "Server: Error in connecting to the channel " << kChannelName << std::endl;
		return 1;
	}
	std::cout << "Server: Connected to IPC channel " << kChannelName << std::endl;

	std::cout << "Initializing listener" << std::endl;

	ipc_thread.Start();
	ipc_thread.Wait(INFINITE);

	return 0;
}