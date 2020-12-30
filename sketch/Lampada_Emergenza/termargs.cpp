/**
*******************************************************************************
* @file    termargs.c
* @brief   Argument parsing functions for terminal - source file
*
* $Author$
* $Revision$
* $LastChangedRevision$
*
* @copyright Copyright 2017 Dinema S.p.A. All rights reserved.
*
******************************************************************************/

////////////////////////////////////////////////////////////////////////////////
//                           I N C L U D E S                                  //
////////////////////////////////////////////////////////////////////////////////

/* System Library Include
*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


/* User Library Include
 */
//#include "../inc/strconv.h"
#include "termargs.h"


////////////////////////////////////////////////////////////////////////////////
//                     T Y P E S    D E F I N I T I O N S                     //
////////////////////////////////////////////////////////////////////////////////

/// 
struct _arg_segment_data {
	char *p; // Pointer to current char
	char **argv;
	unsigned argc_max;
	unsigned argc; // arguments found counter
	unsigned prev_quote: 1;
	unsigned at_start: 1;
	unsigned quoted: 1;
	char *c_arg; // Candidate argument pointer
};


////////////////////////////////////////////////////////////////////////////////
//              P R I V A T E   S T A T I C   F U N C T I O N S               //
////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
** Add current argument to string array
*/
static void _add_arg(struct _arg_segment_data * d)
{
	if (d->prev_quote)
		*(d->p-1) = 0;
	else
		*d->p = 0;
	if (d->c_arg != 0) {
		d->c_arg += d->quoted;
		if (d->argc < d->argc_max)
			d->argv[d->argc++] = d->c_arg;
	}
	d->c_arg = 0;
}

/*******************************************************************************
** Decode string as hexadecimal integer
*/
static uint32_t atox(const char* txt)
{
    uint32_t tmpp = 0;
    char ch = *txt++;
    while (ch)
    {
        if (ch >= '0' && ch <= '9') {
            tmpp *= 16;
            tmpp += ch - '0';
        } else
        if (ch >= 'a' && ch <= 'f') {
            tmpp *= 16;
            tmpp += ch - ('a' - 10);
        } else
        if (ch >= 'A' && ch <= 'F') {
            tmpp *= 16;
            tmpp += ch - ('A' - 10);
        }
        ch = *txt++;
    }

    return tmpp;
}


////////////////////////////////////////////////////////////////////////////////
//                     P U B L I C   F U N C T I O N S                        //
////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
** Divide command line into separate argument strings
*/
int argSegment(char *cmd, char *argv[], const unsigned argv_size)
{
	struct _arg_segment_data d;
	register char c;

  if (!cmd || !argv) {
    return 0;
  }
  
	d.p = cmd;
	d.argv = argv;
	d.argc_max = argv_size;
	d.argc = 0;
	d.c_arg = NULL;
	d.at_start = true;
	d.quoted = false;
	d.prev_quote = false;
	
	while ((c = *d.p) != 0) {
		switch (c) {
			case '"':
				if (d.at_start) {
					d.quoted = true;
					d.c_arg = d.p;
				}
				d.at_start = false;
				d.prev_quote = true;
				break;
			
			case ' ':
				if ((!d.quoted) || (d.quoted && d.prev_quote)) {
					_add_arg(&d);
					d.quoted = false;
					d.at_start = true;
				}
				d.prev_quote = false;
				break;
			
			default:
				if (d.c_arg == 0)
					d.c_arg = d.p;
				d.at_start = false;
				d.prev_quote = false;
		}
		d.p++;
	}
	if ((!d.quoted) || (d.quoted && d.prev_quote))
		_add_arg(&d);
	
	return d.argc;
}

/*******************************************************************************
*/
int32_t argToInt(const char* str)
{
	if (!str) {
		return 0;
	}
	
	bool is_hex = false;
	do {
		int l = strlen(str);
		if (l < 3) { break; } // Deve essere almeno 3 caratteri (0x.)
		// Cerco 0x o 0X
		if (str[0] != '0') { break; }
		if (str[1] == 'x') { is_hex = true; break; }
		if (str[1] == 'X') { is_hex = true; break; }
	} while (0);

	if (is_hex) {
		return (int)atox(&str[2]);
	}

	return atoi(str);
}

/*******************************************************************************
*/
uint32_t argToUint(const char* str)
{
	if (!str) {
		return 0;
	}
	
	bool is_hex = false;
	do {
		int l = strlen(str);
		if (l < 3) { break; } // Deve essere almeno 3 caratteri (0x.)
		// Cerco 0x o 0X
		if (str[0] != '0') { break; }
		if (str[1] == 'x') { is_hex = true; break; }
		if (str[1] == 'X') { is_hex = true; break; }
	} while (0);

	if (is_hex) {
		return atox(&str[2]);
	}

	return (uint32_t)atoi(str);
}

/*******************************************************************************
** Decode param string as decimal integers
*/
//bool argDecodeParam(TermParams* out_p, const char* param)
//{
//	int tmpp;
//	size_t str_len = strlen(param);
//	bool is_valid = false;
//
//	if (out_p == NULL) {
//		return false;
//	}
//
//	out_p->count = 0;
//
//	if (!str_len) {
//		return false;
//	}
//
//	tmpp = 0;
//	for (unsigned x=0; x<=str_len; ++x) {
//		if ((param[x] >= '0') && (param[x] <= '9')) {
//			is_valid = true;
//			tmpp *= 10;
//			tmpp += param[x] - '0';
//		} else
//		if (param[x] == ' ' || param[x] == ',') {
//			if (is_valid) {
//				// Separatore parametro
//				out_p->param[out_p->count] = tmpp;
//				out_p->count++;
//				if (out_p->count >= TERM_MAX_PARAM) {
//					// Raggiunto il massimo
//					return true;
//				}
//				tmpp = 0;
//				is_valid = false;
//			}
//		}
//	}
//
//	if (is_valid) {
//		out_p->param[out_p->count++] = tmpp;
//	}
//
//	return out_p->count ? true : false;
//}

/*******************************************************************************
** Decode param string as hexadecimal integers
*/
//bool argDecodeParamHex(TermParams* out_p, const char* param)
//{
//    char* argv[16];
//    int n = argSegment((char*)param, argv, 16);
//    if (n < 2) {
//        out_p->count = 0;
//        return false;
//    }
//
//    out_p->count = n-1; // Skip command (1st)
//
//    for (int x=1; x<n; ++x) {
//        out_p->param[x-1] = atox(argv[x]);
//    }
//
//    return true;
//}


/*******************************************************************************
** Decode param string as bytes array integers
*/
//size_t argDecodeByteArray(uint8_t* out_b, size_t out_sz, const char* param)
//{
//	if (!param || !out_b)
//		return 0;
//
//	return unhexlify(out_b, param, out_sz);
//}

/* EOF */
