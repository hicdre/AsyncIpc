#include "ipc_thread.h"
#include <cassert>

namespace IPC
{
	Thread::Thread()
		: thread_(NULL)
		, should_quit_(false)
	{
		io_port_ = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
	}

	Thread::~Thread()
	{
		::CloseHandle(io_port_);
	}

	void Thread::RegisterIOHandler(HANDLE file, IOHandler* handler)
	{
		ULONG_PTR key = HandlerToKey(handler, true);
		HANDLE port = CreateIoCompletionPort(file, io_port_, key, 1);
		assert(port);
	}

	ULONG_PTR Thread::HandlerToKey(IOHandler* handler, bool has_valid_io_context)
	{
		ULONG_PTR key = reinterpret_cast<ULONG_PTR>(handler);

		// |IOHandler| is at least pointer-size aligned, so the lowest two bits are
		// always cleared. We use the lowest bit to distinguish completion keys with
		// and without the associated |IOContext|.
		assert((key & 1) == 0);

		// Mark the completion key as context-less.
		if (!has_valid_io_context)
			key = key | 1;
		return key;
	}

	Thread::IOHandler* Thread::KeyToHandler(ULONG_PTR key, bool* has_valid_io_context)
	{
		*has_valid_io_context = ((key & 1) == 0);
		return reinterpret_cast<IOHandler*>(key & ~static_cast<ULONG_PTR>(1));
	}

	void Thread::PostTask(const Task& task)
	{
		{
			AutoLock lock(task_mutex_);
			task_queue_.push_back(task);
		}
		ScheduleWork();
	}

	void Thread::Start()
	{
		if (thread_ == NULL)
		{
			should_quit_ = false;
			thread_ = ::CreateThread(0, 0, IOThreadMain, this, 0, 0);
		}
	}


	void Thread::Stop()
	{
		should_quit_ = true;

		if (thread_)
		{
			::WaitForSingleObject(thread_, 1000);
			CloseHandle(thread_);
			thread_ = NULL;
		}
	}


	void Thread::Wait(DWORD timeout)
	{
		::WaitForSingleObject(thread_, timeout);
	}



	DWORD WINAPI Thread::IOThreadMain(LPVOID params)
	{
		reinterpret_cast<Thread*>(params)->Run();
		return 0;
	}

	void Thread::Run()
	{
		for (;;) {

			bool more_work_is_plausible = DoScheduledWork();

			if (should_quit_)
				break;

			more_work_is_plausible |= WaitForIOCompletion(0, NULL);

			if (should_quit_)
				break;

			if (more_work_is_plausible)
				continue;

			if (should_quit_)
				break;

			WaitForWork();  // Wait (sleep) until we have work to do again.
		}
	}

	bool Thread::DoScheduledWork()
	{
		std::deque<Task> work_queue;
		{
			AutoLock lock(task_mutex_);
			if (task_queue_.empty())
				return false;
			work_queue.swap(task_queue_);
		}

		do
		{
			Task t = work_queue.front();
			work_queue.pop_front();

			t();
		} while (!work_queue.empty());

		return false;
	}

	bool Thread::WaitForIOCompletion(DWORD timeout, IOHandler* filter)
	{
		IOItem item;
		if (completed_io_.empty() || !MatchCompletedIOItem(filter, &item)) {
			// We have to ask the system for another IO completion.
			if (!GetIOItem(timeout, &item))
				return false;

			if (ProcessInternalIOItem(item))
				return true;
		}

		// If |item.has_valid_io_context| is false then |item.context| does not point
		// to a context structure, and so should not be dereferenced, although it may
		// still hold valid non-pointer data.
		if (!item.has_valid_io_context || item.context->handler) {
			if (filter && item.handler != filter) {
				// Save this item for later
				completed_io_.push_back(item);
			}
			else {
				assert(!item.has_valid_io_context ||
					(item.context->handler == item.handler));
				item.handler->OnIOCompleted(item.context, item.bytes_transfered,
					item.error);
			}
		}
		else {
			// The handler must be gone by now, just cleanup the mess.
			delete item.context;
		}
		return true;
	}

	void Thread::WaitForWork()
	{
		int timeout;
		timeout = INFINITE;
		WaitForIOCompletion(timeout, NULL);
	}

	bool Thread::MatchCompletedIOItem(IOHandler* filter, IOItem* item)
	{
		for (std::list<IOItem>::iterator it = completed_io_.begin();
			it != completed_io_.end(); ++it) {
			if (!filter || it->handler == filter) {
				*item = *it;
				completed_io_.erase(it);
				return true;
			}
		}
		return false;
	}

	bool Thread::GetIOItem(DWORD timeout, IOItem* item)
	{
		memset(item, 0, sizeof(*item));
		ULONG_PTR key = NULL;
		OVERLAPPED* overlapped = NULL;
		if (!GetQueuedCompletionStatus(io_port_, &item->bytes_transfered, &key,
			&overlapped, timeout)) {
			if (!overlapped)
				return false;  // Nothing in the queue.
			item->error = GetLastError();
			item->bytes_transfered = 0;
		}

		item->handler = KeyToHandler(key, &item->has_valid_io_context);
		item->context = reinterpret_cast<IOContext*>(overlapped);
		return true;
	}

	bool Thread::ProcessInternalIOItem(const IOItem& item)
	{
		if (this == reinterpret_cast<Thread*>(item.context) &&
			this == reinterpret_cast<Thread*>(item.handler)) {
			// This is our internal completion.
			assert(!item.bytes_transfered);
			return true;
		}
		return false;
	}

	void Thread::ScheduleWork()
	{
		PostQueuedCompletionStatus(io_port_, 0,
			reinterpret_cast<ULONG_PTR>(this),
			reinterpret_cast<OVERLAPPED*>(this));
	}

}
