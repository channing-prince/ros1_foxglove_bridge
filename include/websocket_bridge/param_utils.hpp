#pragma once

#include <regex>
#include <string>
#include <vector>

#include <xmlrpcpp/XmlRpc.h>

#include <websocket_bridge/parameter.hpp>

namespace websocket_bridge {

websocket::Parameter fromRosParam(const std::string& name, const XmlRpc::XmlRpcValue& value);
XmlRpc::XmlRpcValue toRosParam(const websocket::ParameterValue& param);

std::vector<std::regex> parseRegexPatterns(const std::vector<std::string>& strings);

}  // namespace websocket_bridge
