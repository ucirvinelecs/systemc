/*****************************************************************************

  Licensed to Accellera Systems Initiative Inc. (Accellera) under one or
  more contributor license agreements.  See the NOTICE file distributed
  with this work for additional information regarding copyright ownership.
  Accellera licenses this file to you under the Apache License, Version 2.0
  (the "License"); you may not use this file except in compliance with the
  License.  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
  implied.  See the License for the specific language governing
  permissions and limitations under the License.

 *****************************************************************************/

/*****************************************************************************

  scfx_utils.h - 

  Original Author: Martin Janssen, Synopsys, Inc.

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date:
  Description of Modification:

 *****************************************************************************/

// $Log: scfx_utils.h,v $
// Revision 1.2  2009/02/28 00:26:20  acg
//  Andy Goodrich: bug fixes.
//
// Revision 1.1.1.1  2006/12/15 20:31:36  acg
// SystemC 2.2
//
// Revision 1.3  2006/01/13 18:53:58  acg
// Andy Goodrich: added $Log command so that CVS comments are reproduced in
// the source.
//

#ifndef SCFX_UTILS_H
#define SCFX_UTILS_H


#include "sysc/datatypes/fx/sc_fxdefs.h"
#include "sysc/datatypes/fx/scfx_params.h"
#include "sysc/datatypes/fx/scfx_string.h"


namespace sc_dt
{

// ----------------------------------------------------------------------------
//  Find the most and least significant non-zero bits in a unsigned long
// ----------------------------------------------------------------------------

#define MSB_STATEMENT(n) if( x >> n ) { x >>= n; i += n; }

inline
int
scfx_find_msb( unsigned long x )
{
    int i = 0;
#   if defined(SC_LONG_64)
        MSB_STATEMENT( 32 );
#   endif // defined(SC_LONG_64)
    MSB_STATEMENT( 16 );
    MSB_STATEMENT( 8 );
    MSB_STATEMENT( 4 );
    MSB_STATEMENT( 2 );
    MSB_STATEMENT( 1 );
    return i;
}

#undef MSB_STATEMENT

#define LSB_STATEMENT(n) if( x << n ) { x <<= n; i -= n; }

inline
int
scfx_find_lsb( unsigned long x )
{
    int i;
#   if defined(SC_LONG_64)
        i = 63;
        LSB_STATEMENT( 32 );
#   else
        i = 31;
#   endif // defined(SC_LONG_64)
    LSB_STATEMENT( 16 );
    LSB_STATEMENT( 8 );
    LSB_STATEMENT( 4 );
    LSB_STATEMENT( 2 );
    LSB_STATEMENT( 1 );
    return i;
}

#undef LSB_STATEMENT


// ----------------------------------------------------------------------------
//  Utilities for parsing a character string number
// ----------------------------------------------------------------------------

int
scfx_parse_sign( const char*& s, bool& sign_char );

sc_numrep
scfx_parse_prefix( const char*& s );

int
scfx_parse_base( const char*& s );

bool
scfx_is_equal( const char* a, const char* b );

bool
scfx_is_nan( const char* s );

bool
scfx_is_inf( const char* s );

bool
scfx_exp_start( const char* s );

bool
scfx_is_digit( char c, sc_numrep numrep );

int
scfx_to_digit( char c, sc_numrep numrep );


// ----------------------------------------------------------------------------
//  Utilities for printing a character string number
// ----------------------------------------------------------------------------

void
scfx_print_nan( scfx_string& s );

void
scfx_print_inf( scfx_string& s, bool negative );

void
scfx_print_prefix( scfx_string& s, sc_numrep numrep );

void
scfx_print_exp( scfx_string& s, int exp );

void scfx_tc2csd( scfx_string&, int );
void scfx_csd2tc( scfx_string& );

} // namespace sc_dt


#endif

// Taf!
