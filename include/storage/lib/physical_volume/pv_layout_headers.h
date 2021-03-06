#pragma once
#include "../common/macros.h"
#include "../../settings.h"
#include "../utils/Version.hpp"
#include "../common/offset_type_traits.hpp"
#include <cstdint>
#include <type_traits>        // for underlying_type_t

namespace yas {
namespace pv_layout_headers {

constexpr int32_t kBinCount = 11;

// PV layout contains:
// PVHeader
// Freelists
// Inverted index
// Data

STRUCT_PACK(
struct PVHeader {
  uint8_t signature_[6] = { 'Y', 'A', 'S', '_', 'P', 'V' };                        //  + 6 bytes
  utils::Version version_;                                                         //  + 2 bytes
  DOffsetType pv_size_;                                                             //  + 8 bytes
  int32_t cluster_size_ = kDefaultClusterSize;                                     //  + 4 bytes
  int32_t priority_;                                                               //  + 4 bytes
  DOffsetType inverted_index_offset_;                                               //  + sizeof(OffsetType) : 4 or 8 bytes
  int32_t freelist_bins_count_ = kBinCount;                                        //  + 4 bytes
});

STRUCT_PACK(
template<typename OffsetType>
struct FreelistHeader {
  OffsetType free_bins_[kBinCount];
});

enum PVType : uint8_t {
  kInt8   = 0,
  kUint8  = 1,
  kInt16  = 2,
  kUint16 = 3,
  kInt32  = 4,
  kUint32 = 5,
  kFloat  = 6,

  kDouble = 7,
  kInt64  = 8,
  kUint64 = 9,

  kString = 10,
  kBlob   = 11,
  kInvertedIndex = 12,

  kEmpty4Simple = 13,
  kEmpty8Simple = 14,
  kEmptyComplex = 15
};

enum PVTypeState : uint8_t {
  kEmpty = 0x00,
  kIsExpired = 0x01,      // has expired time
  kComplexBegin = 0x02,   // beginning of Complex type
  kComplexSequel = 0x04   // next chunk of Complex type
};

constexpr PVTypeState operator|(PVTypeState lhs, PVTypeState rhs) {
  using UnderlyingType = std::underlying_type_t<PVTypeState>;
  return (PVTypeState)(static_cast<UnderlyingType>(lhs) | static_cast<UnderlyingType>(rhs));
}

constexpr PVTypeState operator&(PVTypeState lhs, PVTypeState rhs) {
  using UnderlyingType = std::underlying_type_t<PVTypeState>;
  return (PVTypeState)(static_cast<UnderlyingType>(lhs) & static_cast<UnderlyingType>(rhs));
}

constexpr PVTypeState& operator|=(PVTypeState &lhs, PVTypeState rhs) {
  using UnderlyingType = std::underlying_type_t<PVTypeState>;
  lhs = (PVTypeState)(static_cast<UnderlyingType>(lhs) | static_cast<UnderlyingType>(rhs));
  return lhs;
}

// I assume that the most common types would be types with 4 and 8 bytes size, so there are specially
// size-optimized headers for them. Each header could be in 2 states: allocated and freed. Allocated 
// headers contain expired_time and data. Freed headers contain the link instead of data. This link 
// points to the next freed header with size in the same bucket's range. These freelists then would be
// used for allocating new entries in file. It will lead to decrease fragmentation and expensive 
// extension process of the physical volume on hdd. Also note that expired_time can also be placed at 
// inverted index and there is also a tradeoff between size/speed. I chose the file location to reduce
// possible RAM costs (following the reqs in proposal).
//
// P.S. I will also note that most device types have some kind of read/write cache. F.e. read/write cache
// in most of std::fstream realizations, aligment and rounding in boost::interprocess::mapping_file. 
// Given this fact it can be said that it would be faster to implement the same logic as in jemalloc or
// low fragmentation heap in Windows >= 8. The first is designed to place objects of the same size next to 
// each other to reach cache-friendly using and the second has buckets with preallocated headers and 
// exploit similar conception. But i think that for test task it is too complex :).

STRUCT_PACK(
  struct PVState {
  PVType value_type_;
  PVTypeState value_state_;
});

STRUCT_PACK(
template <typename T>
struct CommonTypeHeader : public PVState {
  uint16_t expired_time_high_;    // goodbye 2038 problem :)
  STRUCT_PACK(
  union {
    STRUCT_PACK(
    struct {
      uint32_t expired_time_low_;
      T value_;
    });
    DOffsetType next_free_entry_offset_;
  });
});

STRUCT_PACK(
struct Simple4TypeHeader : public CommonTypeHeader<uint32_t> {
});

STRUCT_PACK(
struct Simple8TypeHeader : public CommonTypeHeader<uint64_t> {
});

STRUCT_PACK(
struct ComplexTypeHeader : public PVState {
  uint16_t expired_time_high_ = 0;
  uint32_t expired_time_low_ = 0;
  DOffsetType overall_size_ = offset_traits<DOffsetType>::NonExistValue();
  DOffsetType chunk_size_ = offset_traits<DOffsetType>::NonExistValue();
  DOffsetType sequel_offset_ = offset_traits<DOffsetType>::NonExistValue();
  union {
    DOffsetType next_free_entry_offset_ = offset_traits<DOffsetType>::NonExistValue();
    uint8_t data_[1];
  };
});

static_assert(sizeof(float) == 4, "please fix type mapping: float should be 4 bytes at size");

constexpr uint32_t kTimeSize = sizeof ComplexTypeHeader::expired_time_high_ + sizeof ComplexTypeHeader::expired_time_low_;

static_assert((20 + 2*sizeof(DOffsetType)) == sizeof(PVHeader), "PVHeader should be 28+sizeof(OffsetType) bytes long - please check aligments and type size on your setup");
static_assert(12 == sizeof(Simple4TypeHeader), "Simple4TypeHeader should be 12 bytes long - please check aligments and type size on your setup");
static_assert(16 == sizeof(Simple8TypeHeader), "Simple8TypeHeader should be 16 bytes long - please check aligments and type size on your setup");
static_assert((8 + 4*sizeof(DOffsetType)) == sizeof(ComplexTypeHeader), "ComplexTypeHeader should be 8 + 4*sizeof(OffsetType) bytes long - please check aligments and type size on your setup");

} // namespace pv_layout_headers
} // namespace yas
