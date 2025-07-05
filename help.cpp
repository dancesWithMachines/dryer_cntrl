#include <Arduino.h>

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
                      "@dancesWithMachines";

const char * syntax = "Syntax: [MODE] [PARAM] [VALUE](in \"set\" mode)";
                       
const char * modes = "Modes:\n"
                     "* get - retreive information,\n"
                     "* set - set parameters,\n"
                     "* help - print this message.";

const char * params = "Parameters:\n"
                      "* temp:\n"
                      "  * [get MODE] Get temperature from RP2040 internal sensor. Unit is degrees Celcius.\n"
                      "  * [set MODE] Set target temperature. Allowed values 45-65. Unit is degrees Celcius.\n"
                      "* time:\n"
                      "  * [get MODE] Get estimated time till finish. Counts time using RP2040 hardware timer.\n"
                      "  * [set MODE] Set target time. Allowed values 0-24. Unit is hours.\n"
                      "* status:\n"
                      "  * [get MODE] Check if job is currently running. Returns either \"running\" or \"stopped\".\n"
                      "  * [set MODE] \"start\" starts the job, \"stop\" stops the job.";

const char * flow = "Flow:\n"
                    "1. Set up temperature and time.\n"
                    "2. Set status to \"start\" to start the job.\n"
                    "Notes:\n"
                    "Once the job is running, the settings are applied live.\n"
                    "You can stop the job by setting status to \"stop\", this does not reset set up values.";

const char * responses = "Responses:\n"
                         "* Responses follow <status> or <status>:<value> format.\n"
                         "* Status is either \"OK\" on success or \"ERR\" on failure.\n"
                         "* If function provides any feedback, it will be printed after \":\".";

bool boot = true;

void printHelp() {
  if (boot) {
    Serial.printf("%s\n%s\n\n", logo, author);
    boot = false;
  }
  Serial.println(syntax);
  Serial.println(modes);
  Serial.println(params);
  Serial.println(flow);
  Serial.println(responses);
}
