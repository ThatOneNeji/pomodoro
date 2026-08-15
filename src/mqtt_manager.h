/**
 * @file mqtt_manager.h
 * @brief Minimal MQTT client scaffold: connects (and reconnects) to the broker configured in
 * config.h/mqtt_credentials.h, once WiFi is up. Doesn't publish or subscribe to anything yet —
 * this is just the connection plumbing for later use.
 */
#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

/**
 * @brief Configure the MQTT client against ::MQTT_BROKER_ADDRESS/::MQTT_BROKER_PORT (see
 * config.h). A no-op if MQTT_BROKER_ADDRESS is empty, the default — most checkouts of this repo
 * won't have a broker to connect to. Doesn't itself connect; call mqttLoop() to do that.
 */
void setupMqtt();

/**
 * @brief Service the MQTT connection: reconnect (throttled) if disconnected, otherwise pump
 * PubSubClient::loop() to service keep-alives. Call once per main loop() iteration; a cheap
 * no-op whenever MQTT is disabled, WiFi isn't connected, or a reconnect was already attempted
 * too recently.
 */
void mqttLoop();

/// @return true if currently connected to the MQTT broker.
bool isMqttConnected();

#endif
