#pragma once

#include "ipc/ipc_endpoint.h"

#include "common.h"

#include <iostream>
#include <thread>

class SampleClient : public IPC::Listener
{
public:
	void Init(IPC::Sender* sender)
	{
		
		sender_ = sender;
	}
	virtual bool OnMessageReceived(IPC::Message* msg)
	{
		std::string s;
		msg->routing_id();
		IPC::MessageReader reader(msg);
		reader.ReadString(&s);
		std::cout << "Process [" << msg->routing_id() <<"]: "<< s << std::endl;
		return true;
	}

	virtual void OnChannelConnected(int32 peer_pid)
	{
		id_ = peer_pid;
		std::cout << "Process [" << peer_pid << "] Connected" << std::endl;
	}

	virtual void OnChannelError()
	{
		std::cout << "Process [" << id_ << "] Disconnected" << std::endl;
	}
protected:
	int32 id_;
	IPC::Sender* sender_{ NULL };
};

int ClientMain()
{
	SampleClient listener;
	IPC::Endpoint* endpoint = NULL;
	std::string cmd;
	while (true)
	{
		std::cout << ">>";
		std::cin >> cmd;
		if (cmd == "exit")
		{
			break;
		}
		else if (cmd == "connect")
		{
			endpoint = new IPC::Endpoint(kChannelName, &listener);
		}
		else if (cmd == "close")
		{
			if (endpoint)
			{
				delete endpoint;
				endpoint = NULL;
			}
		}
		else
		{
			if (endpoint == NULL)
			{
				std::cout << "Connect First!" << std::endl;
			}
			else
			{
				IPC::Message* m = new IPC::Message(GetCurrentProcessId(), 0, (IPC::Message::PriorityValue)0);
				m->WriteString(cmd);
				std::cout << "Process [" << GetCurrentProcessId() << "]: " << cmd << std::endl;
				endpoint->Send(m);
			}
		}
	}

	if (endpoint)
	{
		delete endpoint;
		endpoint = NULL;
	}

	return 0;
}