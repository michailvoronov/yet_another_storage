#pragma once
#include <limits>

namespace yas {
namespace index_helper {

template<typename LeafType>
struct leaf_traits {
  static LeafType NonExistValue() {
    return static_cast<LeafType>(std::numeric_limits<LeafType>::max());
  }

  static bool IsExistValue(LeafType value) {
    return value != NonExistValue();
  }
};

} // namespace index_helper
} // namespace yas
