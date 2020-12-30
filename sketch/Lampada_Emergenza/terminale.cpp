////////////////////////////////////////////////////////////////////////////////
//                           I N C L U D E S                                  //
////////////////////////////////////////////////////////////////////////////////

// System Library Include
#include <Arduino.h>
#include <HardwareSerial.h>


// User Library Include
#include "termargs.h"
#include "terminale.h"


////////////////////////////////////////////////////////////////////////////////
//              P R I V A T E   S T A T I C   F U N C T I O N S               //
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//                     P U B L I C   F U N C T I O N S                        //
////////////////////////////////////////////////////////////////////////////////
Terminale::Terminale(const TermCmd* comandi)
  : m_comandi(comandi)
{
}

////////////////////////////////////////////////////////////////////////////////
void Terminale::doWorkAndAnswer(char* cmd)
{
  Serial.println("--------------------");
  char* argv[8];
  int argc = argSegment(cmd, argv, 8);

  for (int i = 0; i < argc; ++i) {
    Serial.print(i);
    Serial.print(" -> '");
    Serial.print(argv[i]);
    Serial.println("'");
  }

  terrminale_help();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
void Terminale::terrminale_help()
{
  Serial.println("-----------------------------------------------------");
  Serial.println("List of available commands:");
  int i = 0;
  while (m_comandi[i].m_name) {
    //        if (is_command_accessible(state, list + i)) {
    Serial.print(m_comandi[i].m_name);
    int spc = 8 - strlen(m_comandi[i].m_name);
    while (spc > 0) {
      Serial.print(' ');
      --spc;
    }
    Serial.print(" : ");
    if (m_comandi[i].m_help) {
      Serial.print(m_comandi[i].m_help);
    }
    Serial.println("");
  }
  //    }
}
