#include "test/extensions/filters/network/dubbo_proxy/mocks.h"

#include "gtest/gtest.h"

using testing::ReturnRef;

namespace Envoy {
namespace Extensions {
namespace NetworkFilters {
namespace DubboProxy {

MockSerialization::MockSerialization() { ON_CALL(*this, name()).WillByDefault(ReturnRef(name_)); }
MockSerialization::~MockSerialization() {}
MockSerializationCallbacks::MockSerializationCallbacks() {}
MockSerializationCallbacks::~MockSerializationCallbacks() {}

MockProtocolCallbacks::MockProtocolCallbacks() {}
MockProtocolCallbacks::~MockProtocolCallbacks() {}

MockProtocol::MockProtocol() { ON_CALL(*this, name()).WillByDefault(ReturnRef(name_)); }
MockProtocol::~MockProtocol() {}

} // namespace DubboProxy
} // namespace NetworkFilters
} // namespace Extensions
} // namespace Envoy
