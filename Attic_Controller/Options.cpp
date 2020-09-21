//----------------------------------------------------------------------------------------------------------------
// Options.cpp
//
// This file exists to contain potentially-sesitive configuration options,
// such as network passwords and the like.
// It does, however, contain other options as well.
//
// Author - Joshua Villwock
// Created - 2016-12-30
// License - Mozilla Public License 2.0 (Do what you want, credit the author, must release under same license)
//----------------------------------------------------------------------------------------------------------------

// WiFi settings
#define WIFI_SSID "villwock"
#define WIFI_PASS "SecurePassword"
#define OTA_PASS  "SecurePassword"

//Name of the device.  Both for DHCP name & OTA Name
#define DEVICE_NAME  "ESP_EIDOLON_IPMI"

//What pin is the DHT connect to?
const int DHTPin = 2;
