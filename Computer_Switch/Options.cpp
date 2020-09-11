//----------------------------------------------------------------------------------------------------------------
// Options.cpp
//
// This file exists to contain potentially-sesitive configuration options,
// such as network passwords and the like.
// It does, however, contain other options as well.
//
// Author - Joshua Villwock
// Created - 2020-09-11
// License - Mozilla Public License 2.0 (Do what you want, credit the author, must release under same license)
//----------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------

//Name of the device.  Both for DHCP name & OTA Name
#define DEVICE_NAME  "ESP_EIDOLON_IPMI"

//Which pins are connected?
#define RELAY_PIN    D1
#define POWER_BUTTON D4
#define POWER_LED    D2
#define DHT_PIN      D5

#define DHTTYPE      DHT22

#define TEMP_CHECK_FREQUENCY     60000 // How often to report temperature
#define POWER_CHECK_FREQUENCY    10000 // How often to check for power off state
#define GRATUITOUS_ARP_FREQUENCY 10000 // How often to send the gratutousARP packet

// WiFi parameters
#define WIFI_SSID "name"
#define WIFI_PASS "SecurePassword"
#define OTA_PASS  "SecurePassword"
//------------------------------------------------------------
//------------------------------------------------------------
