// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IPC_IPC_MESSAGE_H_
#define IPC_IPC_MESSAGE_H_

#include <string>

#include "ipc/ipc_common.h"

#if !defined(NDEBUG)
//#define IPC_MESSAGE_LOG_ENABLED
#endif

namespace IPC {

//------------------------------------------------------------------------------

class Message;
class MessageReader
{
public:
	MessageReader() : read_ptr_(NULL), read_end_ptr_(NULL) {}
	explicit MessageReader(Message* m);

	// Methods for reading the payload of the Pickle. To read from the start of
	// the Pickle, create a PickleIterator from a Pickle. If successful, these
	// methods return true. Otherwise, false is returned to indicate that the
	// result could not be extracted.
	bool ReadBool(bool* result);
	bool ReadInt(int* result);
	bool ReadUInt16(uint16* result);
	bool ReadUInt32(uint32* result);
	bool ReadInt64(int64* result);
	bool ReadUInt64(uint64* result);
	bool ReadFloat(float* result);
	bool ReadString(std::string* result);
	bool ReadWString(std::wstring* result);
	bool ReadData(const char** data, int* length);
	bool ReadBytes(const char** data, int length);


private:
	template <typename Type>
	inline bool ReadBuiltinType(Type* result);

	const char* GetReadPointerAndAdvance(int num_bytes);

	inline const char* GetReadPointerAndAdvance(int num_elements,
		size_t size_element);

	// Pointers to the Pickle data.
	const char* read_ptr_;
	const char* read_end_ptr_;
};

class Message {
 public:
  enum PriorityValue {
    PRIORITY_LOW = 1,
    PRIORITY_NORMAL,
    PRIORITY_HIGH
  };

  // Bit values used in the flags field.
  // Upper 24 bits of flags store a reference number, so this enum is limited to
  // 8 bits.
  enum {
    PRIORITY_MASK     = 0x03,  // Low 2 bits of store the priority value.
    SYNC_BIT          = 0x04,
    REPLY_BIT         = 0x08,
    REPLY_ERROR_BIT   = 0x10,
    UNBLOCK_BIT       = 0x20,
    PUMPING_MSGS_BIT  = 0x40,
    HAS_SENT_TIME_BIT = 0x80,
  };

  Message();

  // Initialize a message with a user-defined type, priority value, and
  // destination WebView ID.
  Message(int32 routing_id, uint32 type, PriorityValue priority);

  // Initializes a message from a const block of data.  The data is not copied;
  // instead the data is merely referenced by this message.  Only const methods
  // should be used on the message when initialized this way.
  Message(const char* data, int data_len);

  void AddRef() const;
  void Release() const;

  // Returns the size of the Pickle's data.
  size_t size() const { return kHeaderSize + header_->payload_size; }

  // Returns the data for this Pickle.
  const void* data() const { return header_; }

  // The payload is the pickle data immediately following the header.
  size_t payload_size() const { return header_->payload_size; }

  const char* payload() const {
	  return reinterpret_cast<const char*>(header_)+kHeaderSize;
  }

  // Returns the address of the byte immediately following the currently valid
  // header + payload.
  const char* end_of_payload() const {
	  // This object may be invalid.
	  return header_ ? payload() + payload_size() : NULL;
  }

  PriorityValue priority() const {
    return static_cast<PriorityValue>(header()->flags & PRIORITY_MASK);
  }

  // True if this is a synchronous message.
  void set_sync() {
    header()->flags |= SYNC_BIT;
  }
  bool is_sync() const {
    return (header()->flags & SYNC_BIT) != 0;
  }

  // Set this on a reply to a synchronous message.
  void set_reply() {
    header()->flags |= REPLY_BIT;
  }

  bool is_reply() const {
    return (header()->flags & REPLY_BIT) != 0;
  }

  // Set this on a reply to a synchronous message to indicate that no receiver
  // was found.
  void set_reply_error() {
    header()->flags |= REPLY_ERROR_BIT;
  }

  bool is_reply_error() const {
    return (header()->flags & REPLY_ERROR_BIT) != 0;
  }

  // Normally when a receiver gets a message and they're blocked on a
  // synchronous message Send, they buffer a message.  Setting this flag causes
  // the receiver to be unblocked and the message to be dispatched immediately.
  void set_unblock(bool unblock) {
    if (unblock) {
      header()->flags |= UNBLOCK_BIT;
    } else {
      header()->flags &= ~UNBLOCK_BIT;
    }
  }

  bool should_unblock() const {
    return (header()->flags & UNBLOCK_BIT) != 0;
  }

  // Tells the receiver that the caller is pumping messages while waiting
  // for the result.
  bool is_caller_pumping_messages() const {
    return (header()->flags & PUMPING_MSGS_BIT) != 0;
  }

  uint32 type() const {
    return header()->type;
  }

  int32 routing_id() const {
    return header()->routing;
  }

  void set_routing_id(int32 new_id) {
    header()->routing = new_id;
  }

  uint32 flags() const {
    return header()->flags;
  }

  // Sets all the given header values. The message should be empty at this
  // call.
  void SetHeaderValues(int32 routing, uint32 type, uint32 flags);

  bool WriteBool(bool value) {
	  return WriteInt(value ? 1 : 0);
  }
  bool WriteInt(int value) {
	  return WriteBytes(&value, sizeof(value));
  }
  bool WriteUInt16(uint16 value) {
	  return WriteBytes(&value, sizeof(value));
  }
  bool WriteUInt32(uint32 value) {
	  return WriteBytes(&value, sizeof(value));
  }
  bool WriteInt64(int64 value) {
	  return WriteBytes(&value, sizeof(value));
  }
  bool WriteUInt64(uint64 value) {
	  return WriteBytes(&value, sizeof(value));
  }
  bool WriteFloat(float value) {
	  return WriteBytes(&value, sizeof(value));
  }
  bool WriteString(const std::string& value);
  bool WriteString(const std::wstring& value);
  // "Data" is a blob with a length. When you read it out you will be given the
  // length. See also WriteBytes.
  bool WriteData(const char* data, int length);
  // "Bytes" is a blob with no length. The caller must specify the lenght both
  // when reading and writing. It is normally used to serialize PoD types of a
  // known size. See also WriteData.
  bool WriteBytes(const void* data, int data_len);

  // Used for async messages with no parameters.
  static void Log(std::string* name, const Message* msg, std::string* l) {
  }

  // 从buffer中寻找下一条消息的起始地址
  // Find the end of the message data that starts at range_start.  Returns NULL
  // if the entire message is not found in the given data range.
  static const char* FindNext(const char* range_start, const char* range_end);

#ifdef IPC_MESSAGE_LOG_ENABLED
  // Adds the outgoing time from Time::Now() at the end of the message and sets
  // a bit to indicate that it's been added.
  void set_sent_time(int64 time);
  int64 sent_time() const;

  void set_received_time(int64 time) const;
  int64 received_time() const { return received_time_; }
  void set_output_params(const std::string& op) const { output_params_ = op; }
  const std::string& output_params() const { return output_params_; }
  // The following four functions are needed so we can log sync messages with
  // delayed replies.  We stick the log data from the sent message into the
  // reply message, so that when it's sent and we have the output parameters
  // we can log it.  As such, we set a flag on the sent message to not log it.
  void set_sync_log_data(LogData* data) const { log_data_ = data; }
  LogData* sync_log_data() const { return log_data_; }
  void set_dont_log() const { dont_log_ = true; }
  bool dont_log() const { return dont_log_; }
#endif

  // Called to trace when message is sent.
  //void TraceMessageBegin() {
  //  TRACE_EVENT_FLOW_BEGIN0("ipc", "IPC", header()->flags);
  //}
  // Called to trace when message is received.
  //void TraceMessageEnd() {
  //  TRACE_EVENT_FLOW_END0("ipc", "IPC", header()->flags);
  //}

 protected:
  friend class Channel;
  virtual ~Message();

#pragma pack(push, 4)
  struct Header {
    int32 routing;  // ID of the view that this message is destined for
    uint32 type;    // specifies the user-defined message type
    uint32 flags;   // specifies control flags for the message
	uint32 payload_size;
  };//16 BYTES
#pragma pack(pop)

  Header* header();
  const Header* header() const;

  // Resize the capacity, note that the input value should include the size of
  // the header: new_capacity = sizeof(Header) + desired_payload_capacity.
  // A realloc() failure will cause a Resize failure... and caller should check
  // the return result for true (i.e., successful resizing).
  bool Resize(size_t new_capacity);

  // Aligns 'i' by rounding it up to the next multiple of 'alignment'
  static size_t AlignInt(size_t i, int alignment) {
	  return i + (alignment - (i % alignment)) % alignment;
  }

  void InitLoggingVariables();

#ifdef IPC_MESSAGE_LOG_ENABLED
  // Used for logging.
  mutable int64 received_time_;
  mutable std::string output_params_;
  mutable LogData* log_data_;
  mutable bool dont_log_;
#endif
  static const uint32 kHeaderSize;

  Header* header_;
  // Allocation size of payload (or -1 if allocation is const).
  size_t capacity_;
  size_t variable_buffer_offset_;  // IF non-zero, then offset to a buffer.

  mutable LONG ref_count_;
};

//------------------------------------------------------------------------------

}  // namespace IPC

enum SpecialRoutingIDs {
  // indicates that we don't have a routing ID yet.
  MSG_ROUTING_NONE = -2,

  // indicates a general message not sent to a particular tab.
  MSG_ROUTING_CONTROL = kint32max,
};

#define IPC_REPLY_ID 0xFFFFFFF0  // Special message id for replies
#define IPC_LOGGING_ID 0xFFFFFFF1  // Special message id for logging

#endif  // IPC_IPC_MESSAGE_H_
