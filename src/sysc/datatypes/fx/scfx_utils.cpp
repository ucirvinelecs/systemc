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

  scfx_utils.cpp - 

  Original Author: Martin Janssen, Synopsys, Inc.

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date:
  Description of Modification:

 *****************************************************************************/


// $Log: scfx_utils.cpp,v $
// Revision 1.1.1.1  2006/12/15 20:20:04  acg
// SystemC 2.3
//
// Revision 1.3  2006/01/13 18:53:58  acg
// Andy Goodrich: added $Log command so that CVS comments are reproduced in
// the source.
//

#include "sysc/datatypes/fx/scfx_utils.h"
//-------------------------------------------------Farah is working here

// ----------------------------------------------------------------------------
//  Utilities for parsing a character string number
// ----------------------------------------------------------------------------
int
sc_dt::scfx_parse_sign( const char*& s, bool& sign_char )
{
    int sign = 1;

    if( *s == '+' )
    {
        ++ s;
	sign_char = true;
    }
    else if( *s == '-' )
    {
        sign = -1;
        ++ s;
	sign_char = true;
    }
    else
	sign_char = false;

    return sign;
}

  sc_dt::sc_numrep
sc_dt::scfx_parse_prefix( const char*& s )
{
    if( s[0] == '0' ) {
        switch( s[1] )
        {
	    case 'b':
	    case 'B':
            {
	        if( (s[2] == 'u' || s[2] == 'U') && (s[3] == 's' || s[3] == 'S') ) {
		    s += 4;
                    return SC_BIN_US;
                }
                if( (s[2] == 's' || s[2] == 'S') && (s[3] == 'm' || s[3] == 'M') ) {
                    s += 4;
                    return SC_BIN_SM;
                }
                s += 2;
                return SC_BIN;
            }
	    case 'o':
	    case 'O':
	    {
                if( (s[2] == 'u' || s[2] == 'U') && (s[3] == 's' || s[3] == 'S') ) {
                    s += 4;
                    return SC_OCT_US;
                }
                if( (s[2] == 's' || s[2] == 'S') && (s[3] == 'm' || s[3] == 'M') ) {
                    s += 4;
                    return SC_OCT_SM;
                }
                s += 2;
                return SC_OCT;
            }
  	    case 'x':
	    case 'X':
            {
                if( (s[2] == 'u' || s[2] == 'U') && (s[3] == 's' || s[3] == 'S') ) {
                    s += 4;
                    return SC_HEX_US;
                }
                if( (s[2] == 's' || s[2] == 'S') && (s[3] == 'm' || s[3] == 'M') ) {
                    s += 4;
                    return SC_HEX_SM;
                }
                s += 2;
                return SC_HEX;
            }
   	    case 'd':
	    case 'D':
            {
                s += 2;
                return SC_DEC;
            }
	    case 'c':
	    case 'C':
            {
                if( (s[2] == 's' || s[2] == 'S') && (s[3] == 'd' || s[3] == 'D') ) {
                    s += 4;
                    return SC_CSD;
                }
                break;
            }
            default:
                break;
        }
    }

    return SC_DEC;
}


int
sc_dt::scfx_parse_base( const char*& s )
{
    const char* s1 = s + 1;

    int base = 10;

    if( *s == '0' )
    {
        switch( *s1 )
	{
            case 'b':
            case 'B': base =  2; s += 2; break;
            case 'o':
            case 'O': base =  8; s += 2; break;
            case 'd':
            case 'D': base = 10; s += 2; break;
            case 'x':
            case 'X': base = 16; s += 2; break;
        }
    }

    return base;
}


bool
sc_dt::scfx_is_equal( const char* a, const char* b )
{
    while( *a != 0 && *b != 0 && *a == *b )
    {
        ++ a;
        ++ b;
    }
    return ( *a == 0 && *b == 0 );
}


bool
sc_dt::scfx_is_nan( const char* s )
{
    return scfx_is_equal( s, "NaN" );
}


bool
sc_dt::scfx_is_inf( const char* s )
{
    return ( scfx_is_equal( s, "Inf" ) || scfx_is_equal( s, "Infinity" ) );
}


bool
sc_dt::scfx_exp_start( const char* s )
{
    if( *s == 'e' || *s == 'E' )
    {
        ++ s;
        if( *s == '+' || *s == '-' )
            return true;
    }
    return false;
}


bool
sc_dt::scfx_is_digit( char c, sc_numrep numrep )
{
    bool is_digit;

    switch( numrep )
    {
        case SC_DEC:
	{
            switch( c )
	    {
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
		{
                    is_digit = true;
                    break;
		}
                default:
                    is_digit = false;
            }
            break;
	}
	case SC_BIN:
	case SC_BIN_US:
	case SC_BIN_SM:
	{
            switch( c )
	    {
                case '0': case '1':
		{
                    is_digit = true;
                    break;
		}
                default:
                    is_digit = false;
            }
            break;
	}
	case SC_OCT:
	case SC_OCT_US:
	case SC_OCT_SM:
	{
            switch( c )
	    {
                case '0': case '1': case '2': case '3':
                case '4': case '5': case '6': case '7':
		{
                    is_digit = true;
                    break;
		}
                default:
                    is_digit = false;
            }
            break;
	}
	case SC_HEX:
	case SC_HEX_US:
	case SC_HEX_SM:
	{
            switch( c )
	    {
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		{
                    is_digit = true;
                    break;
		}
                default:
                    is_digit = false;
            }
            break;
	}
	case SC_CSD:
	{
            switch( c )
	    {
		case '0': case '1': case '-':
		{
                    is_digit = true;
                    break;
		}
                default:
                    is_digit = false;
            }
            break;
	}
	default:
            is_digit = false;
    }

    return is_digit;
}


int
sc_dt::scfx_to_digit( char c, sc_numrep numrep )
{
    int to_digit;
    
    switch( numrep )
    {
	case SC_DEC:
	case SC_BIN:
	case SC_BIN_US:
	case SC_BIN_SM:
	case SC_OCT:
	case SC_OCT_US:
	case SC_OCT_SM:
	{
            to_digit = c - '0';
            break;
	}
	case SC_HEX:
	case SC_HEX_US:
	case SC_HEX_SM:
	{
            switch( c )
	    {
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                    to_digit = c - '0';
                    break;
                case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                    to_digit = c - 'a' + 10;
                    break;
                case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                    to_digit = c - 'A' + 10;
                    break;
                default:
                    to_digit = -2;
            }
            break;
	}
	case SC_CSD:
	{
            if( c == '-' )
                to_digit = -1;
            else
                to_digit = c - '0';
            break;
	}
        default:
            to_digit = -2;
    }

    return to_digit;
}


// ----------------------------------------------------------------------------
//  Utilities for printing a character string number
// ----------------------------------------------------------------------------


void
sc_dt::scfx_print_nan( scfx_string& s )
{
    s += "NaN";
}


void
sc_dt::scfx_print_inf( scfx_string& s, bool negative )
{
    if( negative )
        s += "-Inf";
    else
        s += "Inf";
}


void
sc_dt::scfx_print_prefix( scfx_string& s, sc_numrep numrep )
{
    switch( numrep )
    {
        case SC_DEC:
	    s += "0d";
	    break;
        case SC_BIN:
	    s += "0b";
	    break;
        case SC_BIN_US:
	    s += "0bus";
	    break;
        case SC_BIN_SM:
	    s += "0bsm";
	    break;
        case SC_OCT:
	    s += "0o";
	    break;
        case SC_OCT_US:
	    s += "0ous";
	    break;
        case SC_OCT_SM:
	    s += "0osm";
	    break;
        case SC_HEX:
	    s += "0x";
	    break;
        case SC_HEX_US:
	    s += "0xus";
	    break;
        case SC_HEX_SM:
	    s += "0xsm";
	    break;
        case SC_CSD:
	    s += "0csd";
	    break;
        default:
	    s += "unknown";
    }
}


void
sc_dt::scfx_print_exp( scfx_string& s, int exp )
{
    if( exp != 0 )
    {
        s += 'e';

        if( exp < 0 )
	{
            exp = - exp;
            s += '-';
        }
	else
            s += '+';

        bool first = true;
        int scale = 1000000000;
        do
	{
            int digit = exp / scale;
            exp = exp % scale;
            if( digit != 0 || ! first )
	    {
                s += static_cast<char>( digit + '0' );
                first = false;
            }
            scale /= 10;
        }
	while( scale > 0 );
    }
}

//--------------------------------------------Farah is done working here
namespace sc_dt
{

void
scfx_tc2csd( scfx_string& s, int w_prefix )
{
    if( w_prefix != 0 ) {
	SC_ASSERT_( s[0] == '0' && s[1] == 'c' &&
		    s[2] == 's' && s[3] == 'd', "invalid prefix" );
    }

    scfx_string csd;

    // copy bits from 's' into 'csd'; skip prefix, point, and exponent
    int i = 0;
    int j = (w_prefix != 0 ? 4 : 0);
    while( s[j] )
    {
	if( s[j] == '0' || s[j] == '1' )
	    csd[i ++] = s[j];
	else if( s[j] != '.' )
	    break;
	++ j;
    }
    csd[i] = '\0';

    // convert 'csd' from two's complement to csd
    -- i;
    while( i >= 0 )
    {
	if( csd[i] == '0' )
	    -- i;
	else
	{
	    if( i > 0 && csd[i - 1] == '0' )
		-- i;
	    else if( i == 0 )
		csd[i --] = '-';
	    else
	    {   // i > 0 && csd[i - 1] == '1'
		csd[i --] = '-';
		while( i >= 0 && csd[i] == '1' )
		    csd[i --] = '0';
		if( i > 0 )
		    csd[i] = '1';
		else if( i == 0 )
		    csd[i --] = '1';
	    }
	}
    }

    // copy bits from 'csd' back into 's'
    i = 0;
    j = (w_prefix != 0 ? 4 : 0);
    while( csd[i] )
    {
	if( s[j] == '.' )
	    ++ j;
	s[j ++] = csd[i ++];
    }
}


void
scfx_csd2tc( scfx_string& csd )
{
    SC_ASSERT_( csd[0] == '0' && csd[1] == 'c' &&
		csd[2] == 's' && csd[3] == 'd', "invalid prefix" );

    scfx_string s;

    // copy bits from 'csd' into 's'; skip prefix, point, and exponent
    int i = 0;
    s[i ++] = '0';
    int j = 4;
    while( csd[j] )
    {
	if( csd[j] == '-' || csd[j] == '0' || csd[j] == '1' )
	    s[i ++] = csd[j];
	else if( csd[j] != '.' )
	    break;
	++ j;
    }
    s[i] = '\0';

    // convert 's' from csd to two's complement
    int len = i;
    i = 1;
    while( i < len )
    {
        while( i < len && s[i] != '-' )
	    i ++;
	if( i < len )
	{
	    j = i ++;
	    s[j --] = '1';
	    while( j >= 0 && s[j] == '0' )
	        s[j --] = '1';
	    if( j >= 0 )
	        s[j] = '0';
	}
    }

    // copy bits from 's' back into 'csd'
    j = csd.length();
    csd[j + 1] = '\0';
    while( j > 4 )
    {
	csd[j] = csd[j - 1];
	-- j;
    }
        
    i = 0;
    j = 4;
    while( s[i] )
    {
	if( csd[j] == '.' )
	    ++ j;
	csd[j ++] = s[i ++];
    }
}

} // namespace sc_dt


// Taf!
