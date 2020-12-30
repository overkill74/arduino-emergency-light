#include <LowPower.h>
#include <EEPROM.h>
#include "termargs.h"
#include "terminale.h"

//---------------------------------------------------
// INCLUDES
//---------------------------------------------------

//---------------------------------------------------
// DEFINES
//---------------------------------------------------
#define PIN_VBATT       A0

#define PIN_PRESENZA    2
#define PIN_LED_BIANCO  3
#define PIN_220_SENSE   4
#define PIN_LED_VERDE   5

//#define V_BATT_RATIO    1
////#define V_BATT_RATIO    2.1168
//#define COUNT_TO_V

//---------------------------------------------------
// Forward declatartions
//---------------------------------------------------
class Led;
class Comandi;
class SysCfg;

//---------------------------------------------------
// GLOBALS
//---------------------------------------------------
Led* g_led_bianco = 0;
Led* g_led_verde = 0;
SysCfg* g_sys_cfg = 0;
Comandi* g_cmd = 0;
uint32_t m_timer = 0;
uint32_t m_tout_led = 0;
bool m_man_presenza = false;
bool m_man_emergenza = false;

//---------------------------------------------------
// CLASSI
//---------------------------------------------------
///
/// \brief configurazione
///
class SysCfg
{
  public:
    SysCfg()
      : m_magic(0xAA11CC88)
      , m_led_manual_level(0)
    {
      loadEEprom();
    }

    void printConfig() {
      Serial.print("m_magic              : "); Serial.println(m_magic             );
      Serial.print("m_led_manual_level   : "); Serial.println(m_led_manual_level  );
      Serial.print("m_en_pres            : "); Serial.println(m_en_pres           );
      Serial.print("m_green_level        : "); Serial.println(m_green_level       );
      Serial.print("m_white_pres_level   : "); Serial.println(m_white_pres_level  );
      Serial.print("m_white_no_pwr_level : "); Serial.println(m_white_no_pwr_level);
      Serial.print("m_t_on               : "); Serial.println(m_t_on              );
      Serial.print("m_ramp_spd           : "); Serial.println(m_ramp_spd          );
    }

    void loadDefault() {
      m_en_pres = true;
      m_green_level = 255;
      m_white_pres_level = 255;
      m_white_no_pwr_level = 80;
      m_t_on = 10;
      m_ramp_spd = 6;
    }

    void loadEEprom() {
      uint32_t ver;
      ee_read(0, &ver, sizeof(uint32_t));
      if (ver != m_magic) {
        Serial.println("*** Parametri di Default ***");
        Serial.print("m_magic : "); Serial.println(m_magic             );
        Serial.print("ver     : "); Serial.println(ver             );
        loadDefault();
        return;
      }
      int add = 4;
      m_en_pres            = EEPROM.read(add++);
      m_green_level        = EEPROM.read(add++);
      m_white_pres_level   = EEPROM.read(add++);
      m_white_no_pwr_level = EEPROM.read(add++);
      m_t_on               = EEPROM.read(add++);
      m_ramp_spd           = EEPROM.read(add++);
    }

    void saveEEprom() {
      ee_write(0, &m_magic, sizeof(m_magic));
      int add = 4;
      EEPROM.update(add++, m_en_pres           );
      EEPROM.update(add++, m_green_level       );
      EEPROM.update(add++, m_white_pres_level  );
      EEPROM.update(add++, m_white_no_pwr_level);
      EEPROM.update(add++, m_t_on              );
      EEPROM.update(add++, m_ramp_spd          );
    }

  private:
    ///
    /// \brief Read from EEPROM
    ///
    bool ee_read(int add, void* dest, size_t n) {
      uint8_t* pd = (uint8_t*)dest;
      for (size_t x = 0; x < n; ++x) {
        *pd++ = EEPROM.read(add++);
      }
    }
    ///
    /// \brief Read from EEPROM
    ///
    bool ee_write(int add, const void* src, size_t n) {
      uint8_t* pd = (uint8_t*)src;
      for (size_t x = 0; x < n; ++x) {
        EEPROM.update(add++, *pd++);
      }
    }

  private:
    uint32_t m_magic;               //! Magic
  public:
    uint8_t  m_led_manual_level;    //! Livello impostato manualmente
    bool     m_en_pres;             //! Abilita sensore presenza
    uint8_t  m_green_level;         //! Livello LED verde
    uint8_t  m_white_pres_level;    //! Livello LED Bianco acceso per presenza
    uint8_t  m_white_no_pwr_level;  //! Livello LED Bianco acceso per assenza alimentazione
    uint8_t  m_t_on;                //! Tempo accensione
    uint8_t  m_ramp_spd;            //! Ramp Speed
};


///
/// \brief Timer one shot
///
class OneShotTmr
{
  public:
    OneShotTmr(uint32_t durata)
      : m_run(false)
    {
      reload(durata);
    }

    bool isExpired() {
      if (!m_run) {
        return false;
      }
      return ((m_tout - millis()) < m_durata) ? false : true;
    }

    void reload(uint32_t durata) {
      m_durata = durata;
      m_tout = millis() + m_durata;
    }

    bool isRunning() {
      return m_run;
    }
    void startTimer() {
      reload(m_durata);
      m_run = true;
    }
    void stopTimer()  {
      m_run = false;
    }

  private:
    uint32_t  m_durata;
    uint32_t  m_tout;   //! Timeout
    bool      m_run;
};

///
/// \brief Classe LED
///
class Led
{
  public:
    Led(int pin, const char* name)
      : m_pin(pin)
      , m_name(name)
      , m_luma(0)
      , m_luma_target(0)
        //, m_tmr(new OneShotTmr(3))
    {
      pinMode(m_pin, OUTPUT);
      setLuminosity(0);
    }

    ///
    /// \brief Imposta la luminosita'
    ///
    void setLuminosity(uint8_t luma) {
      m_luma_target = luma;
      //      m_tmr->startTimer();
    }

    ///
    /// \brief Loop per impostare la luninosita'
    ///
    bool doWork()
    {
      // if (m_tmr->isExpired()) {
      if (m_luma_target > m_luma) {
        ++m_luma;
      }
      else if (m_luma_target < m_luma) {
        --m_luma;
      }
      analogWrite(m_pin, m_luma);
      // }

      if (m_luma_target == m_luma) {
        //m_tmr->startTimer();
        return false;
      }

      //m_tmr->startTimer();
      return true;
    }

  private:
    int         m_pin;            //! Il Pin utilizzato
    const char* m_name;           //! Name of LED
    uint8_t     m_luma;           //! Luminosita'
    uint8_t     m_luma_target;    //! Obiettivo Luminosita'
    //OneShotTmr* m_tmr;            //! Timer
};

////////////////////////////////////////////////////////////////////////////////
static int luce_on(int argc, char** argv)
{
  int lev = g_sys_cfg->m_white_pres_level;
  if (argc > 1) {
    lev = atoi(argv[1]);
    if (lev < 1)  lev = 1;
    if (lev > 255) lev = 255;
  }

  g_sys_cfg->m_led_manual_level = lev;
  Serial.print("Luce ON: ");
  Serial.println(g_sys_cfg->m_led_manual_level);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
static int luce_off(int argc, char** argv)
{
  g_sys_cfg->m_led_manual_level = 0;
  Serial.println("Luce OFF");
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
static int lev_green(int argc, char** argv)
{
  if (argc > 1) {
    int val = atoi(argv[1]);
    if (val < 1)  val = 1;
    if (val > 255) val = 255;
    g_sys_cfg->m_green_level = val;
    g_sys_cfg->saveEEprom();
  }

  Serial.print("Led Verde: ");
  Serial.println(g_sys_cfg->m_green_level);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
static int lev_wh_pre(int argc, char** argv)
{
  if (argc > 1) {
    int val = atoi(argv[1]);
    if (val < 1)  val = 1;
    if (val > 255) val = 255;
    g_sys_cfg->m_white_pres_level = val;
    g_sys_cfg->saveEEprom();
  }

  Serial.print("Luma Presenza: ");
  Serial.println(g_sys_cfg->m_white_pres_level);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
static int lev_wh_emg(int argc, char** argv)
{
  if (argc > 1) {
    int val = atoi(argv[1]);
    if (val < 1)  val = 1;
    if (val > 255) val = 255;
    g_sys_cfg->m_white_no_pwr_level = val;
    g_sys_cfg->saveEEprom();
  }

  Serial.print("Luma Emergenza: ");
  Serial.println(g_sys_cfg->m_white_no_pwr_level);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
static int ramp_speed(int argc, char** argv)
{
  if (argc > 1) {
    int val = atoi(argv[1]);
    if (val < 1)  val = 1;
    if (val > 25) val = 25;
    g_sys_cfg->m_ramp_spd = val;
    g_sys_cfg->saveEEprom();
  }

  Serial.print("Ramp Speed: ");
  Serial.println(g_sys_cfg->m_ramp_spd);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
static int t_on_pres(int argc, char** argv)
{
  if (argc > 1) {
    int val = atoi(argv[1]);
    if (val < 3)  val = 3;
    if (val > 120) val = 120;
    g_sys_cfg->m_t_on = val;
    g_sys_cfg->saveEEprom();
  }

  Serial.print("Secondi Accensione: ");
  Serial.println(g_sys_cfg->m_t_on);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
static int en_pres(int argc, char** argv)
{
  if (argc > 1) {
    g_sys_cfg->m_en_pres = atoi(argv[1]) ? true : false;;
    g_sys_cfg->saveEEprom();
  }

  Serial.print("Sensore Pesenza En: ");
  Serial.println(g_sys_cfg->m_en_pres);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
static int presenza(int argc, char** argv)
{
  m_man_presenza = true;
  Serial.println("Manuale Presenza");
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
static int emergenza(int argc, char** argv)
{
  if (argc > 1) {
    m_man_emergenza = atoi(argv[1]) ? true : false;;
  }
  else {
    m_man_emergenza = true;
  }

  Serial.print("Manuale Emergenza: ");
  Serial.println(m_man_emergenza);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
static int prn_cfg(int argc, char** argv)
{
  g_sys_cfg->printConfig();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
static int load_def(int argc, char** argv)
{
  g_sys_cfg->loadDefault();
  Serial.println("Default Config!");
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
static int load_cfg(int argc, char** argv)
{
  g_sys_cfg->loadEEprom();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
void stampa_help(const TermCmd* cmd)
{
  Serial.println("-----------------------------------------------------");
  Serial.println("List of available commands:");
  int i = 0;
  while (cmd[i].m_name) {
    //        if (is_command_accessible(state, list + i)) {
    Serial.print(cmd[i].m_name);
    int spc = 8 - strlen(cmd[i].m_name);
    while (spc > 0) {
      Serial.print(' ');
      --spc;
    }
    Serial.print(" : ");
    if (cmd[i].m_help) {
      Serial.print(cmd[i].m_help);
    }
    Serial.println("");
    ++i;
  }
  //    }
}

static const TermCmd m_term_cmd[] =
{
  // <PAR obbligatorio>
  // [PAR opzionale]

  //-------------------------------------------------------------------------
  { "ON",    luce_on,    0, "[lev] Accende la luce"        },
  { "OFF",   luce_off,   0, "Spegne la luce"               },
  { "GREEN", lev_green,  0, "[lev] Luma LED Verde"         },
  { "WPRES", lev_wh_pre, 0, "[lev] Luma Per Peesenza"      },
  { "WEMG",  lev_wh_emg, 0, "[lev] Luma Emergenza"         },
  { "RAMP",  ramp_speed, 0, "[x] Velocita' rampa"          },
  { "TPRES", t_on_pres,  0, "[x] Durata accensione"        },
  { "ENP",   en_pres,    0, "[1/0] Abilita sensore presenza" },
  { "PRES",  presenza,   0, "Simula Impulso Presenza"      },
  { "EMG",   emergenza,  0, "[1/0] Simula Emergenza"       },
  { "LOAD",  load_cfg,   0, "Ricarica la con Configurazione" },
  { "CFG",   prn_cfg,    0, "Stampa Configurazione"        },
  { "LOADEF",load_def,   0, "COnfigurazione di Default"    },

  { 0, 0, 0, 0 }
};


///
/// \brief Classe Comandi
///
class Comandi
{
  private:

  public:
    Comandi()
    {
      clearRx();
    }

    bool clearRx()
    {
      while (Serial.available()) {
        Serial.read();
      }
      m_cnt = 0;
      m_overflow = false;
      m_rx_complete = false;
    }

    bool rxComando()
    {
      while (Serial.available()) {
        char rxc = Serial.read();
        if (m_cnt < sizeof(m_buffer) - 1) {
          if (rxc >= ' ') {
            m_buffer[m_cnt++] = rxc;
          }
        }
        else {
          m_overflow = true;
        }
        if (rxc == '\n' || rxc == '\r') {
          m_rx_complete = true;
          m_buffer[m_cnt++] = '\0';
        }
      }
      return m_rx_complete;
    }

    bool isError() {
      return m_overflow;
    }

    char* const getComando()
    {
      if (m_rx_complete) {
        return m_buffer;
      }
      return 0;
    }

    ///
    /// \brief elabora la stringa ricevuta ed esegue il comando
    void doWorkAndAnswer()
    {
      if (!rxComando()) {
        return;
      }

      // Parse stringa
      char* argv[8];
      int argc = argSegment(m_buffer, argv, 8);

      // Comando in MAIUSCOLO
      char* cmd = argv[0];
      int x = 0;
      while (cmd[x]) {
        if (cmd[x] >= 'a' && cmd[x] <= 'z') {
          cmd[x] -= ('a' - 'A');
        }
        ++x;
      }

      // Cerca il comando
      x = 0;
      int cmd_idx = -1;
      while (m_term_cmd[x].m_name) {
        if (!strcmp(cmd, m_term_cmd[x].m_name)) {
          // Trovato!
          cmd_idx = x;
          break;
        }
        ++x;
      }

      if (cmd_idx >= 0) {
        // Esegue il comando
        if (m_term_cmd[cmd_idx].m_func) {
          int err = m_term_cmd[cmd_idx].m_func(argc, argv);
          if (!err) {
            Serial.println("*** OK");
          }
          else {
            Serial.print("*** ERR:");
            Serial.println(err);
          }
        }
        else {
          Serial.println("To be done...");
        }
      }
      else {
        stampa_help(m_term_cmd);
      }

      clearRx();
    }

  private:
    char  m_buffer[200];  //! Buffer RX
    int   m_cnt;          //! Contatore caratteri ricevuti
    bool  m_overflow;     //! Comando in overflow
    bool  m_rx_complete;  //! Comando completo
};

//---------------------------------------------------
// Firmware
//---------------------------------------------------
static void wait_led_rampa()
{
  bool ok = false;
  do {
    ok = g_led_bianco->doWork() | g_led_verde->doWork();
    delay(g_sys_cfg->m_ramp_spd);
  } while (ok);
}

static void heartBeat()
{
  g_led_bianco->setLuminosity(g_sys_cfg->m_white_no_pwr_level);
  g_led_verde->setLuminosity(g_sys_cfg->m_white_no_pwr_level);
  wait_led_rampa();

  delay(1000);

  g_led_bianco->setLuminosity(0);
  g_led_verde->setLuminosity(0);
  wait_led_rampa();
}

//---------------------------------------------------
void setup()
{
  Serial.begin(115200);

  Serial.println("Lampada di emergenza!");

  // INGRESSI
  pinMode(PIN_220_SENSE, INPUT);
  pinMode(PIN_PRESENZA,  INPUT);

  // Abilita Pull up
  digitalWrite(PIN_220_SENSE, HIGH);
  digitalWrite(PIN_PRESENZA,  HIGH);

  g_sys_cfg = new SysCfg();

  // LED
  g_led_bianco = new Led(PIN_LED_BIANCO, "BIA");
  g_led_verde  = new Led(PIN_LED_VERDE, "VER");

  heartBeat();

  // Comandi
  g_cmd = new Comandi;
}

//---------------------------------------------------
void loop()
{
  OneShotTmr one_second(1000);
  one_second.startTimer();

  bool ext_pwr = digitalRead(PIN_220_SENSE) ? false : true;
  bool pres = false;
  bool delay_off = false;

  if (m_man_emergenza) {
    // Forza situazione di emergenza
    ext_pwr = false;
  }

  if (g_sys_cfg->m_en_pres) {
    // Sensore di presenza abiltiato
    if (m_man_presenza) {
      // Forza la presenza
      pres = true;
    }
    else {
      // Legge l'ingresso
      pres = digitalRead(PIN_PRESENZA)  ? true : false;
    }
  }
  // Reset forzatura
  m_man_presenza = false;

  int luma_verde = 0;
  int luma_bianco = 0;

  if (ext_pwr) {
    // Alimentazione esterna
    luma_verde = g_sys_cfg->m_green_level;

    if (pres) {
      m_tout_led = m_timer + g_sys_cfg->m_t_on;
    }
  }
  else {
    m_tout_led = 0;
    luma_verde = 0;
    luma_bianco = g_sys_cfg->m_white_no_pwr_level;
    g_sys_cfg->m_led_manual_level = 0;
  }

  if (m_tout_led && !g_sys_cfg->m_led_manual_level) {
    Serial.print("Tout     : "); Serial.println(m_tout_led - m_timer);
    if (m_timer >= m_tout_led) {
      m_tout_led = 0;
      delay_off = true;
    }
    else {
      luma_bianco = g_sys_cfg->m_white_pres_level;
    }
  }

  if (!luma_bianco && g_sys_cfg->m_led_manual_level) {
    luma_bianco = g_sys_cfg->m_led_manual_level;
  }

  g_led_bianco->setLuminosity(luma_bianco);
  g_led_verde->setLuminosity(luma_verde);
  wait_led_rampa();

  //  Serial.println("--------------------");
  //  Serial.print("ExtPwr   : "); Serial.println(ext_pwr);
  //  Serial.print("Presenza : "); Serial.println(pres);
  //  Serial.print("Luma B   : "); Serial.println(luma_bianco);
  //  Serial.print("Luma V   : "); Serial.println(luma_verde);

  // Timer
  while (!one_second.isExpired()) {
    g_cmd->doWorkAndAnswer();
  }
  ++m_timer;

  if (delay_off) {
    delay(2000);
  }

  //LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);

  //  float vbatt = (float)analogRead(PIN_VBATT) / V_BATT_RATIO;
  //  Serial.print("VBatt = ");
  //  Serial.print(vbatt);
  //
  //  //Serial.print((int)((vbatt+0.5)*1000));
  //  Serial.println("mv");
  //  Serial.println(analogRead(PIN_VBATT));
  //
  //  g_led_bianco->off();
  //  g_led_verde->off();
}
