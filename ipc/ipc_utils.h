#pragma once
#include "ipc/ipc_common.h"

class Lock
{
public:
	Lock();
	~Lock();

	bool Try();

	// Take the lock, blocking until it is available if necessary.
	void Dolock();

	// Release the lock.  This must only be called by the lock's holder: after
	// a successful call to Try, or a call to Lock.
	void Unlock();

private:
	CRITICAL_SECTION cs;
	DISALLOW_COPY_AND_ASSIGN(Lock);
};

class AutoLock
{
public:
	explicit AutoLock(Lock& m);
	~AutoLock();

private:
	Lock& m_;
	DISALLOW_COPY_AND_ASSIGN(AutoLock);
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

template <class T>
class scoped_refptr
{
public:
	scoped_refptr(T* t)
	{
		p_ = t;
		if (p_)
			p_->AddRef();
	}
	~scoped_refptr()
	{
		Clear();
	}
	scoped_refptr(const scoped_refptr<T>& r) : p_(r.p_) {
		if (p_)
			p_->AddRef();
	}
	void Clear()
	{
		if (p_)
			p_->Release();
	}
	T* operator->() const
	{
		return p_;
	}
	T* get() const
	{
		return p_;
	}


private:
	T* p_;
};

int RandInt(int min, int max);

uint64 RandGenerator(uint64 range);

std::wstring ASCIIToWide(const std::string& str);