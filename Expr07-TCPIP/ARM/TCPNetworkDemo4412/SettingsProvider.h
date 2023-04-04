/*
 * SETTINGS PROVIDER
 *
 * This file defines default strings, setting keys, deafult values, etc.
 * This file also defines a global-wide setting container.
 *
 */

#ifndef SETTINGSPROVIDER_H
#define SETTINGSPROVIDER_H

#include <QSettings>

/* Main Strings */
#define ST_MAIN_ORGANIZATION  "SEU-BME"
#define ST_MAIN_APPLICATION   "TCPNetworkDemo4412"
#define ST_MAIN_DATABASE_PATH "./Network.ini"

/* Setting Key Names */
//Networking
#define ST_KEY_NETWORKING_PREFIX   "Networking"
#define ST_KEY_SERVER_IP           "ServerIP"
#define ST_KEY_SERVER_PORT         "ServerPort"
#define ST_KEY_IS_AUTORECONN_ON    "IsAutoReconnectEnabled"
#define ST_KEY_AUTORECONN_DELAY_MS "AutoReconnectDelay"
#define ST_KEY_LISTENING_PORT      "ListeningPort"

/* Default Values */
//Networking
#define ST_DEFVAL_SERVER_IP           "127.0.0.1"
#define ST_DEFVAL_SERVER_PORT         "5245"
#define ST_DEFVAL_IS_AUTORECONN_ON    false
#define ST_DEFVAL_AUTORECONN_DELAY_MS 1000
#define ST_DEFVAL_LISTENING_PORT      "6245"

extern QSettings SettingsContainer;

#endif // SETTINGSPROVIDER_H
