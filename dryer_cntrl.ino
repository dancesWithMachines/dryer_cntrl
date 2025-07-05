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
  PARAM_UNKNOWN
};

const String ackErr = "ERR";
const String ack = "OK";

const int pinA = 2; // Encoder A
const int pinB = 3; // Encoder B
const int pinSW = 4; // Switch

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
    debugPrintf("Received input:%s\n", input.c_str());  // Corrected
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
      Serial.print("Invalid mode: ");
      Serial.println(input);
    case MODE_HELP:
      printHelp();
      break;
  }
}

void modeGetHandler(const String& paramStr) {
  switch (strToParam(paramStr)) {
    case PARAM_TEMP:
      Serial.print(ack);
      Serial.print(":");
      Serial.printf("%2.1f\n", analogReadTemp());
      break;
    case PARAM_TIME:
      printUnsupportedMessage(MODE_GET, paramStr);
      break;
    case PARAM_UNKNOWN:
      Serial.print("Invalid param: ");
      Serial.println(paramStr);
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
    case PARAM_UNKNOWN:
      Serial.print("Invalid param: ");
      Serial.println(paramStr);
      break;
  }
}

void setTemp(const String& tempStr) {
  const uint8_t minTemp = 45;
  const uint8_t maxTemp = 65;
  uint8_t temp = (uint8_t) tempStr.toInt();

  debugPrintf("Setting up the temperature\n");

  if (temp < minTemp || temp > maxTemp) {
    Serial.printf("%s: Allowed values range from %d to %d.", ackErr.c_str(), minTemp, maxTemp);
    return;
  }

  debugPrintf("Temperature to set: %d\n", temp);

  // Go to temp settings
  rotateLeft();
  delay(1);
  pressButton();
  delay(1);

  // Reset to minimal value
  for (int i = 0; i < maxTemp - minTemp; i++) {
    rotateRight();
    delay(1);
  }

  // Set new temperature
  for (int i = minTemp; i < temp; i++) {
    rotateLeft();
    delay(1);
  }

  // Confirm temp
  pressButton();

  Serial.println(ack);
}

void setTime(const String& hourStr) {
  const uint8_t minHour = 0;
  const uint8_t maxHour = 24;
  uint8_t hour = (uint8_t) hourStr.toInt();

  debugPrintf("Setting up the temperature\n");

  if (hour < minHour || hour > maxHour) {
    Serial.printf("%s: Allowed values range from %d to %d.", ackErr.c_str(), minHour, maxHour);
    return;
  }

  debugPrintf("Hours to set: %d\n", hour);

  // Go to temp settings
  rotateLeft();
  delay(1);
  rotateLeft();
  delay(1);
  pressButton();
  delay(1);

  // Reset to minimal value
  for (int i = 0; i < maxHour; i++) {
    rotateRight();
    delay(1);
  }

  // Set new temperature
  for (int i = minHour; i < hour; i++) {
    rotateLeft();
    delay(1);
  }

  // Confirm temp
  pressButton();

  Serial.println(ack);
}

void printUnsupportedMessage(Mode mode, const String& paramStr) {
  String modeStr;
  if (mode == MODE_GET)
    modeStr = "set";
  else
   modeStr = "get";
  
  Serial.print(ackErr);
  Serial.print(":\"");
  Serial.print(paramStr);
  Serial.print("\" is currently unsupported in \"");
  Serial.print(modeStr);
  Serial.println("\" mode.");
}

Param strToParam(const String& input) {
  debugPrintf("param:%s\n", input.c_str());
  if (input.equalsIgnoreCase("temp"))
    return PARAM_TEMP;
  if (input.equalsIgnoreCase("time"))
    return PARAM_TIME;
  return PARAM_UNKNOWN;
}

Mode strToMode(const String& input) {
  debugPrintf("mode:%s\n", input.c_str());
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
