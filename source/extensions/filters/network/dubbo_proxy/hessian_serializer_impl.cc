#include "extensions/filters/network/dubbo_proxy/hessian_serializer_impl.h"

#include "envoy/common/exception.h"

#include "common/common/assert.h"
#include "common/common/macros.h"

#include "extensions/filters/network/dubbo_proxy/hessian_utils.h"
#include "extensions/filters/network/dubbo_proxy/serialization.h"
#include "extensions/filters/network/dubbo_proxy/serialization_impl.h"

namespace Envoy {
namespace Extensions {
namespace NetworkFilters {
namespace DubboProxy {

enum class RpcResponseType : unsigned char {
  ResponseWithException = 0,
  ResponseWithValue = 1,
  ResponseWithNullValue = 2,
  ResponseWithExceptionWithAttachments = 3,
  ResponseValueWithAttachments = 4,
  ResponseNullValueWithAttachments = 5,

  // ATTENTION: MAKE SURE THIS REMAINS EQUAL TO THE LAST RpcResponseType TYPE
  LastResponseType = ResponseNullValueWithAttachments,
};

void HessianSerializerImpl::deserializeRpcInvocation(Buffer::Instance& buffer, size_t body_size) {
  ASSERT(buffer.length() >= body_size);
  size_t total_size = 0, size;
  // TODO(zyfjeff:) Add dubbo version format checker
  std::string dubbo_version = HessianUtils::peekString(buffer, &size);
  total_size = total_size + size;

  // TODO(zyfjeff:) Add service name format checker
  std::string service_name = HessianUtils::peekString(buffer, &size, total_size);
  total_size = total_size + size;

  // TODO(zyfjeff:) Add version format checker
  std::string service_version = HessianUtils::peekString(buffer, &size, total_size);
  total_size = total_size + size;

  // TODO(zyfjeff:) Add method name format checker
  std::string method_name = HessianUtils::peekString(buffer, &size, total_size);
  total_size = total_size + size;

  if (body_size < total_size) {
    throw EnvoyException(
        fmt::format("RpcInvocation size({}) large than body size({})", total_size, body_size));
  }

  callbacks_.onRpcInvocation(
      std::make_unique<RpcInvocationImpl>(method_name, service_name, service_version));
  buffer.drain(body_size);
  return;
}

void HessianSerializerImpl::deserializeRpcResult(Buffer::Instance& buffer, size_t body_size) {
  ASSERT(buffer.length() >= body_size);
  size_t total_size = 0;
  bool has_value = true;

  RpcResultPtr result;
  RpcResponseType type = static_cast<RpcResponseType>(HessianUtils::peekInt(buffer, &total_size));

  switch (type) {
  case RpcResponseType::ResponseWithException:
  case RpcResponseType::ResponseWithExceptionWithAttachments:
    result = std::make_unique<RpcResultImpl>(true);
    break;
  case RpcResponseType::ResponseWithValue:
  case RpcResponseType::ResponseWithNullValue:
    has_value = false;
  case RpcResponseType::ResponseValueWithAttachments:
  case RpcResponseType::ResponseNullValueWithAttachments:
    result = std::make_unique<RpcResultImpl>();
    break;
  default:
    throw EnvoyException(fmt::format("not supported return type {}", static_cast<uint8_t>(type)));
  }

  if (body_size < total_size) {
    throw EnvoyException(
        fmt::format("RpcResult size({}) large than body size({})", total_size, body_size));
  }

  if (!has_value && body_size != total_size) {
    throw EnvoyException(
        fmt::format("RpcResult is no value, but the rest of the body size({}) not equal 0",
                    (body_size - total_size)));
  }

  callbacks_.onRpcResult(std::move(result));
  buffer.drain(body_size);
  return;
}

} // namespace DubboProxy
} // namespace NetworkFilters
} // namespace Extensions
} // namespace Envoy