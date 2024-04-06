#pragma once

#include <Arduino.h>
#include <WebSocketsServer.h>

namespace WebSocket
{

  /// Initialize the websocket instance
  void initialize(const String &hostname = "fastledhub");

  /// Handle websocket
  void handle();

  /// Broadcast the current animation status to all active websocket connections
  void broadcastStatus();

  /// Broadcast the current color to all active websocket connections
  void broadcastColor(const String &color);

  /// Broadcast the current lux to all active websocket connections
  void broadcastLux(const float &lux);

} // namespace Websocket
