// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ipc/ipc_message.h"

#include <cassert>
#include <algorithm>

namespace {

int32 g_ref_num = 0;

const int kPayloadUnit = 32;

const size_t kCapacityReadOnly = static_cast<size_t>(-1);



// Create a reference number for identifying IPC messages in traces. The return
// values has the reference number stored in the upper 24 bits, leaving the low
// 8 bits set to 0 for use as flags.
inline uint32 GetRefNumUpper24() {
  int32 pid = 0;
  int32 count = InterlockedExchangeAdd(reinterpret_cast<volatile LONG*>(&g_ref_num), 1);
  // The 24 bit hash is composed of 14 bits of the count and 10 bits of the
  // Process ID. With the current trace event buffer cap, the 14-bit count did
  // not appear to wrap during a trace. Note that it is not a big deal if
  // collisions occur, as this is only used for debugging and trace analysis.
  return ((pid << 14) | (count & 0x3fff)) << 8;
}

}  // namespace

namespace IPC {

	const uint32 Message::kHeaderSize = sizeof(Message::Header);

//------------------------------------------------------------------------------

Message::~Message() {
	if (capacity_ != kCapacityReadOnly)
		free(header_);
}

Message::Message()
	: header_(NULL)
	, capacity_(0)
	, ref_count_(0)
	, variable_buffer_offset_(0) {
	Resize(kPayloadUnit);
	
  header()->payload_size = 0;
  header()->routing = header()->type = 0;
  header()->flags = GetRefNumUpper24();

  

  InitLoggingVariables();
}

Message::Message(int32 routing_id, uint32 type, PriorityValue priority)
	: header_(NULL)
	, capacity_(0)
	, ref_count_(0)
	, variable_buffer_offset_(0) {
	Resize(kPayloadUnit);

  header()->payload_size = 0;
  header()->routing = routing_id;
  header()->type = type;
  assert((priority & 0xffffff00) == 0);
  header()->flags = priority | GetRefNumUpper24();

  
  InitLoggingVariables();
}

Message::Message(const char* data, int data_len) 
	: header_(reinterpret_cast<Header*>(const_cast<char*>(data)))
	, capacity_(kCapacityReadOnly)
	, ref_count_(0)
	, variable_buffer_offset_(0) {

	if (kHeaderSize > static_cast<unsigned int>(data_len))
		header_ = NULL;

	if (header_ && header_->payload_size + kHeaderSize > static_cast<unsigned int>(data_len))
		header_ = NULL;

   InitLoggingVariables();
 }

void Message::InitLoggingVariables() {
#ifdef IPC_MESSAGE_LOG_ENABLED
  received_time_ = 0;
  dont_log_ = false;
  log_data_ = NULL;
#endif
}

void Message::SetHeaderValues(int32 routing, uint32 type, uint32 flags) {
  // This should only be called when the message is already empty.
  assert(payload_size() == 0);

  header()->routing = routing;
  header()->type = type;
  header()->flags = flags;
}

#ifdef IPC_MESSAGE_LOG_ENABLED
void Message::set_sent_time(int64 time) {
  DCHECK((header()->flags & HAS_SENT_TIME_BIT) == 0);
  header()->flags |= HAS_SENT_TIME_BIT;
  WriteInt64(time);
}

int64 Message::sent_time() const {
  if ((header()->flags & HAS_SENT_TIME_BIT) == 0)
    return 0;

  const char* data = end_of_payload();
  data -= sizeof(int64);
  return *(reinterpret_cast<const int64*>(data));
}

void Message::set_received_time(int64 time) const {
  received_time_ = time;
}
#endif

Message::Header* Message::header()
{
	return static_cast<Header*>(header_);
}

const Message::Header* Message::header() const
{
	return static_cast<const Header*>(header_);
}

bool Message::Resize(size_t new_capacity)
{
	new_capacity = AlignInt(new_capacity, kPayloadUnit);

	assert(capacity_ != kCapacityReadOnly);
	void* p = realloc(header_, new_capacity);
	if (!p)
		return false;

	header_ = reinterpret_cast<Header*>(p);
	capacity_ = new_capacity;
	return true;
}

const char* Message::FindNext(const char* range_start, const char* range_end)
{
	if (static_cast<size_t>(range_end - range_start) < sizeof(Header))
		return NULL;

	const Header* hdr = reinterpret_cast<const Header*>(range_start);
	const char* payload_base = range_start + sizeof(Header);
	const char* payload_end = payload_base + hdr->payload_size;
	if (payload_end < payload_base)
		return NULL;

	return (payload_end > range_end) ? NULL : payload_end;
}

bool Message::WriteString(const std::string& value)
{
	if(!WriteInt(static_cast<int>(value.size())))
		return false;

	return WriteBytes(value.data(), static_cast<int>(value.size()));
}

bool Message::WriteString(const std::wstring& value)
{
	if (!WriteInt(static_cast<int>(value.size())))
		return false;

	return WriteBytes(value.data(),
		static_cast<int>(value.size() * sizeof(wchar_t)));
}

bool Message::WriteData(const char* data, int length)
{
	return length >= 0 && WriteInt(length) && WriteBytes(data, length);
}

bool Message::WriteBytes(const void* data, int data_len)
{
	assert(kCapacityReadOnly != capacity_);

	size_t offset = header_->payload_size;

	size_t new_size = offset + data_len;
	size_t needed_size = sizeof(Header) + new_size;
	if (needed_size > capacity_ && !Resize((std::max)(capacity_ * 2, needed_size)))
		return false;

	header_->payload_size = static_cast<uint32>(new_size);
	char* dest = const_cast<char*>(payload()) + offset;
	memcpy(dest, data, data_len);
	return true;
}

void Message::AddRef() const
{
	InterlockedIncrement(&ref_count_);
}

void Message::Release() const
{
	if (InterlockedDecrement(&ref_count_) == 0)
	{
		delete this;
	}
}


MessageReader::MessageReader(Message* m)
	: read_ptr_(m->payload())
	, read_end_ptr_(m->end_of_payload())
{

}

template <typename Type>
inline bool MessageReader::ReadBuiltinType(Type* result) {
	const char* read_from = GetReadPointerAndAdvance(sizeof(Type));
	if (!read_from)
		return false;
	memcpy(result, read_from, sizeof(*result));
	return true;
}

const char* MessageReader::GetReadPointerAndAdvance(int num_bytes) {
	if (num_bytes < 0 || read_end_ptr_ - read_ptr_ < num_bytes)
		return NULL;
	const char* current_read_ptr = read_ptr_;
	read_ptr_ += num_bytes;
	return current_read_ptr;
}

const char* MessageReader::GetReadPointerAndAdvance(int num_elements, size_t size_element)
{
	// Check for int32 overflow.
	int64 num_bytes = static_cast<int64>(num_elements)* size_element;
	int num_bytes32 = static_cast<int>(num_bytes);
	if (num_bytes != static_cast<int64>(num_bytes32))
		return NULL;
	return GetReadPointerAndAdvance(num_bytes32);
}

bool MessageReader::ReadBool(bool* result)
{
	return ReadBuiltinType(result);
}

bool MessageReader::ReadInt(int* result)
{
	return ReadBuiltinType(result);
}

bool MessageReader::ReadUInt16(uint16* result)
{
	return ReadBuiltinType(result);
}

bool MessageReader::ReadUInt32(uint32* result)
{
	return ReadBuiltinType(result);
}

bool MessageReader::ReadInt64(int64* result)
{
	return ReadBuiltinType(result);
}

bool MessageReader::ReadUInt64(uint64* result)
{
	return ReadBuiltinType(result);
}

bool MessageReader::ReadFloat(float* result)
{
	return ReadBuiltinType(result);
}

bool MessageReader::ReadString(std::string* result)
{
	int len;
	if (!ReadInt(&len))
		return false;
	const char* read_from = GetReadPointerAndAdvance(len);
	if (!read_from)
		return false;

	result->assign(read_from, len);
	return true;
}

bool MessageReader::ReadWString(std::wstring* result)
{
	int len;
	if (!ReadInt(&len))
		return false;
	const char* read_from = GetReadPointerAndAdvance(len, sizeof(wchar_t));
	if (!read_from)
		return false;

	result->assign(reinterpret_cast<const wchar_t*>(read_from), len);
	return true;
}

bool MessageReader::ReadData(const char** data, int* length)
{
	*length = 0;
	*data = 0;

	if (!ReadInt(length))
		return false;

	return ReadBytes(data, *length);
}

bool MessageReader::ReadBytes(const char** data, int length)
{
	const char* read_from = GetReadPointerAndAdvance(length);
	if (!read_from)
		return false;
	*data = read_from;
	return true;
}

}  // namespace IPC
