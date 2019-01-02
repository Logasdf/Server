// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: data.proto

#include "data.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/port.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// This is a temporary google only hack
#ifdef GOOGLE_PROTOBUF_ENFORCE_UNIQUENESS
#include "third_party/protobuf/version.h"
#endif
// @@protoc_insertion_point(includes)

namespace protobuf_data_2eproto {
extern PROTOBUF_INTERNAL_EXPORT_protobuf_data_2eproto ::google::protobuf::internal::SCCInfo<0> scc_info_Data_DataMapEntry_DoNotUse;
}  // namespace protobuf_data_2eproto
class Data_DataMapEntry_DoNotUseDefaultTypeInternal {
 public:
  ::google::protobuf::internal::ExplicitlyConstructed<Data_DataMapEntry_DoNotUse>
      _instance;
} _Data_DataMapEntry_DoNotUse_default_instance_;
class DataDefaultTypeInternal {
 public:
  ::google::protobuf::internal::ExplicitlyConstructed<Data>
      _instance;
} _Data_default_instance_;
namespace protobuf_data_2eproto {
static void InitDefaultsData_DataMapEntry_DoNotUse() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  {
    void* ptr = &::_Data_DataMapEntry_DoNotUse_default_instance_;
    new (ptr) ::Data_DataMapEntry_DoNotUse();
  }
  ::Data_DataMapEntry_DoNotUse::InitAsDefaultInstance();
}

::google::protobuf::internal::SCCInfo<0> scc_info_Data_DataMapEntry_DoNotUse =
    {{ATOMIC_VAR_INIT(::google::protobuf::internal::SCCInfoBase::kUninitialized), 0, InitDefaultsData_DataMapEntry_DoNotUse}, {}};

static void InitDefaultsData() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  {
    void* ptr = &::_Data_default_instance_;
    new (ptr) ::Data();
    ::google::protobuf::internal::OnShutdownDestroyMessage(ptr);
  }
  ::Data::InitAsDefaultInstance();
}

::google::protobuf::internal::SCCInfo<1> scc_info_Data =
    {{ATOMIC_VAR_INIT(::google::protobuf::internal::SCCInfoBase::kUninitialized), 1, InitDefaultsData}, {
      &protobuf_data_2eproto::scc_info_Data_DataMapEntry_DoNotUse.base,}};

void InitDefaults() {
  ::google::protobuf::internal::InitSCC(&scc_info_Data_DataMapEntry_DoNotUse.base);
  ::google::protobuf::internal::InitSCC(&scc_info_Data.base);
}

::google::protobuf::Metadata file_level_metadata[2];

const ::google::protobuf::uint32 TableStruct::offsets[] GOOGLE_PROTOBUF_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::Data_DataMapEntry_DoNotUse, _has_bits_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::Data_DataMapEntry_DoNotUse, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::Data_DataMapEntry_DoNotUse, key_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::Data_DataMapEntry_DoNotUse, value_),
  0,
  1,
  ~0u,  // no _has_bits_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::Data, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::Data, datamap_),
};
static const ::google::protobuf::internal::MigrationSchema schemas[] GOOGLE_PROTOBUF_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
  { 0, 7, sizeof(::Data_DataMapEntry_DoNotUse)},
  { 9, -1, sizeof(::Data)},
};

static ::google::protobuf::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::google::protobuf::Message*>(&::_Data_DataMapEntry_DoNotUse_default_instance_),
  reinterpret_cast<const ::google::protobuf::Message*>(&::_Data_default_instance_),
};

void protobuf_AssignDescriptors() {
  AddDescriptors();
  AssignDescriptors(
      "data.proto", schemas, file_default_instances, TableStruct::offsets,
      file_level_metadata, NULL, NULL);
}

void protobuf_AssignDescriptorsOnce() {
  static ::google::protobuf::internal::once_flag once;
  ::google::protobuf::internal::call_once(once, protobuf_AssignDescriptors);
}

void protobuf_RegisterTypes(const ::std::string&) GOOGLE_PROTOBUF_ATTRIBUTE_COLD;
void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::internal::RegisterAllTypes(file_level_metadata, 2);
}

void AddDescriptorsImpl() {
  InitDefaults();
  static const char descriptor[] GOOGLE_PROTOBUF_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
      "\n\ndata.proto\"[\n\004Data\022#\n\007dataMap\030\001 \003(\0132\022."
      "Data.DataMapEntry\032.\n\014DataMapEntry\022\013\n\003key"
      "\030\001 \001(\t\022\r\n\005value\030\002 \001(\t:\0028\001b\006proto3"
  };
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
      descriptor, 113);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "data.proto", &protobuf_RegisterTypes);
}

void AddDescriptors() {
  static ::google::protobuf::internal::once_flag once;
  ::google::protobuf::internal::call_once(once, AddDescriptorsImpl);
}
// Force AddDescriptors() to be called at dynamic initialization time.
struct StaticDescriptorInitializer {
  StaticDescriptorInitializer() {
    AddDescriptors();
  }
} static_descriptor_initializer;
}  // namespace protobuf_data_2eproto

// ===================================================================

Data_DataMapEntry_DoNotUse::Data_DataMapEntry_DoNotUse() {}
Data_DataMapEntry_DoNotUse::Data_DataMapEntry_DoNotUse(::google::protobuf::Arena* arena) : SuperType(arena) {}
void Data_DataMapEntry_DoNotUse::MergeFrom(const Data_DataMapEntry_DoNotUse& other) {
  MergeFromInternal(other);
}
::google::protobuf::Metadata Data_DataMapEntry_DoNotUse::GetMetadata() const {
  ::protobuf_data_2eproto::protobuf_AssignDescriptorsOnce();
  return ::protobuf_data_2eproto::file_level_metadata[0];
}
void Data_DataMapEntry_DoNotUse::MergeFrom(
    const ::google::protobuf::Message& other) {
  ::google::protobuf::Message::MergeFrom(other);
}


// ===================================================================

void Data::InitAsDefaultInstance() {
}
#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int Data::kDataMapFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

Data::Data()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  ::google::protobuf::internal::InitSCC(
      &protobuf_data_2eproto::scc_info_Data.base);
  SharedCtor();
  // @@protoc_insertion_point(constructor:Data)
}
Data::Data(const Data& from)
  : ::google::protobuf::Message(),
      _internal_metadata_(NULL) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  datamap_.MergeFrom(from.datamap_);
  // @@protoc_insertion_point(copy_constructor:Data)
}

void Data::SharedCtor() {
}

Data::~Data() {
  // @@protoc_insertion_point(destructor:Data)
  SharedDtor();
}

void Data::SharedDtor() {
}

void Data::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}
const ::google::protobuf::Descriptor* Data::descriptor() {
  ::protobuf_data_2eproto::protobuf_AssignDescriptorsOnce();
  return ::protobuf_data_2eproto::file_level_metadata[kIndexInFileMessages].descriptor;
}

const Data& Data::default_instance() {
  ::google::protobuf::internal::InitSCC(&protobuf_data_2eproto::scc_info_Data.base);
  return *internal_default_instance();
}


void Data::Clear() {
// @@protoc_insertion_point(message_clear_start:Data)
  ::google::protobuf::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  datamap_.Clear();
  _internal_metadata_.Clear();
}

bool Data::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:Data)
  for (;;) {
    ::std::pair<::google::protobuf::uint32, bool> p = input->ReadTagWithCutoffNoLastTag(127u);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // map<string, string> dataMap = 1;
      case 1: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(10u /* 10 & 0xFF */)) {
          Data_DataMapEntry_DoNotUse::Parser< ::google::protobuf::internal::MapField<
              Data_DataMapEntry_DoNotUse,
              ::std::string, ::std::string,
              ::google::protobuf::internal::WireFormatLite::TYPE_STRING,
              ::google::protobuf::internal::WireFormatLite::TYPE_STRING,
              0 >,
            ::google::protobuf::Map< ::std::string, ::std::string > > parser(&datamap_);
          DO_(::google::protobuf::internal::WireFormatLite::ReadMessageNoVirtual(
              input, &parser));
          DO_(::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
            parser.key().data(), static_cast<int>(parser.key().length()),
            ::google::protobuf::internal::WireFormatLite::PARSE,
            "Data.DataMapEntry.key"));
          DO_(::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
            parser.value().data(), static_cast<int>(parser.value().length()),
            ::google::protobuf::internal::WireFormatLite::PARSE,
            "Data.DataMapEntry.value"));
        } else {
          goto handle_unusual;
        }
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, _internal_metadata_.mutable_unknown_fields()));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:Data)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:Data)
  return false;
#undef DO_
}

void Data::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:Data)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // map<string, string> dataMap = 1;
  if (!this->datamap().empty()) {
    typedef ::google::protobuf::Map< ::std::string, ::std::string >::const_pointer
        ConstPtr;
    typedef ConstPtr SortItem;
    typedef ::google::protobuf::internal::CompareByDerefFirst<SortItem> Less;
    struct Utf8Check {
      static void Check(ConstPtr p) {
        ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
          p->first.data(), static_cast<int>(p->first.length()),
          ::google::protobuf::internal::WireFormatLite::SERIALIZE,
          "Data.DataMapEntry.key");
        ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
          p->second.data(), static_cast<int>(p->second.length()),
          ::google::protobuf::internal::WireFormatLite::SERIALIZE,
          "Data.DataMapEntry.value");
      }
    };

    if (output->IsSerializationDeterministic() &&
        this->datamap().size() > 1) {
      ::std::unique_ptr<SortItem[]> items(
          new SortItem[this->datamap().size()]);
      typedef ::google::protobuf::Map< ::std::string, ::std::string >::size_type size_type;
      size_type n = 0;
      for (::google::protobuf::Map< ::std::string, ::std::string >::const_iterator
          it = this->datamap().begin();
          it != this->datamap().end(); ++it, ++n) {
        items[static_cast<ptrdiff_t>(n)] = SortItem(&*it);
      }
      ::std::sort(&items[0], &items[static_cast<ptrdiff_t>(n)], Less());
      ::std::unique_ptr<Data_DataMapEntry_DoNotUse> entry;
      for (size_type i = 0; i < n; i++) {
        entry.reset(datamap_.NewEntryWrapper(
            items[static_cast<ptrdiff_t>(i)]->first, items[static_cast<ptrdiff_t>(i)]->second));
        ::google::protobuf::internal::WireFormatLite::WriteMessageMaybeToArray(
            1, *entry, output);
        Utf8Check::Check(items[static_cast<ptrdiff_t>(i)]);
      }
    } else {
      ::std::unique_ptr<Data_DataMapEntry_DoNotUse> entry;
      for (::google::protobuf::Map< ::std::string, ::std::string >::const_iterator
          it = this->datamap().begin();
          it != this->datamap().end(); ++it) {
        entry.reset(datamap_.NewEntryWrapper(
            it->first, it->second));
        ::google::protobuf::internal::WireFormatLite::WriteMessageMaybeToArray(
            1, *entry, output);
        Utf8Check::Check(&*it);
      }
    }
  }

  if ((_internal_metadata_.have_unknown_fields() &&  ::google::protobuf::internal::GetProto3PreserveUnknownsDefault())) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        (::google::protobuf::internal::GetProto3PreserveUnknownsDefault()   ? _internal_metadata_.unknown_fields()   : _internal_metadata_.default_instance()), output);
  }
  // @@protoc_insertion_point(serialize_end:Data)
}

::google::protobuf::uint8* Data::InternalSerializeWithCachedSizesToArray(
    bool deterministic, ::google::protobuf::uint8* target) const {
  (void)deterministic; // Unused
  // @@protoc_insertion_point(serialize_to_array_start:Data)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // map<string, string> dataMap = 1;
  if (!this->datamap().empty()) {
    typedef ::google::protobuf::Map< ::std::string, ::std::string >::const_pointer
        ConstPtr;
    typedef ConstPtr SortItem;
    typedef ::google::protobuf::internal::CompareByDerefFirst<SortItem> Less;
    struct Utf8Check {
      static void Check(ConstPtr p) {
        ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
          p->first.data(), static_cast<int>(p->first.length()),
          ::google::protobuf::internal::WireFormatLite::SERIALIZE,
          "Data.DataMapEntry.key");
        ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
          p->second.data(), static_cast<int>(p->second.length()),
          ::google::protobuf::internal::WireFormatLite::SERIALIZE,
          "Data.DataMapEntry.value");
      }
    };

    if (deterministic &&
        this->datamap().size() > 1) {
      ::std::unique_ptr<SortItem[]> items(
          new SortItem[this->datamap().size()]);
      typedef ::google::protobuf::Map< ::std::string, ::std::string >::size_type size_type;
      size_type n = 0;
      for (::google::protobuf::Map< ::std::string, ::std::string >::const_iterator
          it = this->datamap().begin();
          it != this->datamap().end(); ++it, ++n) {
        items[static_cast<ptrdiff_t>(n)] = SortItem(&*it);
      }
      ::std::sort(&items[0], &items[static_cast<ptrdiff_t>(n)], Less());
      ::std::unique_ptr<Data_DataMapEntry_DoNotUse> entry;
      for (size_type i = 0; i < n; i++) {
        entry.reset(datamap_.NewEntryWrapper(
            items[static_cast<ptrdiff_t>(i)]->first, items[static_cast<ptrdiff_t>(i)]->second));
        target = ::google::protobuf::internal::WireFormatLite::
                   InternalWriteMessageNoVirtualToArray(
                       1, *entry, deterministic, target);
;
        Utf8Check::Check(items[static_cast<ptrdiff_t>(i)]);
      }
    } else {
      ::std::unique_ptr<Data_DataMapEntry_DoNotUse> entry;
      for (::google::protobuf::Map< ::std::string, ::std::string >::const_iterator
          it = this->datamap().begin();
          it != this->datamap().end(); ++it) {
        entry.reset(datamap_.NewEntryWrapper(
            it->first, it->second));
        target = ::google::protobuf::internal::WireFormatLite::
                   InternalWriteMessageNoVirtualToArray(
                       1, *entry, deterministic, target);
;
        Utf8Check::Check(&*it);
      }
    }
  }

  if ((_internal_metadata_.have_unknown_fields() &&  ::google::protobuf::internal::GetProto3PreserveUnknownsDefault())) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        (::google::protobuf::internal::GetProto3PreserveUnknownsDefault()   ? _internal_metadata_.unknown_fields()   : _internal_metadata_.default_instance()), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:Data)
  return target;
}

size_t Data::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:Data)
  size_t total_size = 0;

  if ((_internal_metadata_.have_unknown_fields() &&  ::google::protobuf::internal::GetProto3PreserveUnknownsDefault())) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        (::google::protobuf::internal::GetProto3PreserveUnknownsDefault()   ? _internal_metadata_.unknown_fields()   : _internal_metadata_.default_instance()));
  }
  // map<string, string> dataMap = 1;
  total_size += 1 *
      ::google::protobuf::internal::FromIntSize(this->datamap_size());
  {
    ::std::unique_ptr<Data_DataMapEntry_DoNotUse> entry;
    for (::google::protobuf::Map< ::std::string, ::std::string >::const_iterator
        it = this->datamap().begin();
        it != this->datamap().end(); ++it) {
      entry.reset(datamap_.NewEntryWrapper(it->first, it->second));
      total_size += ::google::protobuf::internal::WireFormatLite::
          MessageSizeNoVirtual(*entry);
    }
  }

  int cached_size = ::google::protobuf::internal::ToCachedSize(total_size);
  SetCachedSize(cached_size);
  return total_size;
}

void Data::MergeFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:Data)
  GOOGLE_DCHECK_NE(&from, this);
  const Data* source =
      ::google::protobuf::internal::DynamicCastToGenerated<const Data>(
          &from);
  if (source == NULL) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:Data)
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:Data)
    MergeFrom(*source);
  }
}

void Data::MergeFrom(const Data& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:Data)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  datamap_.MergeFrom(from.datamap_);
}

void Data::CopyFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:Data)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void Data::CopyFrom(const Data& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:Data)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool Data::IsInitialized() const {
  return true;
}

void Data::Swap(Data* other) {
  if (other == this) return;
  InternalSwap(other);
}
void Data::InternalSwap(Data* other) {
  using std::swap;
  datamap_.Swap(&other->datamap_);
  _internal_metadata_.Swap(&other->_internal_metadata_);
}

::google::protobuf::Metadata Data::GetMetadata() const {
  protobuf_data_2eproto::protobuf_AssignDescriptorsOnce();
  return ::protobuf_data_2eproto::file_level_metadata[kIndexInFileMessages];
}


// @@protoc_insertion_point(namespace_scope)
namespace google {
namespace protobuf {
template<> GOOGLE_PROTOBUF_ATTRIBUTE_NOINLINE ::Data_DataMapEntry_DoNotUse* Arena::CreateMaybeMessage< ::Data_DataMapEntry_DoNotUse >(Arena* arena) {
  return Arena::CreateInternal< ::Data_DataMapEntry_DoNotUse >(arena);
}
template<> GOOGLE_PROTOBUF_ATTRIBUTE_NOINLINE ::Data* Arena::CreateMaybeMessage< ::Data >(Arena* arena) {
  return Arena::CreateInternal< ::Data >(arena);
}
}  // namespace protobuf
}  // namespace google

// @@protoc_insertion_point(global_scope)
