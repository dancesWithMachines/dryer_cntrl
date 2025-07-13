#include "HWTimer.h"
#include "help.h"

#define NAME "Filament Dryer Automator"
#define VERSION "0.0.1"
#define DEBUG false

/* Available modes */
enum Mode {
  MODE_GET,
  MODE_SET,
  MODE_HELP,
  MODE_UNKNOWN,
};

/* Available params */
enum Param {
  PARAM_TEMP,
  PARAM_TIME,
  PARAM_STATE,
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
const uint8_t minTemp = 45; // Degrees Celsius
const uint8_t maxTemp = 65; // Degrees Celsius
const uint8_t maxTime = 24; // Hours

/* Global params */
static bool stateRunning = false;
static uint8_t tempToSet = 0;
static uint8_t timeToSet = 0;

/* Timer */
HWTimer timer;

/**
 * @brief Arduino setup() function. Runs one.
 */
void setup() {
  pinMode(pinA, INPUT);
  pinMode(pinB, INPUT);
  pinMode(pinSW, INPUT);

  Serial.begin(115200);
  delay(1000);

  printHelp();
  Serial.println(ack);
}

/**
 * @brief Arduino loop() function. Runs constantly.
 */
void loop() {
  static String input;
  static char ch;

  if (!Serial.available())
    return;

  ch = Serial.read();
  Serial.write(ch);

  switch (ch) {
    case '\r':
      Serial.println();
      if (Serial.peek() == '\n')
        Serial.read();
    case '\n':
      if (Serial.peek() == '\r')
        Serial.read();

      input.trim();
      debugPrintf("Received input: %s\r\n", input.c_str());
      inputHandler(input);

      input = "";
      return;
    case '\b':
      Serial.print(" \b");
      input.remove(input.length() - 1);
      return;
  }

  input += ch;
}


/**
 * @brief Print message on Serial only if DEBUG is enabled.
 *
 * @param fmt message.
 * @param ... additional parameters.
 */
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

/**
 * @brief Converts parameter string to enum value.
 *
 * @param paramStr  parameter string.
 * @return          enum value of the parameter.
 */
Param strToParam(const String& paramStr) {
  debugPrintf("param: %s\r\n", paramStr.c_str());
  if (paramStr.equalsIgnoreCase("temp"))
    return PARAM_TEMP;
  if (paramStr.equalsIgnoreCase("time"))
    return PARAM_TIME;
  if (paramStr.equalsIgnoreCase("state"))
    return PARAM_STATE;
  return PARAM_UNKNOWN;
}

/**
 * @brief Converts mode string to enum value.
 *
 * @param modeStr mode string.
 * @return        enum value of the mode.
 */
Mode strToMode(const String& modeStr) {
  debugPrintf("mode: %s\r\n", modeStr.c_str());
  if (modeStr.equalsIgnoreCase("get"))
    return MODE_GET;
  if (modeStr.equalsIgnoreCase("set"))
    return MODE_SET;
  if (modeStr.equalsIgnoreCase("help"))
    return MODE_HELP;
  return MODE_UNKNOWN;
}

/**
 * @brief Handles input from Serial.
 *
 * @param input input string.
 */
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
      if (input != "") {
        Serial.printf("%s:\"%s\" is not a valid mode.\r\n", ackErr.c_str(), input.c_str());
        return;
      }
      Serial.println(ack);
      break;
    case MODE_HELP:
      printHelp();
      Serial.println(ack);
      break;
  }
}

/**
 * @brief Handles operations in "get" mode.
 *
 * @param paramStr parameter string.
 */
void modeGetHandler(const String& paramStr) {
  switch (strToParam(paramStr)) {
    case PARAM_TEMP:
      Serial.printf("%s:%2.1f\r\n", ack.c_str(), analogReadTemp());
      break;
    case PARAM_TIME:
      Serial.printf("%s:%s\r\n", ack.c_str(), timer.getTimeLeft().c_str());
      break;
    case PARAM_STATE:
      Serial.printf("%s:%s\r\n", ack.c_str(), stateRunning ? "running" : "stopped");
      break;
    case PARAM_UNKNOWN:
      Serial.printf("%s:\"%s\" is not a valid param.\r\n", ackErr.c_str(), paramStr.c_str());
      break;
  }
}

/**
 * @brief Handles operations in "set" mode.
 *
 * @param paramStr  parameter string.
 * @param valueStr  parameter value.
 */
void modeSetHandler(const String& paramStr, const String& valueStr) {
  switch (strToParam(paramStr)) {
    case PARAM_TEMP:
      setTemp(valueStr);
      break;
    case PARAM_TIME:
      setTime(valueStr);
      break;
    case PARAM_STATE:
      setStatus(valueStr);
      break;
    case PARAM_UNKNOWN:
      Serial.printf("%s:\"%s\" is not a valid param.\r\n", ackErr.c_str(), paramStr.c_str());
      break;
  }
}

/**
 * @brief Set up target temperature and apply immediately if already running.
 *
 * @param tempStr temperature string, will be parsed to integer.
 */
void setTemp(const String& tempStr) {
  uint8_t temp = (uint8_t) tempStr.toInt();

  debugPrintf("Setting up the temperature.\r\n");

  if (temp < minTemp || temp > maxTemp) {
    Serial.printf("%s:Allowed values range from %d to %d.\r\n", ackErr.c_str(), minTemp, maxTemp);
    return;
  }

  tempToSet = temp;

  if (stateRunning) {
    debugPrintf("Changing temperature to: %d\r\n", tempToSet);
    dial(PARAM_TEMP, tempToSet);
    return;
  }

  debugPrintf("Temperature to set: %d\r\n", tempToSet);
  Serial.println(ack);
}

/**
 * @brief Set up target time and apply immediately if already running.
 *
 * @param timeStr time string, will be parsed to integer.
 */
void setTime(const String& timeStr) {
  uint8_t time = (uint8_t) timeStr.toInt();

  debugPrintf("Setting up the time.\r\n");

  if (time < 0 || time > maxTime) {
    Serial.printf("%s:Allowed values range from %d to %d.\r\n", ackErr.c_str(), 0, maxTime);
    return;
  }

  timeToSet = time;

  debugPrintf("Time to set: %dh\r\n", timeToSet);

  if (stateRunning) {
    debugPrintf("Changing time to: %dh\r\n", tempToSet);
    dial(PARAM_TIME, timeToSet);
  }

  Serial.println(ack);
}

/**
 * @brief Set the state. Start or stop the job.
 *
 * @param stateStr state string, either "start" or "stop".
 */
void setStatus(const String& stateStr) {
  if (stateStr.equalsIgnoreCase("start")) {
    if (!timeToSet) {
      Serial.printf("%s:Cannot start, time not set.\r\n", ackErr.c_str());
      return;
    }

    dial(PARAM_TEMP, tempToSet);
    dial(PARAM_TIME, timeToSet);

    timer.start(timeToSet, resetStatus);

    stateRunning = true;

    debugPrintf("Started!\r\n");
    Serial.println(ack);

    return;
  }
  if (stateStr.equalsIgnoreCase("stop")) {
    dial(PARAM_TIME, 0);

    timer.stop();

    stateRunning = false;

    debugPrintf("Stopped!\r\n");
    Serial.println(ack);

    return;
  }

  Serial.printf("%s:\"%s\" is not a valid state.\r\n", ackErr.c_str(), stateStr.c_str());
  return;
}

/**
 * @brief A callback function for timer, reset state do "stopped".
 */
void resetStatus() {
  stateRunning = false;
}

/**
 * @brief Sets up target values by emulating encoder movement. Dials in the settings.
 *
 * @param param parameter to set, either temperature or time.
 * @param value value to set to.
 */
void dial(Param param, uint8_t value) {
  uint8_t settingSteps;
  uint8_t minValue, maxValue;

  switch (param) {
    case PARAM_TEMP:
      settingSteps = 1;
      minValue = minTemp;
      maxValue = maxTemp;
      debugPrintf("Dialing in temperature.\r\n");
      break;
    case PARAM_TIME:
      settingSteps = 2;
      minValue = 0;
      maxValue = maxTime;
      debugPrintf("Dialing in time.\r\n");
      break;
    default:
      return;
  }

  debugPrintf("Value to dial in: %d\r\n", value);

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

/**
 * @brief Emulates encoder single step to the right.
 */
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

/**
 * @brief Emulates encoder single step to the left.
 */
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

/**
 * @brief Emulates encoder button press.
 */
void pressButton() {
  pinMode(pinSW, OUTPUT);
  digitalWrite(pinSW, HIGH);
  delay(50);

  pinMode(pinSW, INPUT);
}

/**
 * @brief Set the encoder pins direction.
 *
 * @param a encoder pin A.
 * @param b encoder pin B.
 */
void setPins(int a, int b) {
  digitalWrite(pinA, a);
  digitalWrite(pinB, b);
}
