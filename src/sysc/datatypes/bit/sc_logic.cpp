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

  sc_logic.cpp -- C++ implementation of logic type. Behaves
                  pretty much the same way as HDLs logic type.

  Original Author: Stan Y. Liao, Synopsys, Inc.

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date:
  Description of Modification:

 *****************************************************************************/


// $Log: sc_logic.cpp,v $
// Revision 1.1.1.1  2006/12/15 20:20:04  acg
// SystemC 2.3
//
// Revision 1.3  2006/01/13 18:53:53  acg
// Andy Goodrich: added $Log command so that CVS comments are reproduced in
// the source.
//

#include "sysc/datatypes/bit/sc_bit_ids.h"
#include "sysc/datatypes/bit/sc_logic.h"
//-------------------------------------------------------Farah is working here 
sc_dt::sc_logic_value_t sc_dt::sc_logic::to_value( sc_logic_value_t v )
	{
	    if( v < Log_0 || v > Log_X ) {
		invalid_value( v );
	    }
	    return v;
	}

sc_dt::sc_logic_value_t sc_dt::sc_logic::to_value( bool b )
	{ return ( b ? Log_1 : Log_0 ); }

    sc_dt::sc_logic_value_t sc_dt::sc_logic::to_value( char c )
	{
	    sc_logic_value_t v;
	    unsigned int index = (int)c;
	    if ( index > 127 )
	    {
	        invalid_value(c);
		v = Log_X;
	    }
	    else
	    {
		v = char_to_logic[index];
		if( v < Log_0 || v > Log_X ) {
		    invalid_value( c );
		}
	    }
	    return v;
	}

sc_dt::sc_logic_value_t sc_dt::sc_logic::to_value( int i )
	{
	    if( i < 0 || i > 3 ) {
		invalid_value( i );
	    }
	    return sc_logic_value_t( i );
	}


sc_dt::sc_logic::sc_logic()
	: m_val( Log_X )
	{}

sc_dt::sc_logic::sc_logic( const sc_logic& a )
	: m_val( a.m_val )
	{}

sc_dt::sc_logic::sc_logic( sc_logic_value_t v )
	: m_val( to_value( v ) )
	{}

sc_dt::sc_logic::sc_logic( bool a )
	: m_val( to_value( a ) )
	{}

sc_dt::sc_logic::sc_logic( char a )
	: m_val( to_value( a ) )
	{}

sc_dt::sc_logic::sc_logic( int a )
	: m_val( to_value( a ) )
	{}

sc_dt::sc_logic::sc_logic( const sc_bit& a )
	: m_val( to_value( a.to_bool() ) )
	{}

sc_dt::sc_logic::~sc_logic()
	{}

const sc_dt::sc_logic sc_dt::sc_logic::operator ~ () const
	{ return sc_logic( not_table[m_val] ); }

sc_dt::sc_logic& sc_dt::sc_logic::b_not()
	{ m_val = not_table[m_val]; return *this; }

sc_dt::sc_logic_value_t sc_dt::sc_logic::value() const
	{ return m_val; }

bool sc_dt::sc_logic::is_01() const
	{ return ( (int) m_val == Log_0 || (int) m_val == Log_1 ); }

bool sc_dt::sc_logic::to_bool() const
	{ if( ! is_01() ) { invalid_01(); } return ( (int) m_val != Log_0 ); }

char sc_dt::sc_logic::to_char() const
	{ return logic_to_char[m_val]; }

void sc_dt::sc_logic::print( ::std::ostream& os ) const
	{ os << to_char(); }

void* sc_dt::sc_logic::operator new( std::size_t, void* p ) // placement new
	{ return p; }

void* sc_dt::sc_logic::operator new( std::size_t sz )
	{ return sc_core::sc_mempool::allocate( sz ); }

void sc_dt::sc_logic::operator delete( void* p, std::size_t sz )
	{ sc_core::sc_mempool::release( p, sz ); }

void* sc_dt::sc_logic::operator new [] ( std::size_t sz )
	{ return sc_core::sc_mempool::allocate( sz ); }

void sc_dt::sc_logic::operator delete [] ( void* p, std::size_t sz )
	{ sc_core::sc_mempool::release( p, sz ); }

const sc_dt::sc_logic sc_dt::operator & ( const sc_logic& a, const sc_logic& b )
  { return sc_logic( sc_logic::and_table[a.m_val][b.m_val] ); }

const sc_dt::sc_logic sc_dt::operator | ( const sc_logic& a, const sc_logic& b )
  { return sc_logic( sc_logic::or_table[a.m_val][b.m_val] ); }

const sc_dt::sc_logic sc_dt::operator ^ ( const sc_logic& a, const sc_logic& b )
  { return sc_logic( sc_logic::xor_table[a.m_val][b.m_val] ); }

bool sc_dt::operator == ( const sc_logic& a, const sc_logic& b )
  { return ( (int) a.m_val == b.m_val ); }

bool sc_dt::operator != ( const sc_logic& a, const sc_logic& b )
   { return ( (int) a.m_val != b.m_val ); }

::std::ostream&
sc_dt::operator << ( ::std::ostream& os, const sc_logic& a )
{
    a.print( os );
    return os;
}

::std::istream&
sc_dt::operator >> ( ::std::istream& is, sc_logic& a )
{
    a.scan( is );
    return is;
}

//---------------------------------------------------Farah is done working here
namespace sc_dt
{

// ----------------------------------------------------------------------------
//  CLASS : sc_logic
//
//  Four-valued logic type.
// ----------------------------------------------------------------------------

// support methods

void
sc_logic::invalid_value( sc_logic_value_t v )
{
    char msg[BUFSIZ];
    std::sprintf( msg, "sc_logic( %d )", v );
    SC_REPORT_ERROR( sc_core::SC_ID_VALUE_NOT_VALID_, msg );
}

void
sc_logic::invalid_value( char c )
{
    char msg[BUFSIZ];
    std::sprintf( msg, "sc_logic( '%c' )", c );
    SC_REPORT_ERROR( sc_core::SC_ID_VALUE_NOT_VALID_, msg );
}

void
sc_logic::invalid_value( int i )
{
    char msg[BUFSIZ];
    std::sprintf( msg, "sc_logic( %d )", i );
    SC_REPORT_ERROR( sc_core::SC_ID_VALUE_NOT_VALID_, msg );
}


void
sc_logic::invalid_01() const
{
    if( (int) m_val == Log_Z ) {
	SC_REPORT_WARNING( sc_core::SC_ID_LOGIC_Z_TO_BOOL_, 0 );
    } else {
	SC_REPORT_WARNING( sc_core::SC_ID_LOGIC_X_TO_BOOL_, 0 );
    }
}


// conversion tables

const sc_logic_value_t sc_logic::char_to_logic[128] =
{
    Log_0, Log_1, Log_Z, Log_X, Log_X, Log_X, Log_X, Log_X,
    Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X,
    Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X,
    Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X,
    Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X,
    Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X,
    Log_0, Log_1, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X,
    Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X,
    Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X,
    Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X,
    Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X,
    Log_X, Log_X, Log_Z, Log_X, Log_X, Log_X, Log_X, Log_X,
    Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X,
    Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X,
    Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X,
    Log_X, Log_X, Log_Z, Log_X, Log_X, Log_X, Log_X, Log_X
};

const char sc_logic::logic_to_char[4] = { '0', '1', 'Z', 'X' };

const sc_logic_value_t sc_logic::and_table[4][4] =
{
    { Log_0, Log_0, Log_0, Log_0 },
    { Log_0, Log_1, Log_X, Log_X },
    { Log_0, Log_X, Log_X, Log_X },
    { Log_0, Log_X, Log_X, Log_X }
};

const sc_logic_value_t sc_logic::or_table[4][4] =
{
    { Log_0, Log_1, Log_X, Log_X },
    { Log_1, Log_1, Log_1, Log_1 },
    { Log_X, Log_1, Log_X, Log_X },
    { Log_X, Log_1, Log_X, Log_X }
};

const sc_logic_value_t sc_logic::xor_table[4][4] =
{
    { Log_0, Log_1, Log_X, Log_X },
    { Log_1, Log_0, Log_X, Log_X },
    { Log_X, Log_X, Log_X, Log_X },
    { Log_X, Log_X, Log_X, Log_X }
};

const sc_logic_value_t sc_logic::not_table[4] =
    { Log_1, Log_0, Log_X, Log_X  };


// other methods

void
sc_logic::scan( ::std::istream& is )
{
    char c;
    is >> c;
    *this = c;
}


// #ifdef SC_DT_DEPRECATED
const sc_logic sc_logic_0( Log_0 );
const sc_logic sc_logic_1( Log_1 );
const sc_logic sc_logic_Z( Log_Z );
const sc_logic sc_logic_X( Log_X );
// #endif

const sc_logic SC_LOGIC_0( Log_0 );
const sc_logic SC_LOGIC_1( Log_1 );
const sc_logic SC_LOGIC_Z( Log_Z );
const sc_logic SC_LOGIC_X( Log_X );

} // namespace sc_dt
