#include "HWTimer.h"

#define NAME "Filament Dryer Automator"
#define VERSION "0.0.0"
#define DEBUG true

enum Mode {
  MODE_GET,
  MODE_SET,
  MODE_HELP,
  MODE_UNKNOWN,
};

enum Param {
  PARAM_TEMP,
  PARAM_TIME,
  PARAM_STATUS,
  PARAM_UNKNOWN
};

/* Status messages */
const String ackErr = "ERR";
const String ack = "OK";

/* GPIO definitions */
const uint8_t pinA = 2; // Encoder A
const uint8_t pinB = 3; // Encoder B
const uint8_t pinSW = 4; // Switch

/* Max values */
const uint8_t minTemp = 45;
const uint8_t maxTemp = 65;
const uint8_t maxHours = 24;

/* Global params */
static bool statusRunning = false;
static uint8_t tempToSet = 0;
static uint8_t hoursToSet = 0;

/* Timer */
HWTimer timer;

void setup() {
  pinMode(pinA, INPUT);
  pinMode(pinB, INPUT);
  pinMode(pinSW, INPUT);

  Serial.begin(115200);
  delay(1000);

  printHelp();
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    debugPrintf("Received input: %s\n", input.c_str());
    inputHandler(input);
  }
}

void debugPrintf(const char* fmt, ...) {
  const uint8_t buff_size = 255;

  if (DEBUG) {
    char buffer[buff_size];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    Serial.print("<DEB>");
    Serial.print(buffer);
  }
}

void inputHandler(const String& input) {
  int sepIndex = 0;
  String mode, param, value;
  
  sepIndex = input.indexOf(' ');
  mode = input.substring(0, sepIndex);
  param = input.substring(++sepIndex);
  sepIndex = param.indexOf(' ');
  value = param.substring(sepIndex + 1);
  param = param.substring(0, sepIndex);

  switch (strToMode(mode)) {
    case MODE_GET:
      modeGetHandler(param);
      break;
    case MODE_SET:
      modeSetHandler(param, value);
      break;
    case MODE_UNKNOWN:
      Serial.printf("%s:\"%s\" is not a valid mode.\n", ackErr.c_str(), input.c_str());
    case MODE_HELP:
      printHelp();
      break;
  }
}

void modeGetHandler(const String& paramStr) {
  switch (strToParam(paramStr)) {
    case PARAM_TEMP:
      Serial.printf("%s:%2.1f\n", ack.c_str(), analogReadTemp());
      break;
    case PARAM_TIME:
      Serial.printf("%s:%s\n", ack.c_str(), timer.getTimeLeft().c_str());
      break;
    case PARAM_STATUS:
      Serial.printf("%s:%s\n", ack.c_str(), statusRunning ? "running" : "stopped");
      break;
    case PARAM_UNKNOWN:
      Serial.printf("%s:\"%s\" is not a valid param.\n", ackErr.c_str(), paramStr.c_str());
      break;
  }
}

void modeSetHandler(const String& paramStr, const String& value) {
  switch (strToParam(paramStr)) {
    case PARAM_TEMP:
      setTemp(value);
      break;
    case PARAM_TIME:
      setTime(value);
      break;
    case PARAM_STATUS:
      setStatus(value);
      break;
    case PARAM_UNKNOWN:
      Serial.printf("%s:\"%s\" is not a valid param.\n", ackErr.c_str(), paramStr.c_str());
      break;
  }
}

void setTemp(const String& tempStr) {
  uint8_t temp = (uint8_t) tempStr.toInt();

  debugPrintf("Setting up the temperature.\n");

  if (temp < minTemp || temp > maxTemp) {
    Serial.printf("%s:Allowed values range from %d to %d.\n", ackErr.c_str(), minTemp, maxTemp);
    return;
  }

  tempToSet = temp;

  if (statusRunning) {
    debugPrintf("Changing temperature to: %d\n", tempToSet);
    dial(PARAM_TEMP, tempToSet);
    return;
  }

  debugPrintf("Temperature to set: %d\n", tempToSet);
  Serial.println(ack);
}

void setTime(const String& hourStr) {
  uint8_t hour = (uint8_t) hourStr.toInt();

  debugPrintf("Setting up the time.\n");

  if (hour < 0 || hour > maxHours) {
    Serial.printf("%s:Allowed values range from %d to %d.\n", ackErr.c_str(), 0, maxHours);
    return;
  }

  hoursToSet = hour;

  debugPrintf("Hours to set: %d\n", hoursToSet);

  if (statusRunning) {
    debugPrintf("Changing time to: %d\n", tempToSet);
    dial(PARAM_TIME, hoursToSet);
  }

  Serial.println(ack);
}

void setStatus(const String& statusStr) {
  if (statusStr.equalsIgnoreCase("start")) {
    if (!hoursToSet) {
      Serial.printf("%s:Cannot start, time not set.\n", ackErr.c_str());
      return;
    }

    dial(PARAM_TEMP, tempToSet);
    dial(PARAM_TIME, hoursToSet);

    timer.start(hoursToSet, resetStatus);

    statusRunning = true;
    
    debugPrintf("Started!\n");
    Serial.println(ack);

    return;
  }
  if (statusStr.equalsIgnoreCase("stop")) {
    dial(PARAM_TIME, 0);

    timer.stop();

    statusRunning = false;
    
    debugPrintf("Stopped!\n");
    Serial.println(ack);

    return;
  }
  
  Serial.printf("%s:\"%s\" is not a valid status.\n", ackErr.c_str(), statusStr.c_str());
  return;
}

void resetStatus() {
  statusRunning = false;
}

void dial(Param param, uint8_t value) {
  uint8_t settingSteps;
  uint8_t minValue, maxValue;

  switch (param) {
    case PARAM_TEMP:
      settingSteps = 1;
      minValue = minTemp;
      maxValue = maxTemp;
      debugPrintf("Dialing in temperature.\n");
      break;
    case PARAM_TIME:
      settingSteps = 2;
      minValue = 0;
      maxValue = maxHours;
      debugPrintf("Dialing in time.\n");
      break;
    default:
      return;
  }

  debugPrintf("Value to dial in: %d\n", value);

  // Select which param to set
  for (int i = 0; i < settingSteps; i++) {
    rotateLeft();
    delay(1);
  }
  
  // Confirm
  pressButton();
  delay(1);

  // Reset to minimal value
  for (int i = 0; i < maxValue - minValue; i++) {
    rotateRight();
    delay(1);
  }

  // Dial new value
  for (int i = minValue; i < value; i++) {
    rotateLeft();
    delay(1);
  }

  // Confirm settings
  pressButton();
  delay(1);
}

Param strToParam(const String& input) {
  debugPrintf("param: %s\n", input.c_str());
  if (input.equalsIgnoreCase("temp"))
    return PARAM_TEMP;
  if (input.equalsIgnoreCase("time"))
    return PARAM_TIME;
  if (input.equalsIgnoreCase("status"))
    return PARAM_STATUS;
  return PARAM_UNKNOWN;
}

Mode strToMode(const String& input) {
  debugPrintf("mode: %s\n", input.c_str());
  if (input.equalsIgnoreCase("get"))
    return MODE_GET;
  if (input.equalsIgnoreCase("set"))
    return MODE_SET;
  if (input.equalsIgnoreCase("help"))
    return MODE_HELP;
  return MODE_UNKNOWN;
}

void rotateRight() {
  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);
  delay(1);

  setPins(LOW, LOW);
  delayMicroseconds(500);
  setPins(HIGH, LOW);
  delayMicroseconds(500);
  setPins(HIGH, HIGH);
  delay(1);

  pinMode(pinA, INPUT);
  pinMode(pinB, INPUT);
}

void rotateLeft() {
  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);
  delay(1);

  setPins(LOW, LOW);
  delayMicroseconds(500);
  setPins(LOW, HIGH);
  delayMicroseconds(500);
  setPins(HIGH, HIGH);
  delay(1);

  pinMode(pinA, INPUT);
  pinMode(pinB, INPUT);
}

void setPins(int a, int b) {
  digitalWrite(pinA, a);
  digitalWrite(pinB, b);
}

void pressButton() {
  pinMode(pinSW, OUTPUT);
  digitalWrite(pinSW, HIGH);
  delay(50);

  pinMode(pinSW, INPUT);
}

void printHelp() {
  
  const char * logo = "             __--^^|\n"
                      "       __--^^      |\n"
                      " __--^^      __--^^ \n"
                      "|           | .--^^|\n"
                      "|__--^^|    | |    |\n"
                      ".--^^| |    | |    |\n"
                      "|    | |    | |    |\n"
                      "|    | |    | |    |\n"
                      "|    | |    | |    |\n"
                      "|    | |    | |__--^\n"
                      "|    | |__--^\n"
                      "|__--^";

  const char * author = "Mateusz Kusiak (Timax)\n"
                        " @dancesWithMachines";

  static bool boot = true;

  if (boot) {
    Serial.printf("%s %s\n\n%s\n\n%s\n", NAME, VERSION, logo, author);
    boot = false;
  }


}
