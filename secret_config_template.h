/************************* WiFi Access Point *********************************/
#define WLAN_SSID "WhateverYourWiFiNetworkIsCalled"  // "...your SSID..." Amusingly, this cannot contains spaces or hyphens.
#define WLAN_PASS "ThePasswordForYourWiFiNetwork"    // "...your password..."

/************************* Adafruit.io Setup *********************************/
#define AIO_SERVER         "io.adafruit.com"                   // Perhaps obviously, the API base URL for adafruit.io.
#define AIO_SERVERPORT     1883                                // use 8883 for SSL (requires a certificate and is a bit of a pain), otherwise use 1883
#define AIO_USERNAME       "YourUsernameAtAdafruitIO"          // "...your AIO username (see https://accounts.adafruit.com)..."
#define AIO_KEY            "aaaabbbbccccddddeeeeffff00001111"  // "...your AIO key..."
#define AIO_FEED_PATH      "/feeds/"
#define AIO_PUBLISH_FEED   "weasleyclockposition"  // The name of the Adafruit IO Feed if operating in PubSub mode.
#define AIO_SUBSCRIBE_FEED "weasleyclockstatus"    // The name of your Adafruit IO subscription for a standard get.
