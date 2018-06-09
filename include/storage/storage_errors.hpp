#pragma once
#include <cstdint>

namespace yas {
namespace storage {

enum class StorageError : uint32_t {
  kSuccess = 0,
  kDeviceGeneralError = 1,
  kDeviceReadError = 2,
  kDeviceWriteError = 3,
  kDeviceExpandError = 4,
  kInvertedIndexDeserializationError = 5,
  kInvertedIndexDeserializationVersionUnsupportedError = 6,   // different type from kInvertedIndexDeserializationError
                                                              // because of it could be recover from it with different
                                                              // version header
  kCorruptedHeaderError = 7,
  kInvalidPVSignatureError = 8,
  kPVVersionNotSupported = 9,
  kKeyNotFound = 10,
  kKeyExpired = 11,
  kKeyDoesntExpired = 12,
  kPVMountPointManagerAdapterNotReady = 13,
  kIncorrectStorageValue = 14,                                // used for valueless std::variant
  kKeyAlreadyCreated = 15,
  kPVVersionUnsupported = 16,
  kUnknownError = 17,
  kCatalogNotFoundError = 18,
  kMemoryNotEnough = 19,

  kUnknownExceptionType
};

struct StorageErrorDescriptor {
  StorageErrorDescriptor(std::string message, StorageError error_code)
      : message_(std::move(message)),
        error_code_(error_code)
  {}
  StorageError error_code_;
  std::string message_;           // can be empty

  operator bool() const {
    return StorageError::kSuccess == error_code_ ;
  }
};

} // namespace storage
} // namespace yas