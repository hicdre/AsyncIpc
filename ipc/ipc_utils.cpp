#include "ipc_utils.h"
#include <cassert>
#include <stdlib.h>

Lock::Lock()
{
	::InitializeCriticalSectionAndSpinCount(&cs, 2000);
}

Lock::~Lock()
{
	::DeleteCriticalSection(&cs);
}

bool Lock::Try()
{
	if (::TryEnterCriticalSection(&cs) != FALSE) {
		return true;
	}
	return false;
}

void Lock::Dolock()
{
	::EnterCriticalSection(&cs);
}

void Lock::Unlock()
{
	::LeaveCriticalSection(&cs);
}

AutoLock::AutoLock(Lock& m)
	: m_(m)
{
	m_.Dolock();
}

AutoLock::~AutoLock()
{
	m_.Unlock();
}

uint32 RandUint32() {
	uint32 number;
	rand_s(&number);
	return number;
}

uint64 RandUint64() {
	uint32 first_half = RandUint32();
	uint32 second_half = RandUint32();
	return (static_cast<uint64>(first_half) << 32) + second_half;
}

int RandInt(int min, int max)
{
	assert(min < max);

	uint64 range = static_cast<uint64>(max)-min + 1;
	int result = min + static_cast<int>(RandGenerator(range));
	return result;
}

uint64 RandGenerator(uint64 range)
{
	assert(range > 0u);
	// We must discard random results above this number, as they would
	// make the random generator non-uniform (consider e.g. if
	// MAX_UINT64 was 7 and |range| was 5, then a result of 1 would be twice
	// as likely as a result of 3 or 4).
	uint64 max_acceptable_value =
		((std::numeric_limits<uint64>::max)() / range) * range - 1;

	uint64 value;
	do {
		value = RandUint64();
	} while (value > max_acceptable_value);

	return value % range;
}

std::wstring ASCIIToWide(const std::string& mb)
{
	if (mb.empty())
		return std::wstring();

	int mb_length = static_cast<int>(mb.length());
	// Compute the length of the buffer.
	int charcount = MultiByteToWideChar(CP_UTF8, 0,
		mb.data(), mb_length, NULL, 0);
	if (charcount == 0)
		return std::wstring();

	std::wstring wide;
	wide.resize(charcount);
	MultiByteToWideChar(CP_UTF8, 0, mb.data(), mb_length, &wide[0], charcount);

	return wide;
}
