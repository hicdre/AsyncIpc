#pragma once

#include "ipc/ipc_channel.h"
#include "ipc/ipc_thread.h"
#include "ipc/ipc_listener.h"

#include "common.h"

#include <iostream>

class SampleServer : public IPC::Listener
{
public:
	virtual bool OnMessageReceived(IPC::Message* msg)
	{
		std::string s;
		msg->routing_id();
		IPC::MessageReader reader(msg);
		reader.ReadString(&s);
		std::cout << "Server: Message Received: " << s << std::endl;
		return true;
	}

	virtual void OnChannelConnected(int32 peer_pid)
	{
		std::cout << "====Process ["<<peer_pid<<"] Connected" << std::endl;
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

	std::cout << "====Creating Server : " << kChannelName << std::endl;
	IPC::Channel serverChannel(kChannelName, IPC::Channel::MODE_SERVER, &serverListener, &ipc_thread);

	if (!serverChannel.Connect()){
		std::cout << "====Creating Server Error!"<< std::endl;
		return 1;
	}
	std::cout << "====Creating Server Success!"<<std::endl;

	ipc_thread.Start();
	ipc_thread.Wait(INFINITE);

	return 0;
}