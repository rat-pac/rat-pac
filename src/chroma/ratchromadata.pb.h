// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: ratchromadata.proto

#ifndef PROTOBUF_ratchromadata_2eproto__INCLUDED
#define PROTOBUF_ratchromadata_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3000000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3000000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
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

namespace ratchroma {

// Internal implementation detail -- do not call these.
void protobuf_AddDesc_ratchromadata_2eproto();
void protobuf_AssignDesc_ratchromadata_2eproto();
void protobuf_ShutdownFile_ratchromadata_2eproto();

class CherenkovPhoton;
class ScintStep;
class ChromaData;

// ===================================================================

class CherenkovPhoton : public ::google::protobuf::Message {
 public:
  CherenkovPhoton();
  virtual ~CherenkovPhoton();

  CherenkovPhoton(const CherenkovPhoton& from);

  inline CherenkovPhoton& operator=(const CherenkovPhoton& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const CherenkovPhoton& default_instance();

  void Swap(CherenkovPhoton* other);

  // implements Message ----------------------------------------------

  inline CherenkovPhoton* New() const { return New(NULL); }

  CherenkovPhoton* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const CherenkovPhoton& from);
  void MergeFrom(const CherenkovPhoton& from);
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
  void InternalSwap(CherenkovPhoton* other);
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

  // optional float x = 2;
  void clear_x();
  static const int kXFieldNumber = 2;
  float x() const;
  void set_x(float value);

  // optional float y = 3;
  void clear_y();
  static const int kYFieldNumber = 3;
  float y() const;
  void set_y(float value);

  // optional float z = 4;
  void clear_z();
  static const int kZFieldNumber = 4;
  float z() const;
  void set_z(float value);

  // optional float t = 5;
  void clear_t();
  static const int kTFieldNumber = 5;
  float t() const;
  void set_t(float value);

  // optional float dx = 6;
  void clear_dx();
  static const int kDxFieldNumber = 6;
  float dx() const;
  void set_dx(float value);

  // optional float dy = 7;
  void clear_dy();
  static const int kDyFieldNumber = 7;
  float dy() const;
  void set_dy(float value);

  // optional float dz = 8;
  void clear_dz();
  static const int kDzFieldNumber = 8;
  float dz() const;
  void set_dz(float value);

  // optional float wavelength = 9;
  void clear_wavelength();
  static const int kWavelengthFieldNumber = 9;
  float wavelength() const;
  void set_wavelength(float value);

  // optional float px = 10;
  void clear_px();
  static const int kPxFieldNumber = 10;
  float px() const;
  void set_px(float value);

  // optional float py = 11;
  void clear_py();
  static const int kPyFieldNumber = 11;
  float py() const;
  void set_py(float value);

  // optional float pz = 12;
  void clear_pz();
  static const int kPzFieldNumber = 12;
  float pz() const;
  void set_pz(float value);

  // @@protoc_insertion_point(class_scope:ratchroma.CherenkovPhoton)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  float x_;
  float y_;
  float z_;
  float t_;
  float dx_;
  float dy_;
  float dz_;
  float wavelength_;
  float px_;
  float py_;
  float pz_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_ratchromadata_2eproto();
  friend void protobuf_AssignDesc_ratchromadata_2eproto();
  friend void protobuf_ShutdownFile_ratchromadata_2eproto();

  void InitAsDefaultInstance();
  static CherenkovPhoton* default_instance_;
};
// -------------------------------------------------------------------

class ScintStep : public ::google::protobuf::Message {
 public:
  ScintStep();
  virtual ~ScintStep();

  ScintStep(const ScintStep& from);

  inline ScintStep& operator=(const ScintStep& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const ScintStep& default_instance();

  void Swap(ScintStep* other);

  // implements Message ----------------------------------------------

  inline ScintStep* New() const { return New(NULL); }

  ScintStep* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const ScintStep& from);
  void MergeFrom(const ScintStep& from);
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
  void InternalSwap(ScintStep* other);
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

  // optional int32 nphotons = 1;
  void clear_nphotons();
  static const int kNphotonsFieldNumber = 1;
  ::google::protobuf::int32 nphotons() const;
  void set_nphotons(::google::protobuf::int32 value);

  // optional float step_start_x = 2;
  void clear_step_start_x();
  static const int kStepStartXFieldNumber = 2;
  float step_start_x() const;
  void set_step_start_x(float value);

  // optional float step_start_y = 3;
  void clear_step_start_y();
  static const int kStepStartYFieldNumber = 3;
  float step_start_y() const;
  void set_step_start_y(float value);

  // optional float step_start_z = 4;
  void clear_step_start_z();
  static const int kStepStartZFieldNumber = 4;
  float step_start_z() const;
  void set_step_start_z(float value);

  // optional float step_start_t = 5;
  void clear_step_start_t();
  static const int kStepStartTFieldNumber = 5;
  float step_start_t() const;
  void set_step_start_t(float value);

  // optional float step_end_x = 6;
  void clear_step_end_x();
  static const int kStepEndXFieldNumber = 6;
  float step_end_x() const;
  void set_step_end_x(float value);

  // optional float step_end_y = 7;
  void clear_step_end_y();
  static const int kStepEndYFieldNumber = 7;
  float step_end_y() const;
  void set_step_end_y(float value);

  // optional float step_end_z = 8;
  void clear_step_end_z();
  static const int kStepEndZFieldNumber = 8;
  float step_end_z() const;
  void set_step_end_z(float value);

  // optional float step_end_t = 9;
  void clear_step_end_t();
  static const int kStepEndTFieldNumber = 9;
  float step_end_t() const;
  void set_step_end_t(float value);

  // optional string material = 10;
  void clear_material();
  static const int kMaterialFieldNumber = 10;
  const ::std::string& material() const;
  void set_material(const ::std::string& value);
  void set_material(const char* value);
  void set_material(const char* value, size_t size);
  ::std::string* mutable_material();
  ::std::string* release_material();
  void set_allocated_material(::std::string* material);

  // @@protoc_insertion_point(class_scope:ratchroma.ScintStep)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::int32 nphotons_;
  float step_start_x_;
  float step_start_y_;
  float step_start_z_;
  float step_start_t_;
  float step_end_x_;
  float step_end_y_;
  float step_end_z_;
  ::google::protobuf::internal::ArenaStringPtr material_;
  float step_end_t_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_ratchromadata_2eproto();
  friend void protobuf_AssignDesc_ratchromadata_2eproto();
  friend void protobuf_ShutdownFile_ratchromadata_2eproto();

  void InitAsDefaultInstance();
  static ScintStep* default_instance_;
};
// -------------------------------------------------------------------

class ChromaData : public ::google::protobuf::Message {
 public:
  ChromaData();
  virtual ~ChromaData();

  ChromaData(const ChromaData& from);

  inline ChromaData& operator=(const ChromaData& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const ChromaData& default_instance();

  void Swap(ChromaData* other);

  // implements Message ----------------------------------------------

  inline ChromaData* New() const { return New(NULL); }

  ChromaData* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const ChromaData& from);
  void MergeFrom(const ChromaData& from);
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
  void InternalSwap(ChromaData* other);
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

  // optional int32 eventid = 1;
  void clear_eventid();
  static const int kEventidFieldNumber = 1;
  ::google::protobuf::int32 eventid() const;
  void set_eventid(::google::protobuf::int32 value);

  // optional int32 ncherenkov = 2;
  void clear_ncherenkov();
  static const int kNcherenkovFieldNumber = 2;
  ::google::protobuf::int32 ncherenkov() const;
  void set_ncherenkov(::google::protobuf::int32 value);

  // optional int32 nsteps = 3;
  void clear_nsteps();
  static const int kNstepsFieldNumber = 3;
  ::google::protobuf::int32 nsteps() const;
  void set_nsteps(::google::protobuf::int32 value);

  // repeated .ratchroma.CherenkovPhoton cherekovdata = 4;
  int cherekovdata_size() const;
  void clear_cherekovdata();
  static const int kCherekovdataFieldNumber = 4;
  const ::ratchroma::CherenkovPhoton& cherekovdata(int index) const;
  ::ratchroma::CherenkovPhoton* mutable_cherekovdata(int index);
  ::ratchroma::CherenkovPhoton* add_cherekovdata();
  const ::google::protobuf::RepeatedPtrField< ::ratchroma::CherenkovPhoton >&
      cherekovdata() const;
  ::google::protobuf::RepeatedPtrField< ::ratchroma::CherenkovPhoton >*
      mutable_cherekovdata();

  // repeated .ratchroma.ScintStep stepdata = 5;
  int stepdata_size() const;
  void clear_stepdata();
  static const int kStepdataFieldNumber = 5;
  const ::ratchroma::ScintStep& stepdata(int index) const;
  ::ratchroma::ScintStep* mutable_stepdata(int index);
  ::ratchroma::ScintStep* add_stepdata();
  const ::google::protobuf::RepeatedPtrField< ::ratchroma::ScintStep >&
      stepdata() const;
  ::google::protobuf::RepeatedPtrField< ::ratchroma::ScintStep >*
      mutable_stepdata();

  // @@protoc_insertion_point(class_scope:ratchroma.ChromaData)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::int32 eventid_;
  ::google::protobuf::int32 ncherenkov_;
  ::google::protobuf::RepeatedPtrField< ::ratchroma::CherenkovPhoton > cherekovdata_;
  ::google::protobuf::RepeatedPtrField< ::ratchroma::ScintStep > stepdata_;
  ::google::protobuf::int32 nsteps_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_ratchromadata_2eproto();
  friend void protobuf_AssignDesc_ratchromadata_2eproto();
  friend void protobuf_ShutdownFile_ratchromadata_2eproto();

  void InitAsDefaultInstance();
  static ChromaData* default_instance_;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// CherenkovPhoton

// optional float x = 2;
inline void CherenkovPhoton::clear_x() {
  x_ = 0;
}
inline float CherenkovPhoton::x() const {
  // @@protoc_insertion_point(field_get:ratchroma.CherenkovPhoton.x)
  return x_;
}
inline void CherenkovPhoton::set_x(float value) {
  
  x_ = value;
  // @@protoc_insertion_point(field_set:ratchroma.CherenkovPhoton.x)
}

// optional float y = 3;
inline void CherenkovPhoton::clear_y() {
  y_ = 0;
}
inline float CherenkovPhoton::y() const {
  // @@protoc_insertion_point(field_get:ratchroma.CherenkovPhoton.y)
  return y_;
}
inline void CherenkovPhoton::set_y(float value) {
  
  y_ = value;
  // @@protoc_insertion_point(field_set:ratchroma.CherenkovPhoton.y)
}

// optional float z = 4;
inline void CherenkovPhoton::clear_z() {
  z_ = 0;
}
inline float CherenkovPhoton::z() const {
  // @@protoc_insertion_point(field_get:ratchroma.CherenkovPhoton.z)
  return z_;
}
inline void CherenkovPhoton::set_z(float value) {
  
  z_ = value;
  // @@protoc_insertion_point(field_set:ratchroma.CherenkovPhoton.z)
}

// optional float t = 5;
inline void CherenkovPhoton::clear_t() {
  t_ = 0;
}
inline float CherenkovPhoton::t() const {
  // @@protoc_insertion_point(field_get:ratchroma.CherenkovPhoton.t)
  return t_;
}
inline void CherenkovPhoton::set_t(float value) {
  
  t_ = value;
  // @@protoc_insertion_point(field_set:ratchroma.CherenkovPhoton.t)
}

// optional float dx = 6;
inline void CherenkovPhoton::clear_dx() {
  dx_ = 0;
}
inline float CherenkovPhoton::dx() const {
  // @@protoc_insertion_point(field_get:ratchroma.CherenkovPhoton.dx)
  return dx_;
}
inline void CherenkovPhoton::set_dx(float value) {
  
  dx_ = value;
  // @@protoc_insertion_point(field_set:ratchroma.CherenkovPhoton.dx)
}

// optional float dy = 7;
inline void CherenkovPhoton::clear_dy() {
  dy_ = 0;
}
inline float CherenkovPhoton::dy() const {
  // @@protoc_insertion_point(field_get:ratchroma.CherenkovPhoton.dy)
  return dy_;
}
inline void CherenkovPhoton::set_dy(float value) {
  
  dy_ = value;
  // @@protoc_insertion_point(field_set:ratchroma.CherenkovPhoton.dy)
}

// optional float dz = 8;
inline void CherenkovPhoton::clear_dz() {
  dz_ = 0;
}
inline float CherenkovPhoton::dz() const {
  // @@protoc_insertion_point(field_get:ratchroma.CherenkovPhoton.dz)
  return dz_;
}
inline void CherenkovPhoton::set_dz(float value) {
  
  dz_ = value;
  // @@protoc_insertion_point(field_set:ratchroma.CherenkovPhoton.dz)
}

// optional float wavelength = 9;
inline void CherenkovPhoton::clear_wavelength() {
  wavelength_ = 0;
}
inline float CherenkovPhoton::wavelength() const {
  // @@protoc_insertion_point(field_get:ratchroma.CherenkovPhoton.wavelength)
  return wavelength_;
}
inline void CherenkovPhoton::set_wavelength(float value) {
  
  wavelength_ = value;
  // @@protoc_insertion_point(field_set:ratchroma.CherenkovPhoton.wavelength)
}

// optional float px = 10;
inline void CherenkovPhoton::clear_px() {
  px_ = 0;
}
inline float CherenkovPhoton::px() const {
  // @@protoc_insertion_point(field_get:ratchroma.CherenkovPhoton.px)
  return px_;
}
inline void CherenkovPhoton::set_px(float value) {
  
  px_ = value;
  // @@protoc_insertion_point(field_set:ratchroma.CherenkovPhoton.px)
}

// optional float py = 11;
inline void CherenkovPhoton::clear_py() {
  py_ = 0;
}
inline float CherenkovPhoton::py() const {
  // @@protoc_insertion_point(field_get:ratchroma.CherenkovPhoton.py)
  return py_;
}
inline void CherenkovPhoton::set_py(float value) {
  
  py_ = value;
  // @@protoc_insertion_point(field_set:ratchroma.CherenkovPhoton.py)
}

// optional float pz = 12;
inline void CherenkovPhoton::clear_pz() {
  pz_ = 0;
}
inline float CherenkovPhoton::pz() const {
  // @@protoc_insertion_point(field_get:ratchroma.CherenkovPhoton.pz)
  return pz_;
}
inline void CherenkovPhoton::set_pz(float value) {
  
  pz_ = value;
  // @@protoc_insertion_point(field_set:ratchroma.CherenkovPhoton.pz)
}

// -------------------------------------------------------------------

// ScintStep

// optional int32 nphotons = 1;
inline void ScintStep::clear_nphotons() {
  nphotons_ = 0;
}
inline ::google::protobuf::int32 ScintStep::nphotons() const {
  // @@protoc_insertion_point(field_get:ratchroma.ScintStep.nphotons)
  return nphotons_;
}
inline void ScintStep::set_nphotons(::google::protobuf::int32 value) {
  
  nphotons_ = value;
  // @@protoc_insertion_point(field_set:ratchroma.ScintStep.nphotons)
}

// optional float step_start_x = 2;
inline void ScintStep::clear_step_start_x() {
  step_start_x_ = 0;
}
inline float ScintStep::step_start_x() const {
  // @@protoc_insertion_point(field_get:ratchroma.ScintStep.step_start_x)
  return step_start_x_;
}
inline void ScintStep::set_step_start_x(float value) {
  
  step_start_x_ = value;
  // @@protoc_insertion_point(field_set:ratchroma.ScintStep.step_start_x)
}

// optional float step_start_y = 3;
inline void ScintStep::clear_step_start_y() {
  step_start_y_ = 0;
}
inline float ScintStep::step_start_y() const {
  // @@protoc_insertion_point(field_get:ratchroma.ScintStep.step_start_y)
  return step_start_y_;
}
inline void ScintStep::set_step_start_y(float value) {
  
  step_start_y_ = value;
  // @@protoc_insertion_point(field_set:ratchroma.ScintStep.step_start_y)
}

// optional float step_start_z = 4;
inline void ScintStep::clear_step_start_z() {
  step_start_z_ = 0;
}
inline float ScintStep::step_start_z() const {
  // @@protoc_insertion_point(field_get:ratchroma.ScintStep.step_start_z)
  return step_start_z_;
}
inline void ScintStep::set_step_start_z(float value) {
  
  step_start_z_ = value;
  // @@protoc_insertion_point(field_set:ratchroma.ScintStep.step_start_z)
}

// optional float step_start_t = 5;
inline void ScintStep::clear_step_start_t() {
  step_start_t_ = 0;
}
inline float ScintStep::step_start_t() const {
  // @@protoc_insertion_point(field_get:ratchroma.ScintStep.step_start_t)
  return step_start_t_;
}
inline void ScintStep::set_step_start_t(float value) {
  
  step_start_t_ = value;
  // @@protoc_insertion_point(field_set:ratchroma.ScintStep.step_start_t)
}

// optional float step_end_x = 6;
inline void ScintStep::clear_step_end_x() {
  step_end_x_ = 0;
}
inline float ScintStep::step_end_x() const {
  // @@protoc_insertion_point(field_get:ratchroma.ScintStep.step_end_x)
  return step_end_x_;
}
inline void ScintStep::set_step_end_x(float value) {
  
  step_end_x_ = value;
  // @@protoc_insertion_point(field_set:ratchroma.ScintStep.step_end_x)
}

// optional float step_end_y = 7;
inline void ScintStep::clear_step_end_y() {
  step_end_y_ = 0;
}
inline float ScintStep::step_end_y() const {
  // @@protoc_insertion_point(field_get:ratchroma.ScintStep.step_end_y)
  return step_end_y_;
}
inline void ScintStep::set_step_end_y(float value) {
  
  step_end_y_ = value;
  // @@protoc_insertion_point(field_set:ratchroma.ScintStep.step_end_y)
}

// optional float step_end_z = 8;
inline void ScintStep::clear_step_end_z() {
  step_end_z_ = 0;
}
inline float ScintStep::step_end_z() const {
  // @@protoc_insertion_point(field_get:ratchroma.ScintStep.step_end_z)
  return step_end_z_;
}
inline void ScintStep::set_step_end_z(float value) {
  
  step_end_z_ = value;
  // @@protoc_insertion_point(field_set:ratchroma.ScintStep.step_end_z)
}

// optional float step_end_t = 9;
inline void ScintStep::clear_step_end_t() {
  step_end_t_ = 0;
}
inline float ScintStep::step_end_t() const {
  // @@protoc_insertion_point(field_get:ratchroma.ScintStep.step_end_t)
  return step_end_t_;
}
inline void ScintStep::set_step_end_t(float value) {
  
  step_end_t_ = value;
  // @@protoc_insertion_point(field_set:ratchroma.ScintStep.step_end_t)
}

// optional string material = 10;
inline void ScintStep::clear_material() {
  material_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& ScintStep::material() const {
  // @@protoc_insertion_point(field_get:ratchroma.ScintStep.material)
  return material_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ScintStep::set_material(const ::std::string& value) {
  
  material_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:ratchroma.ScintStep.material)
}
inline void ScintStep::set_material(const char* value) {
  
  material_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:ratchroma.ScintStep.material)
}
inline void ScintStep::set_material(const char* value, size_t size) {
  
  material_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:ratchroma.ScintStep.material)
}
inline ::std::string* ScintStep::mutable_material() {
  
  // @@protoc_insertion_point(field_mutable:ratchroma.ScintStep.material)
  return material_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* ScintStep::release_material() {
  
  return material_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ScintStep::set_allocated_material(::std::string* material) {
  if (material != NULL) {
    
  } else {
    
  }
  material_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), material);
  // @@protoc_insertion_point(field_set_allocated:ratchroma.ScintStep.material)
}

// -------------------------------------------------------------------

// ChromaData

// optional int32 eventid = 1;
inline void ChromaData::clear_eventid() {
  eventid_ = 0;
}
inline ::google::protobuf::int32 ChromaData::eventid() const {
  // @@protoc_insertion_point(field_get:ratchroma.ChromaData.eventid)
  return eventid_;
}
inline void ChromaData::set_eventid(::google::protobuf::int32 value) {
  
  eventid_ = value;
  // @@protoc_insertion_point(field_set:ratchroma.ChromaData.eventid)
}

// optional int32 ncherenkov = 2;
inline void ChromaData::clear_ncherenkov() {
  ncherenkov_ = 0;
}
inline ::google::protobuf::int32 ChromaData::ncherenkov() const {
  // @@protoc_insertion_point(field_get:ratchroma.ChromaData.ncherenkov)
  return ncherenkov_;
}
inline void ChromaData::set_ncherenkov(::google::protobuf::int32 value) {
  
  ncherenkov_ = value;
  // @@protoc_insertion_point(field_set:ratchroma.ChromaData.ncherenkov)
}

// optional int32 nsteps = 3;
inline void ChromaData::clear_nsteps() {
  nsteps_ = 0;
}
inline ::google::protobuf::int32 ChromaData::nsteps() const {
  // @@protoc_insertion_point(field_get:ratchroma.ChromaData.nsteps)
  return nsteps_;
}
inline void ChromaData::set_nsteps(::google::protobuf::int32 value) {
  
  nsteps_ = value;
  // @@protoc_insertion_point(field_set:ratchroma.ChromaData.nsteps)
}

// repeated .ratchroma.CherenkovPhoton cherekovdata = 4;
inline int ChromaData::cherekovdata_size() const {
  return cherekovdata_.size();
}
inline void ChromaData::clear_cherekovdata() {
  cherekovdata_.Clear();
}
inline const ::ratchroma::CherenkovPhoton& ChromaData::cherekovdata(int index) const {
  // @@protoc_insertion_point(field_get:ratchroma.ChromaData.cherekovdata)
  return cherekovdata_.Get(index);
}
inline ::ratchroma::CherenkovPhoton* ChromaData::mutable_cherekovdata(int index) {
  // @@protoc_insertion_point(field_mutable:ratchroma.ChromaData.cherekovdata)
  return cherekovdata_.Mutable(index);
}
inline ::ratchroma::CherenkovPhoton* ChromaData::add_cherekovdata() {
  // @@protoc_insertion_point(field_add:ratchroma.ChromaData.cherekovdata)
  return cherekovdata_.Add();
}
inline const ::google::protobuf::RepeatedPtrField< ::ratchroma::CherenkovPhoton >&
ChromaData::cherekovdata() const {
  // @@protoc_insertion_point(field_list:ratchroma.ChromaData.cherekovdata)
  return cherekovdata_;
}
inline ::google::protobuf::RepeatedPtrField< ::ratchroma::CherenkovPhoton >*
ChromaData::mutable_cherekovdata() {
  // @@protoc_insertion_point(field_mutable_list:ratchroma.ChromaData.cherekovdata)
  return &cherekovdata_;
}

// repeated .ratchroma.ScintStep stepdata = 5;
inline int ChromaData::stepdata_size() const {
  return stepdata_.size();
}
inline void ChromaData::clear_stepdata() {
  stepdata_.Clear();
}
inline const ::ratchroma::ScintStep& ChromaData::stepdata(int index) const {
  // @@protoc_insertion_point(field_get:ratchroma.ChromaData.stepdata)
  return stepdata_.Get(index);
}
inline ::ratchroma::ScintStep* ChromaData::mutable_stepdata(int index) {
  // @@protoc_insertion_point(field_mutable:ratchroma.ChromaData.stepdata)
  return stepdata_.Mutable(index);
}
inline ::ratchroma::ScintStep* ChromaData::add_stepdata() {
  // @@protoc_insertion_point(field_add:ratchroma.ChromaData.stepdata)
  return stepdata_.Add();
}
inline const ::google::protobuf::RepeatedPtrField< ::ratchroma::ScintStep >&
ChromaData::stepdata() const {
  // @@protoc_insertion_point(field_list:ratchroma.ChromaData.stepdata)
  return stepdata_;
}
inline ::google::protobuf::RepeatedPtrField< ::ratchroma::ScintStep >*
ChromaData::mutable_stepdata() {
  // @@protoc_insertion_point(field_mutable_list:ratchroma.ChromaData.stepdata)
  return &stepdata_;
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace ratchroma

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_ratchromadata_2eproto__INCLUDED
