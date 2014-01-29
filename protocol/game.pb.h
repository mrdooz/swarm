// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: game.proto

#ifndef PROTOBUF_game_2eproto__INCLUDED
#define PROTOBUF_game_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2005000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2005000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_game_2eproto();
void protobuf_AssignDesc_game_2eproto();
void protobuf_ShutdownFile_game_2eproto();

class Position;
class Monster;
class SwarmState;

// ===================================================================

class Position : public ::google::protobuf::Message {
 public:
  Position();
  virtual ~Position();

  Position(const Position& from);

  inline Position& operator=(const Position& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const Position& default_instance();

  void Swap(Position* other);

  // implements Message ----------------------------------------------

  Position* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const Position& from);
  void MergeFrom(const Position& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional float x = 1;
  inline bool has_x() const;
  inline void clear_x();
  static const int kXFieldNumber = 1;
  inline float x() const;
  inline void set_x(float value);

  // optional float y = 2;
  inline bool has_y() const;
  inline void clear_y();
  static const int kYFieldNumber = 2;
  inline float y() const;
  inline void set_y(float value);

  // @@protoc_insertion_point(class_scope:Position)
 private:
  inline void set_has_x();
  inline void clear_has_x();
  inline void set_has_y();
  inline void clear_has_y();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  float x_;
  float y_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(2 + 31) / 32];

  friend void  protobuf_AddDesc_game_2eproto();
  friend void protobuf_AssignDesc_game_2eproto();
  friend void protobuf_ShutdownFile_game_2eproto();

  void InitAsDefaultInstance();
  static Position* default_instance_;
};
// -------------------------------------------------------------------

class Monster : public ::google::protobuf::Message {
 public:
  Monster();
  virtual ~Monster();

  Monster(const Monster& from);

  inline Monster& operator=(const Monster& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const Monster& default_instance();

  void Swap(Monster* other);

  // implements Message ----------------------------------------------

  Monster* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const Monster& from);
  void MergeFrom(const Monster& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional .Position pos = 1;
  inline bool has_pos() const;
  inline void clear_pos();
  static const int kPosFieldNumber = 1;
  inline const ::Position& pos() const;
  inline ::Position* mutable_pos();
  inline ::Position* release_pos();
  inline void set_allocated_pos(::Position* pos);

  // optional float size = 2;
  inline bool has_size() const;
  inline void clear_size();
  static const int kSizeFieldNumber = 2;
  inline float size() const;
  inline void set_size(float value);

  // @@protoc_insertion_point(class_scope:Monster)
 private:
  inline void set_has_pos();
  inline void clear_has_pos();
  inline void set_has_size();
  inline void clear_has_size();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::Position* pos_;
  float size_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(2 + 31) / 32];

  friend void  protobuf_AddDesc_game_2eproto();
  friend void protobuf_AssignDesc_game_2eproto();
  friend void protobuf_ShutdownFile_game_2eproto();

  void InitAsDefaultInstance();
  static Monster* default_instance_;
};
// -------------------------------------------------------------------

class SwarmState : public ::google::protobuf::Message {
 public:
  SwarmState();
  virtual ~SwarmState();

  SwarmState(const SwarmState& from);

  inline SwarmState& operator=(const SwarmState& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const SwarmState& default_instance();

  void Swap(SwarmState* other);

  // implements Message ----------------------------------------------

  SwarmState* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const SwarmState& from);
  void MergeFrom(const SwarmState& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // repeated .Monster monster = 1;
  inline int monster_size() const;
  inline void clear_monster();
  static const int kMonsterFieldNumber = 1;
  inline const ::Monster& monster(int index) const;
  inline ::Monster* mutable_monster(int index);
  inline ::Monster* add_monster();
  inline const ::google::protobuf::RepeatedPtrField< ::Monster >&
      monster() const;
  inline ::google::protobuf::RepeatedPtrField< ::Monster >*
      mutable_monster();

  // @@protoc_insertion_point(class_scope:SwarmState)
 private:

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::RepeatedPtrField< ::Monster > monster_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(1 + 31) / 32];

  friend void  protobuf_AddDesc_game_2eproto();
  friend void protobuf_AssignDesc_game_2eproto();
  friend void protobuf_ShutdownFile_game_2eproto();

  void InitAsDefaultInstance();
  static SwarmState* default_instance_;
};
// ===================================================================


// ===================================================================

// Position

// optional float x = 1;
inline bool Position::has_x() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void Position::set_has_x() {
  _has_bits_[0] |= 0x00000001u;
}
inline void Position::clear_has_x() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void Position::clear_x() {
  x_ = 0;
  clear_has_x();
}
inline float Position::x() const {
  return x_;
}
inline void Position::set_x(float value) {
  set_has_x();
  x_ = value;
}

// optional float y = 2;
inline bool Position::has_y() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void Position::set_has_y() {
  _has_bits_[0] |= 0x00000002u;
}
inline void Position::clear_has_y() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void Position::clear_y() {
  y_ = 0;
  clear_has_y();
}
inline float Position::y() const {
  return y_;
}
inline void Position::set_y(float value) {
  set_has_y();
  y_ = value;
}

// -------------------------------------------------------------------

// Monster

// optional .Position pos = 1;
inline bool Monster::has_pos() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void Monster::set_has_pos() {
  _has_bits_[0] |= 0x00000001u;
}
inline void Monster::clear_has_pos() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void Monster::clear_pos() {
  if (pos_ != NULL) pos_->::Position::Clear();
  clear_has_pos();
}
inline const ::Position& Monster::pos() const {
  return pos_ != NULL ? *pos_ : *default_instance_->pos_;
}
inline ::Position* Monster::mutable_pos() {
  set_has_pos();
  if (pos_ == NULL) pos_ = new ::Position;
  return pos_;
}
inline ::Position* Monster::release_pos() {
  clear_has_pos();
  ::Position* temp = pos_;
  pos_ = NULL;
  return temp;
}
inline void Monster::set_allocated_pos(::Position* pos) {
  delete pos_;
  pos_ = pos;
  if (pos) {
    set_has_pos();
  } else {
    clear_has_pos();
  }
}

// optional float size = 2;
inline bool Monster::has_size() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void Monster::set_has_size() {
  _has_bits_[0] |= 0x00000002u;
}
inline void Monster::clear_has_size() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void Monster::clear_size() {
  size_ = 0;
  clear_has_size();
}
inline float Monster::size() const {
  return size_;
}
inline void Monster::set_size(float value) {
  set_has_size();
  size_ = value;
}

// -------------------------------------------------------------------

// SwarmState

// repeated .Monster monster = 1;
inline int SwarmState::monster_size() const {
  return monster_.size();
}
inline void SwarmState::clear_monster() {
  monster_.Clear();
}
inline const ::Monster& SwarmState::monster(int index) const {
  return monster_.Get(index);
}
inline ::Monster* SwarmState::mutable_monster(int index) {
  return monster_.Mutable(index);
}
inline ::Monster* SwarmState::add_monster() {
  return monster_.Add();
}
inline const ::google::protobuf::RepeatedPtrField< ::Monster >&
SwarmState::monster() const {
  return monster_;
}
inline ::google::protobuf::RepeatedPtrField< ::Monster >*
SwarmState::mutable_monster() {
  return &monster_;
}


// @@protoc_insertion_point(namespace_scope)

#ifndef SWIG
namespace google {
namespace protobuf {


}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_game_2eproto__INCLUDED
