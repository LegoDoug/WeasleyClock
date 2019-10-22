#include <AccelStepper.h>
#include "Adafruit_IO_Client.h"
#include <CapacitiveSensor.h>
#include <ESP8266WiFi.h>

/**
 * Be sure to set the board type to Adafruit Feather HUZZAH ESP8266.
 * 
 * https://www.amazon.com/gp/product/B015RQ97W8/ref=oh_aui_search_detailpage?ie=UTF8&psc=1
 * 5-wire unipolar steppers with controller
 * Longruner 5x Geared Stepper Motor 28byj 48 Uln2003 5v Stepper Motor Uln2003 Driver 
 * Board for arduino LK67
 * 
 * AWESOME details on these motors/controllers:
 * https://arduino-info.wikispaces.com/SmallSteppers
 * 
 * Better stepper library: AccelStepper
 * http://www.airspayce.com/mikem/arduino/AccelStepper/classAccelStepper.html
 */

/************************* WiFi Access Point *********************************/
//#define WLAN_SSID "R2-D2"                          // "...your SSID..." Amusingly, the cannot contains spaces or hyphens.
//#define WLAN_PASS "hoginthenetwork"                // "...your password..."
//#define WLAN_SSID "CIA Listening Post"           // "...your SSID..."
//#define WLAN_PASS "certifiedinthestateofma"      // "...your password..."
//#define WLAN_SSID "Bonhoeffer's Cafe"
//#define WLAN_PASS "cafe16cafe"
#define WLAN_SSID "VeraGuest"                          // "...your SSID..." Amusingly, the cannot contains spaces or hyphens.
#define WLAN_PASS "DevSecOps01!"                // "...your password..."


/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                                // use 8883 for SSL, otherwise use 1883
#define AIO_USERNAME    "LegoDoug"                          // "...your AIO username (see https://accounts.adafruit.com)..."
#define AIO_KEY         "aa30a41bdbbf48278999b54433b59c19"  // "...your AIO key..."
#define AIO_FEED_PATH   "/feeds/"
#define AIO_PUBLISH_FEED          "weasleyclockposition"
#define AIO_SUBSCRIBE_FEED        "weasleyclockstatus"

// Motor pin definitions
#define motorPin1  14    // IN1 on the ULN2003 driver board
#define motorPin2  12    // IN2 on the ULN2003 driver board
#define motorPin3  13    // IN3 on the ULN2003 driver board
#define motorPin4  15    // IN4 on the ULN2003 driver board

#define DELAY 1
#define HALFSTEP 8

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the WiFi network.
WiFiClient client;
// or... use WiFiFlientSecure for SSL (requires adding a certificate to the code).
// WiFiClientSecure client;

// Create an Adafruit IO Client instance.  Notice that this needs to take a
// WiFiClient object as the first parameter, and as the second parameter a
// default Adafruit IO key to use when accessing feeds (however each feed can
// override this default key value if required, see further below).
Adafruit_IO_Client aio = Adafruit_IO_Client(client, AIO_KEY);

// Alternatively to access a feed with a specific key:
Adafruit_IO_Feed clockFeed = aio.getFeed(AIO_SUBSCRIBE_FEED, AIO_KEY);

// States
const String LD_HOME         = "ld_na";
const String LD_TRAVELING    = "ld_tr";
const String LD_VERACODE     = "ld_of";
const String LD_CHURCH       = "ld_ch";
const String LD_MORTAL_PERIL = "ld_mp";
const String LD_GLOUCESTER   = "ld_gl";
const String PLUS_ONE        = "plus_1";
const String MINUS_ONE       = "minus_1";
const String PLUS_FIVE       = "plus_5";
const String MINUS_FIVE      = "minus_5";
const String NO_MOVEMENT     = "none";

// Steps
const int STEPS_HOME         = 0;
const int STEPS_TRAVELING    = 600;
const int STEPS_VERACODE     = 1250;
const int STEPS_CHURCH       = 1900;
const int STEPS_MORTAL_PERIL = 2600;
const int STEPS_GLOUCESTER   = 3450;
const int STEPS_ONE          = 2 * 32;
const int STEPS_FIVE         = 5 * 32;
const int CAP_SENSOR_THRESHHOLD = 3000;

String fValue = "";

const unsigned long requestInterval = 1000L; // delay between updates, in milliseconds

void advanceWhilePressed();
void stepBySteps(int newPosition, boolean resetWhenDone = false);

AccelStepper clockStepper(HALFSTEP, motorPin1, motorPin3, motorPin2, motorPin4);
CapacitiveSensor cSensor = CapacitiveSensor(5,4); // 1 megohm resistor between pins 4 & 2, pin 2 is sensor pin, add wire, foil


void setup() {
  Serial.begin(115200); // Many examples use Serial.begin(9600), for USB boards, but for the WiFi feather, this number needs to be higher.
  delay(10);

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  clockStepper.setMaxSpeed(1000.0);
  clockStepper.setAcceleration(100.0);
  clockStepper.setSpeed(200);
  clockStepper.setCurrentPosition(0);
}

void loop() {

  // Wait for a bit and read the current feed value.
  Serial.println(F("Waiting ..."));
  delay(requestInterval);
  // To read the latest feed value call the receive function on the feed.
  // The returned object will be a FeedData instance and you can check if it's
  // valid (i.e. was successfully read) by calling isValid(), and then get the
  // value either as a text value, or converted to an int, float, etc.

  Serial.println(F("Checking feed ..."));
  FeedData latest = clockFeed.receive();
  if (latest.isValid()) {
    Serial.print(F("Received value from feed: ")); Serial.println(latest);
    // By default the received feed data item has a string value, however you
    // can use the following functions to attempt to convert it to a numeric
    // value like an int or float.  Each function returns a boolean that indicates
    // if the conversion succeeded, and takes as a parameter by reference the
    // output value. Also, beware. There seems to be a limit on how long the
    // feed value can be. I had trouble when "minus_five" was used, which makes
    // me think the limit is 8 characters.

    // Want some fun? Learng about "conversion from 'FeedData' to non-scalar type 'String' requested" the hard way.
    fValue = latest;

    if(fValue == LD_HOME) {
      Serial.println("Nashua");
      stepBySteps(STEPS_HOME);
    }
    if(fValue == LD_TRAVELING) {
      Serial.println("Traveling");
      stepBySteps(STEPS_TRAVELING);
    }
    if(fValue == LD_VERACODE) {
      Serial.println("Veracode");
      stepBySteps(STEPS_VERACODE);
    }
    if(fValue == LD_CHURCH) {
      Serial.println("Church");
      stepBySteps(STEPS_CHURCH);
    }
    if(fValue == LD_MORTAL_PERIL) {
      Serial.println("Mortal Peril!");
      stepBySteps(STEPS_MORTAL_PERIL);
    }
    if(fValue == LD_GLOUCESTER) {
      Serial.println("Glostah");
      stepBySteps(STEPS_GLOUCESTER);
    }
    if(fValue == PLUS_ONE) {
      Serial.println("Forward one.");
      stepBySteps(clockStepper.currentPosition() + STEPS_ONE, true);
    }
    if(fValue == MINUS_ONE) {
      Serial.println("Back one.");
      stepBySteps(clockStepper.currentPosition() - STEPS_ONE, true);
    }
    if(fValue == PLUS_FIVE) {
      Serial.println("Forward five.");
      stepBySteps(clockStepper.currentPosition() + STEPS_FIVE, true);
    }
    if(fValue == MINUS_FIVE) {
      Serial.println("Back five.");
      stepBySteps(clockStepper.currentPosition() - STEPS_FIVE, true);
    }
    if(fValue == NO_MOVEMENT || fValue == "") {
      Serial.println("Not moving.");
    }
  } else { // is not valid
    Serial.print(F("Failed to receive the latest feed value!"));
  }

  long capSense =  cSensor.capacitiveSensor(30);
  Serial.print(F("Capacity sensor value MAIN: "));
  Serial.println(capSense);
  if (capSense > CAP_SENSOR_THRESHHOLD) {
    advanceWhilePressed();
  }
}

void advanceWhilePressed() {
  boolean pressed = true;
  while (pressed) {
    stepBySteps(clockStepper.currentPosition() + STEPS_FIVE, true);
    Serial.println("Advancing PLUS FIVE.");
    long capSense =  cSensor.capacitiveSensor(30);
    Serial.print(F("Capacity sensor value: "));
    Serial.println(capSense);
    if (capSense < CAP_SENSOR_THRESHHOLD) {
      Serial.println("Exiting manual advance.");
      break;
    }
    delay(100);
  }
}

void stepBySteps(int newPosition, boolean resetWhenDone) {
  long originalPosition = clockStepper.currentPosition();
  clockStepper.enableOutputs();
  clockStepper.moveTo(newPosition);

  while (clockStepper.isRunning()) {
    clockStepper.run();
    delay(DELAY);
  }
  clockStepper.disableOutputs();

  if (resetWhenDone) {
    clockStepper.setCurrentPosition(originalPosition);
  }
  
}
