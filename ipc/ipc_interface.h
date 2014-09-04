#pragma once

namespace IPC
{
	enum ErrorCode
	{
		ERROR_OK = 0,

		ERROR_MESSAGE_READONLY = 1,

		ERROR_DEST_DISCONNECTED = 2,
	};

	class IListener
	{
	public:
		virtual void AddRef() const = 0;
		virtual void Release() const = 0;
		virtual void OnMessageReceived(const char* message, size_t len) {};
		virtual void OnDestConnected(int process_id) {}
		virtual void OnDestDisconnected() {}
	};

	class IEndpoint
	{
	public:
		virtual void      AddRef() const = 0;
		virtual void      Release() const = 0;
		virtual ErrorCode Send(const char* message, size_t len) = 0;
		virtual bool      HasListener() const = 0;
		virtual void	  SetListener(IListener* listener) = 0;
	};

}