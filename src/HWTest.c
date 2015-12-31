/*
 * This file is part of Raspberry Pi steuert Modelleisenbahn
 * Copyright(C)  2008 - 2015 Kurt Zerzawy www.zerzawy.ch
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
 *
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK
 * www.speak.li
 */

/**
 * \file
 * hardware test program
 * \author
 * Kurt Zerzawy
 */

/* 
 * HW Test programm
 * Spreitenbacher Eisenbahn Amateur Klub SPEAK
 * www.speak.li
 *
 * $Author: Kurt $
 * $Date: 2015-08-05 08:06:23 +0200 (Mi, 05 Aug 2015) $
 * $Revision: 2537 $
 */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include "HW.h"

/**
* prints the main title of the project
*/
void print_main( void )
 {
  printf ( "This file is part of Raspberry Pi steuert Modelleisenbahn.\n" );
  printf ( "\n" );
  printf ( "uPStellwerk is free software: you can redistribute it and/or modify\n" );
  printf ( "it under the terms of the GNU General Public License as published by\n" );
  printf ( "the Free Software Foundation, either version 3 of the License, or\n" );
  printf ( "(at your option) any later version.\n" );
  printf ( "\n\n" );
  printf ( "uPStellwerk is distributed in the hope that it will be useful,\n" );
  printf ( "but WITHOUT ANY WARRANTY; without even the implied warranty of\n" );
  printf ( "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n" );
  printf ( "GNU General Public License for more details.\n" );
  printf ( "\n" );
  printf ( "You should have received a copy of the GNU General Public License\n" );
  printf ( "along with uPStellwerk.  If not, see <http://www.gnu.org/licenses/>.\n" );
  printf ( "\n" );
  printf ( "Test program for the uController Hardware\n" );
  printf ( "author: Kurt Zerzawy, www.zerzawy.ch\n" );
  printf ( "date: %s\n", __DATE__);
  printf ( "\n" );
 };
 
/**
* prints a title and a line underneath
* @param title title to print out
*/
void print_title( title )
 char * title;
 {
  int i;
  
  printf ( "\n" );
  printf ( "%s\n", title );
  for( i = 0; i < strlen( title ); i++ ) printf ( "-" );
  printf ( "\n" );
 }
 
void print_footer( void )
 {
  printf ( "\n" );
  printf ( "q : quit" );
  printf ( "\n" );
  printf ( "\n>" );
 }
 
int dummy_int_void( void )
 {
  return 0;
 };
 
void dummy_void_int( dummy_var )
 int dummy_var;
 {
  return;
 };
 
int main( void )
 {
  const char * main_title = "main menu";
  void (* exit) ( void ) = ports_cleanup;
  
  struct smenu
   {
    int     key;
    char *  text;
    void    (*f)( int );
    int     (*g)( void );
   };
   
  struct smenu menu[] = 
    { '1', "strobe output    (A14, C14)", port_set_out_strobe,  dummy_int_void,
      '2', "clk output       (A18, C18)", port_set_out_clk,     dummy_int_void,
      '3', "data output      (A16)",      port_set_out_out,     dummy_int_void,
      '4', "data from output (C16)",      dummy_void_int,       port_get_out_in, 
      '0', " ",                           dummy_void_int,       dummy_int_void,
      '5', "Pn/S input       (A20, C20)", port_set_in_ps,       dummy_int_void,
      '6', "clk input        (A22, C22)", port_set_in_clk,      dummy_int_void,
      '7', "data to input    (A24)",      port_set_in_out,      dummy_int_void,
      '8', "data from input  (C24)",      dummy_void_int,       port_get_in_in,
      '0', " ",                           dummy_void_int,       dummy_int_void,
      '9', "LAN LED",                     port_set_lan_led,     dummy_int_void,
      'a', "key",                         dummy_void_int,       port_get_key,
      'b', "watchdog",                    port_set_watchdog,    dummy_int_void,
      '-', " ",                           dummy_void_int,       dummy_int_void,
    };
  long    i, j;
  char    ch;           /* variable for menu choice */
  char    ch1;          /* variable for submenu     */
  
  
  /* print main title */
  print_main();
  
   
  if( 0 != atexit( exit ))
   {
    fprintf ( stderr, "__FILE__:new_port_a at line __LINE__, not able to initialise exit function \n" );
   }
  ports_init();

  do
   {
    /* print main menu and ask for selektion  */
    
    /* print title  */
    print_title( main_title );
    
    i = -1;
    while( 1 )
     {
      i++;
      if( '-' == menu[i].key )
        break;
      if( '0' != menu[i].key )
        printf ( "%c : %s\n", menu[i].key, menu[i].text );
      else
        printf ( "\n" );
     }
    
    print_footer();
    ch = getchar();

    i = -1;
    while( 1 )
     {
      i++;
      if( '-' == menu[i].key )
        /* letzter Eintrag  */
        break;
      if( ch == menu[i].key )
       {
        if(( '-' == ch ) || ( '0' == ch ) || ( 'q' == ch ))
          /* find out impossible values   */
          break;
        ch = '\0';
        while( 1 )
         {
          ch1 = '\0';
          if( menu[i].f != dummy_void_int )
           /* first function supported  */
           {
            print_title( menu[i].text );
            printf ( "0 : set to 0\n" );
            printf ( "1 : set to 1\n");
            printf ( "p : pulse output\n");
            print_footer();
            ch1 = getchar();
             printf ( "->%c: ", ch1 );
            
            switch( ch1 ) 
             {
            case '0': 
              printf ( "switch to 0\n" );
              menu[i].f ( 0 );
              break;
            case '1': 
              printf ( "switch to 1\n" );
              menu[i].f ( 1 );
              break;
            case 'p': 
              printf ( "pulsing output, please wait\n" );
              for( j = 0; j < 1000000; j++ )
               {
                menu[i].f ( 1 );
                menu[i].f ( 0 );
               }
              break;
            case 'q': break;
            default: 
              printf ( "Falsche Eingabe\n" );
              break; 
             }
            if( 'q' == ch1 )
             {
              printf ( "verlasse Menu\n" );
              break;
             }
            ch1 = '\0';
           }
          if( menu[i].g != dummy_int_void )
           {
           /* second function supported  */
            print_title( menu[i].text );
            printf ( "r : read\n" );
            print_footer();
            ch1 = getchar();
           
            if( 'r' == ch1 )
             {
              if( menu[i].g() )
                printf ( "1" );
              else
                printf ( "0" );
             }
            if( 'q' == ch1 )
              break;
            ch1 = '\0';
           }
         }
       }
     }
   }
  while( 'q' != ch );
  return 0;
 }
