// Enable debug prints to serial monitor
//#define MY_DEBUG

#define MY_GATEWAY_SERIAL

#include <MySensors.h>
#include <Bounce2.h>
//#include "PCF8574.h"
//#include "Adafruit_MCP23017.h"

#if defined(PCF8574_H) || defined(_Adafruit_MCP23017_H_)
  #define USE_EXPANDER
  #include <Wire.h>    // Required for I2C communication
  uint8_t expanderAddresses[] = {0x20};
  const int numberOfExpanders = sizeof(expanderAddresses);
  #if defined(PCF8574_H)
    PCF8574 expander[numberOfExpanders];
  #elif defined(_Adafruit_MCP23017_H_)
    Adafruit_MCP23017 expander[numberOfExpanders];
  #endif
  #define E(expanderNo, ExpanderPin) (((expanderNo+1)<<8) | (ExpanderPin))
#endif

// No Button Constant
#define NOB -1
#define MULTI_RELAY_VERSION 9
#define RELAY_STATE_STORAGE 1

const uint8_t RELAY_TRIGGER_LOW  = 0;
const uint8_t RELAY_TRIGGER_HIGH = 1;
const uint8_t RELAY_STARTUP_ON   = 2;
const uint8_t RELAY_STARTUP_OFF  = 4;
const uint8_t RELAY_STARTUP_MASK = RELAY_STARTUP_ON | RELAY_STARTUP_OFF;

enum ButtonType {
  MONO_STABLE = 0,
  BI_STABLE = 1,
  DING_DONG = 2 // HIGH state immediatly after push, LOW state after release
};

typedef struct {
  int sensorId;
  int relay;
  int button;
  uint8_t relayOptions;
  ButtonType buttonType;
  const char * relayDescription;
} RelayButton;

// CONFIGURE ONLY THIS ARRAY!
// Row params: sensor ID - sensor ID reported on MySensor Gateway
//             relay pin - Expander supported
//             button pin - <0 for virtual buttons (only available in MySensor Gateway); no support for Expander
//             relay options - [RELAY_TRIGGER_LOW|RELAY_TRIGGER_HIGH] {RELAY_STARTUP_ON|RELAY_STARTUP_OFF}
//             button type - [MONO_STABLE|BI_STABLE|DING_DONG]
//             relay description - reported on MySensor Gateway, can help identify device on initial configuration in Home Automation App, can be empty ("")
RelayButton myRelayButtons[] = {
  {0, 4, A0, RELAY_TRIGGER_LOW, MONO_STABLE, "Ł2 - kinkiet"},  // WŁ: Ł2
  {1, 19, A1, RELAY_TRIGGER_LOW, BI_STABLE, "Salon 2"},  // WŁ: Salon prawy
  {2, 18, A2, RELAY_TRIGGER_LOW, BI_STABLE, "Salon 1"},  // WŁ: Salon lewy
  {3, 14, A3, RELAY_TRIGGER_LOW | RELAY_STARTUP_OFF, BI_STABLE, "Halogen - Taras"},  // WŁ: Taras
  {4, 25, A4, RELAY_TRIGGER_LOW, BI_STABLE, "Kuchnia"},  // WŁ: Kuchnia lewy
  {5, 26, A5, RELAY_TRIGGER_LOW, BI_STABLE, "Kuchnia - Kinkiet"},  // WŁ: Kuchnia prawy
  {6, 31, A6, RELAY_TRIGGER_LOW, BI_STABLE, "Jadalnia 2"},  // WŁ: Hall I/Jadalnia prawy
  {17, 22, A7, RELAY_TRIGGER_LOW, BI_STABLE, "Ł1 - Kinkiet"},  // WŁ: Hall I/Ł1 prawy
  {8, 34, A8, RELAY_TRIGGER_LOW, MONO_STABLE, "Garaż"},  // WŁ: Kotłownia/Garaż
  {8, 34, A9, RELAY_TRIGGER_LOW, MONO_STABLE, "Garaż"},  // WŁ: Garaż
  {10, 3, A10, RELAY_TRIGGER_LOW | RELAY_STARTUP_ON, BI_STABLE, "Halogen - wejście"},  // WŁ: Drzwi wejściowe
  {12, 32, A12, RELAY_TRIGGER_LOW, BI_STABLE, "Hall 1"},  // WŁ: Hall I/Jadalnia lewy
  {12, 32, A13, RELAY_TRIGGER_LOW, BI_STABLE, "Hall 1"},  // WŁ: Hall I/Wiatrołap
  {14, 35, A14, RELAY_TRIGGER_LOW, BI_STABLE, "Wiatrołap"},  // WŁ: Wiatrołap/Hall I
  {15, 24, A15, RELAY_TRIGGER_LOW, MONO_STABLE, "Kotłownia"},  // WŁ: Kotłownia/Hall I
  {16, 27, 53, RELAY_TRIGGER_LOW, BI_STABLE, "Ł1 - Taśma LED"},  // WŁ: Hall I/Ł1 środek
  {17, 22, 52, RELAY_TRIGGER_LOW, MONO_STABLE, "Ł1 - Kinkiet"},  // WŁ: Ł1
  {18, 23, 51, RELAY_TRIGGER_LOW, BI_STABLE, "Ł1"},  // WŁ: Hall I/Ł1 lewy
  {19, 8, 50, RELAY_TRIGGER_LOW, BI_STABLE, "Klatka Schodowa"},  // WŁ: Hall I/Schody prawy
  {12, 32, 49, RELAY_TRIGGER_LOW, BI_STABLE, "Hall 1"},  // WŁ: Hall I/Schody lewy
  {21, 29, 48, RELAY_TRIGGER_LOW, BI_STABLE, "Gabinet"},  // WŁ: Gabinet
  {22, 9, 47, RELAY_TRIGGER_LOW, BI_STABLE, "Hall 2"},  // WŁ: Hall II/Schody prawy
  {19, 8, 46, RELAY_TRIGGER_LOW, BI_STABLE, "Klatka Schodowa"},  // WŁ: Hall II/Schody lewy
  {24, 11, 45, RELAY_TRIGGER_LOW, BI_STABLE, "Garderoba"},  // WŁ: Garderoba
  {26, 7, 43, RELAY_TRIGGER_LOW, BI_STABLE, "Pok. nad kuchnią 1"},  // WŁ: Pok. nad kuchnią 1
  {27, 10, 42, RELAY_TRIGGER_LOW, BI_STABLE, "Pok. nad salonem 2"},  // WŁ: Pok. nad salonem 2
  {29, 5, 40, RELAY_TRIGGER_LOW, BI_STABLE, "Ł2"},  // WŁ: Hall II/Ł2 lewy
  {30, 6, 39, RELAY_TRIGGER_LOW, BI_STABLE, "Ł2 - Taśma LED"},  // WŁ: Hall II/Ł2 prawy
  {22, 9, 38, RELAY_TRIGGER_LOW, BI_STABLE, "Hall 2"},  // WŁ: Hall II/Sypialnia
  {32, 12, 37, RELAY_TRIGGER_LOW, BI_STABLE, "Sypialnia 2"},  // WŁ: Sypialnia 2
  {34, 28, -1, RELAY_TRIGGER_LOW | RELAY_STARTUP_ON, MONO_STABLE, "Halogen - Garaż"},  // WŁ: Virtual Button 1
  {35, 30, -2, RELAY_TRIGGER_LOW | RELAY_STARTUP_OFF, MONO_STABLE, "Ł1 - Wentylator"},  // WŁ: Virtual Button 2
  {36, 15, -3, RELAY_TRIGGER_LOW | RELAY_STARTUP_OFF, MONO_STABLE, "Halogen - wschód"},  // WŁ: Virtual Button 3
  {40, 2, -7, RELAY_TRIGGER_LOW | RELAY_STARTUP_OFF, MONO_STABLE, "Ł2 - wentylator"},  // WŁ: Virtual Button 7
};

const int numberOfRelayButtons = sizeof(myRelayButtons) / sizeof(RelayButton);

typedef struct {
  int firstButton;
  int nextButton;
} RelayMultiButtons;

RelayMultiButtons relayMultiButtons[numberOfRelayButtons];
uint8_t myRelayState[numberOfRelayButtons];

// MySensors - Sending Data
// To send data you have to create a MyMessage container to hold the information.
MyMessage msgs[numberOfRelayButtons];

Bounce myButtonDebouncer[numberOfRelayButtons];

//Function Declaration
uint8_t loadRelayState(int relayNum, uint8_t forceEeprom = 0);
void saveRelayState(int relayNum, uint8_t state, uint8_t useEeprom);
void saveRelayState(int relayNum, uint8_t state);
void changeRelayState(int relayNum, uint8_t relayState);



// MySensors - This will execute before MySensors starts up
void before() {
  Serial.begin(115200);
  
  #ifdef USE_EXPANDER
    /* Start I2C bus and PCF8574 instance */
    for(int i = 0; i < numberOfExpanders; i++) {
      expander[i].begin(expanderAddresses[i]);
    }
  #endif
  
  // initialize multiple buttons list structure
  for (int i = 0; i < numberOfRelayButtons; i++) {
    relayMultiButtons[i].firstButton = -1;
    relayMultiButtons[i].nextButton = -1;
  }
  // find multiple buttons for the same relay (uni-directional list)
  for (int i = 0; i < numberOfRelayButtons-1; i++) {
    if (relayMultiButtons[i].firstButton == -1) {
      int prevRelayButton = i;
      for (int j = i+1; j < numberOfRelayButtons; j++) {
        if (myRelayButtons[i].relay == myRelayButtons[j].relay) {
          relayMultiButtons[prevRelayButton].firstButton = i;
          relayMultiButtons[prevRelayButton].nextButton = j;
          relayMultiButtons[j].firstButton = i;
          prevRelayButton = j;
        }
      }
    }
  }
  
  // if version has changed, reset state of all relays
  int versionChangeResetState = (MULTI_RELAY_VERSION == loadState(0) ) ? 0 : 1;
  
  for (int i = 0; i < numberOfRelayButtons; i++) {
    // if this relay has multiple buttons, load only first
    if (relayMultiButtons[i].firstButton == -1 || relayMultiButtons[i].firstButton == i) {
      // Then set relay pins in output mode
      #ifdef USE_EXPANDER
        if ( myRelayButtons[i].relay & 0xff00 ) {
          // EXPANDER
          int expanderNo = (myRelayButtons[i].relay >> 8) - 1;
          int expanderPin = myRelayButtons[i].relay & 0xff;
          expander[expanderNo].pinMode(expanderPin, OUTPUT);
        } else {
      #endif
          pinMode(myRelayButtons[i].relay, OUTPUT);
      #ifdef USE_EXPANDER
        }
      #endif
      
      uint8_t isTurnedOn = 0;
      
      if (myRelayButtons[i].relayOptions & RELAY_STARTUP_ON) {
        isTurnedOn = 1;
      } else if (myRelayButtons[i].relayOptions & RELAY_STARTUP_OFF) {
      } else {
        // Set relay to last known state (using eeprom storage)
        isTurnedOn = loadRelayState(i, 1); // 1 - true, 0 - false
        if (versionChangeResetState && isTurnedOn) {
          saveRelayState(i, 0, 1);
          isTurnedOn = 0;
        }
      }

      changeRelayState(i, isTurnedOn);
      myRelayState[i] = isTurnedOn;
    }
  }
  if (versionChangeResetState) {
    // version has changed, so store new version in eeporom
    saveState(0, MULTI_RELAY_VERSION);
  }
}

// executed AFTER mysensors has been initialised
void setup() {
  for(int i = 0; i < numberOfRelayButtons; i++) {
    if (myRelayButtons[i].button >= 0) {
      // No Expander support for buttons (de-bouncing)
      pinMode(myRelayButtons[i].button, INPUT_PULLUP); // HIGH state when button is not pushed
    }
  }
  // Setup locally attached sensors
  delay(5000);
  // Send state to MySensor Gateway
  for(int i = 0; i < numberOfRelayButtons; i++) {
    // if this relay has multiple buttons, send only first
    if (relayMultiButtons[i].firstButton == -1 || relayMultiButtons[i].firstButton == i) {
      msgs[i] = MyMessage(myRelayButtons[i].sensorId, V_STATUS);
      uint8_t relayState;
      if (myRelayButtons[i].relayOptions & RELAY_STARTUP_ON) {
        relayState = 1;
      } else if (myRelayButtons[i].relayOptions & RELAY_STARTUP_OFF) {
        relayState = 0;
      } else {
        relayState = loadRelayState(i);
      }
      send(msgs[i].set(relayState)); // send current state
    }
  }
  // Setup buttons
  for(int i = 0; i < numberOfRelayButtons; i++) {
    if (myRelayButtons[i].button >= 0) {
      // setup debouncer
      myButtonDebouncer[i] = Bounce();
      myButtonDebouncer[i].attach(myRelayButtons[i].button);
      myButtonDebouncer[i].interval(50);
    }
  }
}

void loop() {
  for(int i = 0; i < numberOfRelayButtons; i++) {
    if (myRelayButtons[i].button >= 0 && myButtonDebouncer[i].update()) {
      int buttonState = myButtonDebouncer[i].read();
      #ifdef MY_DEBUG
        Serial.print("# Button ");
        Serial.print(i);
        Serial.print(" changed to: ");
        Serial.println(buttonState);
      #endif
      
      int relayNum = (relayMultiButtons[i].firstButton == -1) ? i : relayMultiButtons[i].firstButton;
      
      if (myRelayButtons[i].buttonType == DING_DONG) {
        if (buttonState == LOW) { // button pressed
          changeRelayState(relayNum, 1);
          send(msgs[relayNum].set(1));
        } else { // button released
          changeRelayState(relayNum, 0);
          send(msgs[relayNum].set(0));
        }
      } else if (myRelayButtons[i].buttonType == BI_STABLE || buttonState == HIGH) {
        // If button type is BI_STABLE, any change will toggle relay state
        // For MONO_STABLE, button must be pushed and released (HIGH)
        uint8_t isTurnedOn = ! loadRelayState(relayNum); // 1 - true, 0 - false
        changeRelayState(relayNum, isTurnedOn);
        send(msgs[relayNum].set(isTurnedOn));
        saveRelayState(relayNum, isTurnedOn);
      }
    }
  }
}



// MySensors - Presentation
// Your sensor must first present itself to the controller.
// The presentation is a hint to allow controller prepare for the sensor data that eventually will come.
// Executed after "before()" and before "setup()" in: _begin (MySensorsCore.cpp) > gatewayTransportInit() > presentNode()
void presentation() {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Multi Relay", "1.2");
  
  // Register every relay as separate sensor
  for (int i = 0; i < numberOfRelayButtons; i++) {
    // if this relay has multiple buttons, register only first
    if (relayMultiButtons[i].firstButton == -1 || relayMultiButtons[i].firstButton == i) {
      // Register all sensors to gw (they will be created as child devices)
      // void present(uint8_t childSensorId, uint8_t sensorType, const char *description, bool ack);
      //   childSensorId - The unique child id you want to choose for the sensor connected to this Arduino. Range 0-254.
      //   sensorType - The sensor type you want to create.
      //   description An optional textual description of the attached sensor.
      //   ack - Set this to true if you want destination node to send ack back to this node. Default is not to request any ack.
      present(myRelayButtons[i].sensorId, S_BINARY, myRelayButtons[i].relayDescription);
    }
  }
}


// MySensors - Handling incoming messages
// Nodes that expects incoming data, such as an actuator or repeating nodes,
// must implement the receive() - function to handle the incoming messages.
// Do not sleep a node where you expect incoming data or you will lose messages.
void receive(const MyMessage &message) {
  // We only expect one type of message from controller. But we better check anyway.
  if (message.type == V_STATUS) {
    uint8_t isTurnedOn = message.getBool(); // 1 - true, 0 - false
    int relayNum = getRelayNum(message.sensor);
    if (relayNum == -1) return;
    changeRelayState(relayNum, isTurnedOn);
    // TODO: support for DING-DONG
    // Store state in eeprom if changed
    if (loadRelayState(relayNum) != isTurnedOn) {
      saveRelayState(relayNum, isTurnedOn);
    }
    send(msgs[relayNum].set(isTurnedOn)); // support for OPTIMISTIC=FALSE (Home Asistant)
    #ifdef MY_DEBUG
      // Write some debug info
      Serial.print("# Incoming change for sensor: " + relayNum);
      Serial.println(", New status: " + isTurnedOn);
    #endif
  }
}

uint8_t loadRelayState(int relayNum, uint8_t forceEeprom) {
  uint8_t relayState;
  if (forceEeprom) {
    relayState = loadState(RELAY_STATE_STORAGE + relayNum);
  } else {
    relayState = myRelayState[relayNum];
  }
  #ifdef MY_DEBUG
    Serial.print("# loadRelayState: ");
    Serial.print(relayNum);
    if (forceEeprom) {
      Serial.print("(byte ");
      Serial.print(RELAY_STATE_STORAGE + relayNum);
      Serial.print(")");
    }
    Serial.print(" = ");
    Serial.println(relayState);
  #endif
  return(relayState);
}

void saveRelayState(int relayNum, uint8_t state, uint8_t useEeprom) {
  
  int mainRelayNum = (relayMultiButtons[relayNum].firstButton == -1) ? relayNum : relayMultiButtons[relayNum].firstButton;
  
  myRelayState[mainRelayNum] = state;
  if (useEeprom && (relayNum == mainRelayNum)) {
    saveState(RELAY_STATE_STORAGE + mainRelayNum, state);
  }
  
  int nextButton = mainRelayNum;
  // update all buttons
  while ((nextButton = relayMultiButtons[nextButton].nextButton) != -1) {
    myRelayState[nextButton] = state;
  };
}

void saveRelayState(int relayNum, uint8_t state) {
  uint8_t useEeprom = ((myRelayButtons[relayNum].relayOptions & RELAY_STARTUP_MASK) == 0);
  saveRelayState(relayNum, state, useEeprom);
}

void changeRelayState(int relayNum, uint8_t relayState) {
  
  uint8_t relayTrigger = myRelayButtons[relayNum].relayOptions & RELAY_TRIGGER_HIGH;
  uint8_t digitalOutState = relayState ? relayTrigger : ! relayTrigger;
  
  #ifdef USE_EXPANDER
    if ( myRelayButtons[relayNum].relay & 0xff00 ) {
      int expanderNo = (myRelayButtons[relayNum].relay >> 8) - 1;
      int expanderPin = myRelayButtons[relayNum].relay & 0xff;
      expander[expanderNo].digitalWrite(expanderPin, digitalOutState);
    } else {
  #endif
    digitalWrite(myRelayButtons[relayNum].relay, digitalOutState);
  #ifdef USE_EXPANDER
    }
  #endif
}

int getRelayNum(int sensorId) {
  for (int i = 0; i < numberOfRelayButtons; i++) {
    if (myRelayButtons[i].sensorId == sensorId) return(i);
  }
  return(-1);
}
