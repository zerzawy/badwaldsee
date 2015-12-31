/*
 * This file is part of Raspberry Pi steuert Modelleisenbahn
 * Copyright(C)  2009 - 2015 Kurt Zerzawy www.zerzawy.ch
 * 
 * Raspberry Pi steuert Modelleisenbahn is free software:
 * you can redistribute it and/or modify it under the terms of the
 * GNU General Public Licence as published by the Free Software Foundation,
 * either version 3 of the Licence, or (at your option) any later version.
 *
 * Raspberry Pi steuert Modelleisenbahn is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE. See GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Raspberry Pi steuert Modelleisenbahn. 
 * If not, see <http://www.gnu.org/licenses/>.
 */
 
/*
 * Functions for logging 
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK
 * www.speak.li
 *
 * $Author: Kurt $
 * $Date: 2015-08-05 08:06:23 +0200 (Mi, 05 Aug 2015) $
 * $Revision: 2537 $
 */

#ifndef BAD_WALDSEE_LOGGING_H__
#define BAD_WALDSEE_LOGGING_H__

#include "stdio.h"

#ifdef TESTRUN
char errtxt[1000];
char tmpstr[1000];
#endif

/** Note: there is no locking mechanism if several processes are using
 * the macros same time. It means the output can be strange if more
 * than one thread is using the macros same time.
 */

/**
 * Macro for writing error text to stderr
 * the macro is adding the name of the calling file the
 * name of the calling function and the line
 * @param   text to write
 */
#ifdef TESTRUN
#define LOG_ERR(txt) { sprintf(tmpstr, "ERROR %s:%s, line %d: %s\n", __FILE__, __FUNCTION__, __LINE__, (txt)); strcat(errtxt, tmpstr);}
#else
#define LOG_ERR(txt) fprintf(stderr, "ERROR %s:%s, line %d: %s\n", __FILE__, __FUNCTION__, __LINE__, (txt));
#endif

/** 
 * Macro for writing a log text to stderr
 * the macro is adding the name of the calling file the name
 * of the calling function and the line
 * @param   text to write
 */
#ifdef TESTRUN
#define LOG_INF(txt) { sprintf(errtxt, "INFO  %s:%s, line %d: %s\n", __FILE__, __FUNCTION__, __LINE__, (txt)); strcat(errtxt, tmpstr); }
#else
#define LOG_INF(txt) fprintf(stderr, "INFO  %s:%s, line %d: %s\n", __FILE__, __FUNCTION__, __LINE__, (txt));
#endif

/**
 * Macro for writing a log text to stderr
 * the macro is only sending the text given
 * @param   text to write
 */

#ifdef TESTRUN
#define LOG_STW(...) { strcat(errtxt, "STW: "); sprintf(tmpstr, __VA_ARGS__); strcat(errtxt, tmpstr); strcat(errtxt, "\n"); }
#else
#define LOG_STW(...) { fprintf(stderr, "STW: "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); }
#endif

/** 
 * Macro for clearing errortest string
 */
#ifdef TESTRUN
#define CLR_ERRTXT() { * errtxt = '\0'; }
#endif

#endif /* BAD_WALDSEE_LOGGING */

