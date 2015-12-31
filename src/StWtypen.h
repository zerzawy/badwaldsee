/* 
 * This file is part of Raspberry Pi steuert Modelleisenbahn
 * Copyright (C)  2008, 2009 Kurt Zerzawy www.zerzawy.ch
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

/* This is the layer with the data types
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK
 * www.speak.li
 *
 * $Author: Kurt $
 * $Date: 2015-08-05 08:06:23 +0200 (Mi, 05 Aug 2015) $
 * $Revision: 2537 $
 */

#ifndef STWPARAM_H_
#define STWPARAM_H_


#include "StWtypen.h"
#include "params.h"

/* Ausgabe-Typ, wird verwendet für SetzeBit         */
enum AusgabeTyp
{
  AUS_EIN,          /* Ausgang ist eingeschaltet    */
  AUS_AUS,          /* Ausgang ist ausgeschaltet    */
  AUS_BLINKEN,      /* Ausgang blinkt               */
  AUS_BLINKEN_INV   /* Ausgang blinkt invers        */
};


//! Typ für die Farbe, die eine LED annehmen kann
enum LEDFarbTyp
{
  LED_KEINE,        /* keine LED leuchtet           */
  LED_WEISS,        /* weisse LED leuchtet          */
  LED_ROT,          /* rote LED leuchtet            */
  LED_AUTO,         /* belegt: rote LED, frei: keine LED  */
  LED_AUTO_WEISS    /* belegt: rote LED, frei: weisse LED */
};

/* TypEN für die Anzeige eines Signales.        */
/* Hauptsignal  */
enum eHauptSignalFarbTyp
{
  SIG_H_DUNKEL,     /* Hauptsignal ist dunkel           */
  SIG_H_HALT,       /* Hauptsignal zeigt Halt           */
  SIG_H_NOTHALT,    /* Hauptsignal zeigt Nothalt        */
  SIG_H_1,          /* Hauptsignal zeigt Fahrbegriff 1  */
  SIG_H_2,          /*                               2  */
  SIG_H_3,          /*                               3  */
  SIG_H_4,          /*                               4  */
  SIG_H_5,          /*                               5  */
  SIG_H_6,          /*                               6  */
  SIG_H_ANZ_BEGRIFFE
};

enum eVorSignalFarbTyp
{
  SIG_V_DUNKEL,     /* Vorsignal ist dunkel             */
  SIG_V_WARNUNG,    /* Vorsignal zeigt Warnung          */
  SIG_V_1,          /* Vorsignal zeigt Fahrbegriff 1*   */
  SIG_V_2,          /*                             2*   */
  SIG_V_3,          /*                             3*   */
  SIG_V_5           /*                             5*   */
};

enum AbfSignalTyp
{
  SIG_A_DUNKEL,     /* Abfahrsignal ist dunkel          */
  SIG_A_EIN         /* Abfahrsignal zeigt Abfahrt       */
};


#endif /* STWPARAM_H_ */
