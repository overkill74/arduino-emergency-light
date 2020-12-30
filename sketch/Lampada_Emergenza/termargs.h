/**
 * *****************************************************************************
 * @file    termargs.h
 * @brief   Command line parsing functions for terminal
 *
 * $Author$
 * $Revision$
 * $LastChangedRevision$
 *
 * @copyright Copyright 2017 Dinema S.p.A. All rights reserved.
 *
 * @verbatim
 * @endverbatim
 *
 ******************************************************************************
 * @attention
 *
 ******************************************************************************
 * @note
 *
 *****************************************************************************/

//Include only once
#ifndef __TERMARGS_INCLUDED
#define __TERMARGS_INCLUDED

////////////////////////////////////////////////////////////////////////////////
//                          I N C L U D E S                                   //
////////////////////////////////////////////////////////////////////////////////

/* System Library Include
 */
#include <stdint.h>
#include <stdbool.h>

/* User Library Include
 */
// Intentionally blank section

/* Application Local Include
 */
// Intentionally blank section

#ifdef __cplusplus
extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////
//                  C O N S T A N T S   D E F I N I T I O N S                 //
////////////////////////////////////////////////////////////////////////////////

#define TERM_MAX_PARAM  8

	
////////////////////////////////////////////////////////////////////////////////
//                     T Y P E S    D E F I N I T I O N S                     //
////////////////////////////////////////////////////////////////////////////////

///< Terminal command parameters as integer
struct TermParams
{
	char    cmd[8];
	unsigned int param[TERM_MAX_PARAM];      ///< Integer Params
	int     count;                      ///< Param counter
};
typedef struct TermParams TermParams;


/*******************************************************************************
** @brief Divide command line into separate argument strings
** @warning The given command line buffer will be modified
** @param[in] cmd        commad line to process
** @param[in] argv       array of string pointers
** @param[in] argv_size  size of string pointers array
** @return number of succesfully processed arguments
*/
int argSegment(char *cmd, char *argv[], const unsigned argv_size);


/***************************************************************************//**
 * @brief Decode param string as decimal integers
 *
 * @param[out] out_p - decoded params
 * @param[in] param  - param string to parse
 * @return true if found at least one parameter
 *****************************************************************************/
bool argDecodeParam(TermParams* out_p, const char* param);


/*****************************************************************************
 * @brief Decode param string as hexadecimal integers
 *
 * @param[out] out_p - decoded params
 * @param[in] param  - param string to parse
 * @return true if found at least one parameter
 *****************************************************************************/
bool argDecodeParamHex(TermParams* out_p, const char* param);


/*****************************************************************************
 * @brief Decode param string as bytes array integers
 *
 * @param[out] out_b - decoded params
 * @param[in] out_sz - out_b size
 * @param[in] param  - param string to parse
 * @return number of parameter found
 *****************************************************************************/
size_t argDecodeByteArray(uint8_t* out_b, size_t out_sz, const char* param);


/*****************************************************************************
 * @brief Convert a string into integer, includin HEX (0x or 0X)
 *
 * @param[in] str    - string to convert
 * @return Integer
 *****************************************************************************/
int32_t argToInt(const char* str);

/*****************************************************************************
 * @brief Convert a string into integer, includin HEX (0x or 0X)
 *
 * @param[in] str    - string to convert
 * @return Integer
 *****************************************************************************/
uint32_t argToUint(const char* str);

#ifdef __cplusplus
}
#endif

#endif /* __TERMARGS_INCLUDED */
