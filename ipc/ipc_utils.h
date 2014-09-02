#pragma once
#include "ipc/ipc_common.h"

class Mutex
{
public:
	Mutex();
	~Mutex();

	bool Try();

	// Take the lock, blocking until it is available if necessary.
	void Lock();

	// Release the lock.  This must only be called by the lock's holder: after
	// a successful call to Try, or a call to Lock.
	void Unlock();

private:
	CRITICAL_SECTION cs;
	DISALLOW_COPY_AND_ASSIGN(Mutex);
};

class Lock
{
public:
	explicit Lock(Mutex& m);
	~Lock();

private:
	Mutex& m_;
	DISALLOW_COPY_AND_ASSIGN(Lock);
};

class StaticAtomicSequenceNumber {
public:
	inline int GetNext() {
		return static_cast<int>(
			InterlockedExchangeAdd(reinterpret_cast<volatile LONG*>(&seq_),	1));
	}

private:
	friend class AtomicSequenceNumber;

	inline void Reset() {
		seq_ = 0;
	}

	int32 seq_;
};

int RandInt(int min, int max);

uint64 RandGenerator(uint64 range);

std::wstring ASCIIToWide(const std::string& str);