#include <Arduino.h>

const char * logo = "             __--^^|\r\n"
                    "       __--^^      |\r\n"
                    " __--^^      __--^^ \r\n"
                    "|           | .--^^|\r\n"
                    "|__--^^|    | |    |\r\n"
                    ".--^^| |    | |    |\r\n"
                    "|    | |    | |    |\r\n"
                    "|    | |    | |    |\r\n"
                    "|    | |    | |    |\r\n"
                    "|    | |    | |__--^\r\n"
                    "|    | |__--^\r\n"
                    "|__--^";

const char * author = "Mateusz Kusiak (Timax)\r\n"
                      "@dancesWithMachines";

const char * syntax = "Syntax: [MODE] [PARAM] [VALUE](in \"set\" mode)";

const char * modes = "Modes:\r\n"
                     "* get - retreive information,\r\n"
                     "* set - set parameters,\r\n"
                     "* help - print this message.";

const char * params = "Parameters:\r\n"
                      "* temp:\r\n"
                      "  * [get MODE] Get temperature from RP2040 internal sensor. Unit is degrees Celcius.\r\n"
                      "  * [set MODE] Set target temperature. Allowed values 45-65. Unit is degrees Celcius.\r\n"
                      "* time:\r\n"
                      "  * [get MODE] Get estimated time till finish. Counts time using RP2040 hardware timer.\r\n"
                      "  * [set MODE] Set target time. Allowed values 0-24. Unit is hours.\r\n"
                      "* status:\r\n"
                      "  * [get MODE] Check if job is currently running. Returns either \"running\" or \"stopped\".\r\n"
                      "  * [set MODE] \"start\" starts the job, \"stop\" stops the job.";

const char * flow = "Flow:\r\n"
                    "1. Set up temperature and time.\r\n"
                    "2. Set status to \"start\" to start the job.\r\n"
                    "Notes:\r\n"
                    "Once the job is running, the settings are applied live.\r\n"
                    "You can stop the job by setting status to \"stop\", this does not reset set up values.";

const char * responses = "Responses:\r\n"
                         "* Responses follow <status> or <status>:<value> format.\r\n"
                         "* Status is either \"OK\" on success or \"ERR\" on failure.\r\n"
                         "* If function provides any feedback, it will be printed after \":\".";

bool boot = true;

void printHelp() {
  if (boot) {
    Serial.printf("%s\r\n%s\r\n\r\n", logo, author);
    boot = false;
  }
  Serial.println(syntax);
  Serial.println(modes);
  Serial.println(params);
  Serial.println(flow);
  Serial.println(responses);
}
