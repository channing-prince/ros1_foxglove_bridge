#include "websocket_bridge/websocket_bridge.hpp"

#include "websocketpp/version.hpp"

namespace websocket {

const char* WebSocketUserAgent() {
  return websocketpp::user_agent;
}

}  // namespace websocket
