#pragma once
#include "ipc/ipc_common.h"
#include "ipc/ipc_utils.h"

#include <functional>
#include <queue>
#include <list>

namespace IPC
{
	class Thread
	{
	public:
		typedef std::function<void(void)> Task;

		struct IOContext;

		class IOHandler {
		public:
			virtual ~IOHandler() {}
			// This will be called once the pending IO operation associated with
			// |context| completes. |error| is the Win32 error code of the IO operation
			// (ERROR_SUCCESS if there was no error). |bytes_transfered| will be zero
			// on error.
			virtual void OnIOCompleted(IOContext* context, DWORD bytes_transfered,
				DWORD error) = 0;
		};

		struct IOContext {
			OVERLAPPED overlapped;
			IOHandler* handler;
		};

		Thread();
		~Thread();

		void Start();
		void Stop();
		void Wait(DWORD timeout);

		void RegisterIOHandler(HANDLE file, IOHandler* handler);
		bool WaitForIOCompletion(DWORD timeout, IOHandler* filter);

		void PostTask(const Task& task);
	private:
		struct IOItem {
			IOHandler* handler;
			IOContext* context;
			DWORD bytes_transfered;
			DWORD error;

			// In some cases |context| can be a non-pointer value casted to a pointer.
			// |has_valid_io_context| is true if |context| is a valid IOContext
			// pointer, and false otherwise.
			bool has_valid_io_context;
		};

		static DWORD WINAPI IOThreadMain(LPVOID params);
		// Converts an IOHandler pointer to a completion port key.
		// |has_valid_io_context| specifies whether completion packets posted to
		// |handler| will have valid OVERLAPPED pointers.
		static ULONG_PTR HandlerToKey(IOHandler* handler, bool has_valid_io_context);
		static IOHandler* KeyToHandler(ULONG_PTR key, bool* has_valid_io_context);

		void Run();
		bool DoScheduledWork();
		void ScheduleWork();
		void WaitForWork();

		bool MatchCompletedIOItem(IOHandler* filter, IOItem* item);
		bool GetIOItem(DWORD timeout, IOItem* item);
		bool ProcessInternalIOItem(const IOItem& item);
		void WillProcessIOEvent();
		void DidProcessIOEvent();

		HANDLE thread_;
		bool should_quit_;

		HANDLE io_port_;
		std::list<IOItem> completed_io_;

		Lock task_mutex_;
		std::deque<Task> task_queue_;
	};
}