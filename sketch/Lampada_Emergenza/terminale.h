/**
 * *****************************************************************************
 * \brief   Serial Terminal
 *
 * \Author  Ivan Zoli
 *****************************************************************************/

// Include only once
#ifndef TERMINAL_H
#define TERMINAL_H

////////////////////////////////////////////////////////////////////////////////
//                          I N C L U D E S                                   //
////////////////////////////////////////////////////////////////////////////////

// System Library Include
#include <stdint.h>
#include <stddef.h>

// User Library Include

// Application Local Include

/// Command function prototype
typedef int (*TermCmdFunc)(int argc, char** argv);
/// Flags
typedef uint8_t TermAccessFlg;    ///< Terminal access privilege mask

///
/// \brief Termina command
///
class TermCmd {
public:
    TermCmd(const char* name, const TermCmdFunc func, const TermAccessFlg access, const char* help="")
    : m_name(name)
    , m_func(func)
    , m_access(access)
    , m_help(help)
    {}
    
public:
    const char*         m_name;     ///< Command name
    const TermCmdFunc   m_func;     ///< Command function
    const TermAccessFlg m_access;   ///< Access control flags function
    const char*         m_help;     ///< Brief help string for command
};

///
/// \brief Terminale
///
class Terminale
{
public:
  Terminale(const TermCmd* comandi);

  ///
  /// \brief risposta ai comandi
  ///
  void doWorkAndAnswer(char* cmd);

protected:
  /// \brief Stampa l'help
  ///
  void terrminale_help();

private:
  const TermCmd*  m_comandi;
};

///
/// \brief Fine della lista
///
#define TERMINALE_FINE_LISTA  { 0, 0, 0, 0 }


#endif
