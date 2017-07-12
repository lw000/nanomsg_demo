// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: game.proto

#ifndef PROTOBUF_game_2eproto__INCLUDED
#define PROTOBUF_game_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3001000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3001000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace msg {

// Internal implementation detail -- do not call these.
void protobuf_AddDesc_game_2eproto();
void protobuf_InitDefaults_game_2eproto();
void protobuf_AssignDesc_game_2eproto();
void protobuf_ShutdownFile_game_2eproto();

class game_end_msg;
class game_start_msg;
class sit_down_msg;
class sit_up_msg;

// ===================================================================

class game_start_msg : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:msg.game_start_msg) */ {
 public:
  game_start_msg();
  virtual ~game_start_msg();

  game_start_msg(const game_start_msg& from);

  inline game_start_msg& operator=(const game_start_msg& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const game_start_msg& default_instance();

  static const game_start_msg* internal_default_instance();

  void Swap(game_start_msg* other);

  // implements Message ----------------------------------------------

  inline game_start_msg* New() const { return New(NULL); }

  game_start_msg* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const game_start_msg& from);
  void MergeFrom(const game_start_msg& from);
  void Clear();
  bool IsInitialized() const;

  size_t ByteSizeLong() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(game_start_msg* other);
  void UnsafeMergeFrom(const game_start_msg& from);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional int32 state = 1;
  void clear_state();
  static const int kStateFieldNumber = 1;
  ::google::protobuf::int32 state() const;
  void set_state(::google::protobuf::int32 value);

  // @@protoc_insertion_point(class_scope:msg.game_start_msg)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::int32 state_;
  mutable int _cached_size_;
  friend void  protobuf_InitDefaults_game_2eproto_impl();
  friend void  protobuf_AddDesc_game_2eproto_impl();
  friend void protobuf_AssignDesc_game_2eproto();
  friend void protobuf_ShutdownFile_game_2eproto();

  void InitAsDefaultInstance();
};
extern ::google::protobuf::internal::ExplicitlyConstructed<game_start_msg> game_start_msg_default_instance_;

// -------------------------------------------------------------------

class game_end_msg : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:msg.game_end_msg) */ {
 public:
  game_end_msg();
  virtual ~game_end_msg();

  game_end_msg(const game_end_msg& from);

  inline game_end_msg& operator=(const game_end_msg& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const game_end_msg& default_instance();

  static const game_end_msg* internal_default_instance();

  void Swap(game_end_msg* other);

  // implements Message ----------------------------------------------

  inline game_end_msg* New() const { return New(NULL); }

  game_end_msg* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const game_end_msg& from);
  void MergeFrom(const game_end_msg& from);
  void Clear();
  bool IsInitialized() const;

  size_t ByteSizeLong() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(game_end_msg* other);
  void UnsafeMergeFrom(const game_end_msg& from);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional int32 state = 1;
  void clear_state();
  static const int kStateFieldNumber = 1;
  ::google::protobuf::int32 state() const;
  void set_state(::google::protobuf::int32 value);

  // @@protoc_insertion_point(class_scope:msg.game_end_msg)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::int32 state_;
  mutable int _cached_size_;
  friend void  protobuf_InitDefaults_game_2eproto_impl();
  friend void  protobuf_AddDesc_game_2eproto_impl();
  friend void protobuf_AssignDesc_game_2eproto();
  friend void protobuf_ShutdownFile_game_2eproto();

  void InitAsDefaultInstance();
};
extern ::google::protobuf::internal::ExplicitlyConstructed<game_end_msg> game_end_msg_default_instance_;

// -------------------------------------------------------------------

class sit_up_msg : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:msg.sit_up_msg) */ {
 public:
  sit_up_msg();
  virtual ~sit_up_msg();

  sit_up_msg(const sit_up_msg& from);

  inline sit_up_msg& operator=(const sit_up_msg& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const sit_up_msg& default_instance();

  static const sit_up_msg* internal_default_instance();

  void Swap(sit_up_msg* other);

  // implements Message ----------------------------------------------

  inline sit_up_msg* New() const { return New(NULL); }

  sit_up_msg* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const sit_up_msg& from);
  void MergeFrom(const sit_up_msg& from);
  void Clear();
  bool IsInitialized() const;

  size_t ByteSizeLong() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(sit_up_msg* other);
  void UnsafeMergeFrom(const sit_up_msg& from);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional int32 uid = 1;
  void clear_uid();
  static const int kUidFieldNumber = 1;
  ::google::protobuf::int32 uid() const;
  void set_uid(::google::protobuf::int32 value);

  // @@protoc_insertion_point(class_scope:msg.sit_up_msg)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::int32 uid_;
  mutable int _cached_size_;
  friend void  protobuf_InitDefaults_game_2eproto_impl();
  friend void  protobuf_AddDesc_game_2eproto_impl();
  friend void protobuf_AssignDesc_game_2eproto();
  friend void protobuf_ShutdownFile_game_2eproto();

  void InitAsDefaultInstance();
};
extern ::google::protobuf::internal::ExplicitlyConstructed<sit_up_msg> sit_up_msg_default_instance_;

// -------------------------------------------------------------------

class sit_down_msg : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:msg.sit_down_msg) */ {
 public:
  sit_down_msg();
  virtual ~sit_down_msg();

  sit_down_msg(const sit_down_msg& from);

  inline sit_down_msg& operator=(const sit_down_msg& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const sit_down_msg& default_instance();

  static const sit_down_msg* internal_default_instance();

  void Swap(sit_down_msg* other);

  // implements Message ----------------------------------------------

  inline sit_down_msg* New() const { return New(NULL); }

  sit_down_msg* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const sit_down_msg& from);
  void MergeFrom(const sit_down_msg& from);
  void Clear();
  bool IsInitialized() const;

  size_t ByteSizeLong() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(sit_down_msg* other);
  void UnsafeMergeFrom(const sit_down_msg& from);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional int32 uid = 1;
  void clear_uid();
  static const int kUidFieldNumber = 1;
  ::google::protobuf::int32 uid() const;
  void set_uid(::google::protobuf::int32 value);

  // @@protoc_insertion_point(class_scope:msg.sit_down_msg)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::int32 uid_;
  mutable int _cached_size_;
  friend void  protobuf_InitDefaults_game_2eproto_impl();
  friend void  protobuf_AddDesc_game_2eproto_impl();
  friend void protobuf_AssignDesc_game_2eproto();
  friend void protobuf_ShutdownFile_game_2eproto();

  void InitAsDefaultInstance();
};
extern ::google::protobuf::internal::ExplicitlyConstructed<sit_down_msg> sit_down_msg_default_instance_;

// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// game_start_msg

// optional int32 state = 1;
inline void game_start_msg::clear_state() {
  state_ = 0;
}
inline ::google::protobuf::int32 game_start_msg::state() const {
  // @@protoc_insertion_point(field_get:msg.game_start_msg.state)
  return state_;
}
inline void game_start_msg::set_state(::google::protobuf::int32 value) {
  
  state_ = value;
  // @@protoc_insertion_point(field_set:msg.game_start_msg.state)
}

inline const game_start_msg* game_start_msg::internal_default_instance() {
  return &game_start_msg_default_instance_.get();
}
// -------------------------------------------------------------------

// game_end_msg

// optional int32 state = 1;
inline void game_end_msg::clear_state() {
  state_ = 0;
}
inline ::google::protobuf::int32 game_end_msg::state() const {
  // @@protoc_insertion_point(field_get:msg.game_end_msg.state)
  return state_;
}
inline void game_end_msg::set_state(::google::protobuf::int32 value) {
  
  state_ = value;
  // @@protoc_insertion_point(field_set:msg.game_end_msg.state)
}

inline const game_end_msg* game_end_msg::internal_default_instance() {
  return &game_end_msg_default_instance_.get();
}
// -------------------------------------------------------------------

// sit_up_msg

// optional int32 uid = 1;
inline void sit_up_msg::clear_uid() {
  uid_ = 0;
}
inline ::google::protobuf::int32 sit_up_msg::uid() const {
  // @@protoc_insertion_point(field_get:msg.sit_up_msg.uid)
  return uid_;
}
inline void sit_up_msg::set_uid(::google::protobuf::int32 value) {
  
  uid_ = value;
  // @@protoc_insertion_point(field_set:msg.sit_up_msg.uid)
}

inline const sit_up_msg* sit_up_msg::internal_default_instance() {
  return &sit_up_msg_default_instance_.get();
}
// -------------------------------------------------------------------

// sit_down_msg

// optional int32 uid = 1;
inline void sit_down_msg::clear_uid() {
  uid_ = 0;
}
inline ::google::protobuf::int32 sit_down_msg::uid() const {
  // @@protoc_insertion_point(field_get:msg.sit_down_msg.uid)
  return uid_;
}
inline void sit_down_msg::set_uid(::google::protobuf::int32 value) {
  
  uid_ = value;
  // @@protoc_insertion_point(field_set:msg.sit_down_msg.uid)
}

inline const sit_down_msg* sit_down_msg::internal_default_instance() {
  return &sit_down_msg_default_instance_.get();
}
#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------

// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace msg

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_game_2eproto__INCLUDED
