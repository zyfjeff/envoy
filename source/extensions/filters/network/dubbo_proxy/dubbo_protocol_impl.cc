#include "extensions/filters/network/dubbo_proxy/dubbo_protocol_impl.h"

#include "common/common/assert.h"

#include "extensions/filters/network/dubbo_proxy/buffer_helper.h"

namespace Envoy {
namespace Extensions {
namespace NetworkFilters {
namespace DubboProxy {

// Consistent with the SerializationType
bool isValidSerializationType(SerializationType type) {
  switch (type) {
  case SerializationType::Hessian:
  case SerializationType::Json:
    break;
  default:
    return false;
  }
  return true;
}

// Consistent with the ResponseStatus
bool isValidResponseStatus(ResponseStatus status) {
  switch (status) {
  case ResponseStatus::Ok:
  case ResponseStatus::ClientTimeout:
  case ResponseStatus::ServerTimeout:
  case ResponseStatus::BadRequest:
  case ResponseStatus::BadResponse:
  case ResponseStatus::ServiceNotFound:
  case ResponseStatus::ServiceError:
  case ResponseStatus::ClientError:
  case ResponseStatus::ServerThreadpoolExhaustedError:
    break;
  default:
    return false;
  }
  return true;
}

void RequestMessageImpl::fromBuffer(Buffer::Instance& data) {
  ASSERT(data.length() >= DubboProtocolImpl::MessageSize);
  uint8_t flag = BufferHelper::peekU8(data, 2);
  is_two_way_ = (flag & TwoWayMask) == TwoWayMask ? true : false;
  type_ = static_cast<SerializationType>(flag & SerializationTypeMask);
  if (!isValidSerializationType(type_)) {
    throw EnvoyException(
        fmt::format("invalid dubbo message serialization type {}",
                    static_cast<std::underlying_type<SerializationType>::type>(type_)));
  }
}

void ResponseMessageImpl::fromBuffer(Buffer::Instance& data) {
  ASSERT(data.length() >= DubboProtocolImpl::MessageSize);
  status_ = static_cast<ResponseStatus>(BufferHelper::peekU8(data, 3));
  if (!isValidResponseStatus(status_)) {
    throw EnvoyException(
        fmt::format("invalid dubbo message response status {}",
                    static_cast<std::underlying_type<ResponseStatus>::type>(status_)));
  }
}

bool DubboProtocolImpl::decode(Buffer::Instance& buffer, Protocol::Context* context) {
  if (buffer.length() < MessageSize) {
    return false;
  }

  uint16_t magic_number = BufferHelper::peekU16(buffer);
  if (magic_number != MagicNumber) {
    throw EnvoyException(fmt::format("invalid dubbo message magic number {}", magic_number));
  }

  uint8_t flag = BufferHelper::peekU8(buffer, 2);
  MessageType type = static_cast<MessageType>((flag & MessageTypeMask) >> 7);
  bool is_event = (flag & EventMask) == EventMask ? true : false;
  int64_t request_id = BufferHelper::peekI64(buffer, 4);
  int32_t body_size = BufferHelper::peekI32(buffer, 12);

  if (body_size > MaxBodySize || body_size <= 0) {
    throw EnvoyException(fmt::format("invalid dubbo message size {}", body_size));
  }

  context->body_size_ = body_size;

  if (type == MessageType::Request) {
    RequestMessageImplPtr req =
        std::make_unique<RequestMessageImpl>(request_id, body_size, is_event);
    req->fromBuffer(buffer);
    context->is_request_ = true;
    callbacks_.onRequestMessage(std::move(req));
  } else {
    ResponseMessageImplPtr res =
        std::make_unique<ResponseMessageImpl>(request_id, body_size, is_event);
    res->fromBuffer(buffer);
    callbacks_.onResponseMessage(std::move(res));
  }

  buffer.drain(16);
  return true;
}

} // namespace DubboProxy
} // namespace NetworkFilters
} // namespace Extensions
} // namespace Envoy