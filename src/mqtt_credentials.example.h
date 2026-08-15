/**
 * @file mqtt_credentials.example.h
 * @brief Template for mqtt_credentials.h — copy this file to mqtt_credentials.h and fill in your
 * broker's login, if it requires one. mqtt_credentials.h is gitignored so real credentials never
 * get committed.
 *
 * MQTT_BROKER_ADDRESS/MQTT_BROKER_PORT (not sensitive) live in config.h instead; see mqtt_manager.h.
 */
#ifndef MQTT_CREDENTIALS_H
#define MQTT_CREDENTIALS_H

#define MQTT_USERNAME ""  ///< Leave empty if the broker allows anonymous connections.
#define MQTT_PASSWORD ""  ///< Leave empty if the broker allows anonymous connections.

#endif
