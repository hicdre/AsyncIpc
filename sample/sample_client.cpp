// sample.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "sample_client.h"
#include "ipc/ipc_message.h"


int _tmain(int argc, _TCHAR* argv[])
{
	SampleClient listener;
	IPC::Endpoint endpoint(kChannelName, &listener);
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
			scoped_refptr<IPC::Message> m(new IPC::Message(GetCurrentProcessId(), 0, (IPC::Message::PriorityValue)0));
			m->WriteString(cmd);
			//std::cout << "Process [" << GetCurrentProcessId() << "]: " << cmd << std::endl;
			endpoint.Send(m.get());
		}
	}

	return 0;
}


void SampleClient::OnChannelError()
{
	std::cout << "Process [" << id_ << "] Disconnected" << std::endl;
}

void SampleClient::OnChannelConnected(int32 peer_pid)
{
	id_ = peer_pid;
	std::cout << "Process [" << peer_pid << "] Connected" << std::endl;
}

bool SampleClient::OnMessageReceived(IPC::Message* msg)
{
	std::string s;
	msg->routing_id();
	IPC::MessageReader reader(msg);
	reader.ReadString(&s);
	std::cout << "Process [" << id_ << "]: " << s << std::endl;
	return true;
}

