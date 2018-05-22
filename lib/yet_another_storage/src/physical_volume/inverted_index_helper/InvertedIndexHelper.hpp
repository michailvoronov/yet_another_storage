#pragma once
#include "AhoCorasickEngine.hpp"
#include "AhoCorasickSerializationHelper.hpp"
#include "../../utils/Version.hpp"
#include "../../common/common.h"
#include <memory>
#include <string_view>
#include <type_traits>

namespace yas {
namespace index_helper {

template <typename CharType, typename LeafType>
class InvertedIndexHelper {
  using StringViewType = std::basic_string_view<CharType>;
public:
  InvertedIndexHelper() = default;
  ~InvertedIndexHelper() = default;

  InvertedIndexHelper(InvertedIndexHelper &&other) noexcept
      : engine_(std::move(other.engine_))
  {}

  bool Insert(StringViewType key, LeafType leaf) {
    if (key.empty()) {
      return false;
    }
    return engine_.Insert(key, leaf);
  }

  LeafType Get(StringViewType key) noexcept {
    if (key.empty()) {
      return leaf_traits<Leaf>::NonExistValue();
    }
    return engine_.Get(key);
  }

  const LeafType Get(StringViewType key) const noexcept {
    if (key.empty()) {
      return leaf_traits<Leaf>::NonExistValue();
    }
    return engine_.Get(key);
  }

  bool Delete(StringViewType key) {
    if (key.empty()) {
      return false;
    }
    return engine_.Delete(key);
  }

  bool HasKey(StringViewType key) const noexcept {
    if (key.empty()) {
      return false;
    }
    return engine_.HasKey(key);
  }

  template<typename IdType>
  ByteVector Serialize(utils::Version version) const {
    AhoCorasickSerializationHelper<CharType, LeafType, IdType> serializer(version);
    return serializer.Serialize(engine_);
  }

  template<typename IdType, typename Iterator>
  static InvertedIndexHelper Deserialize(Iterator begin, Iterator end, utils::Version version) {
    AhoCorasickSerializationHelper<CharType, LeafType, IdType> serializer(version);
    InvertedIndexHelper<CharType, LeafType> index_helper;
    serializer.Deserialize(begin, end, index_helper.engine_);

    return index_helper;
  }

  InvertedIndexHelper(const InvertedIndexHelper&) = delete;
  InvertedIndexHelper& operator=(const InvertedIndexHelper&) = delete;
  InvertedIndexHelper& operator=(InvertedIndexHelper&&) = delete;
private:
  // TODO: engine probably should be template or inner class (pimpl?) - need to decide later
  AhoCorasickEngine<CharType, LeafType> engine_;
};

} // namespace index_helper
} // namespace yas
