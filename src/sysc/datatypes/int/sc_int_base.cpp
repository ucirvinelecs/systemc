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

  sc_int_base.cpp -- contains interface definitions between sc_int and
                sc_signed, sc_unsigned, and definitions for sc_int_subref.

  Original Author: Ali Dasdan, Synopsys, Inc.

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date:
  Description of Modification:
    
 *****************************************************************************/


// $Log: sc_int_base.cpp,v $
// Revision 1.5  2011/02/18 20:19:14  acg
//  Andy Goodrich: updating Copyright notice.
//
// Revision 1.4  2010/02/04 22:23:29  acg
//  Andy Goodrich: fixed bug in concatenation reads for part selections,
//  the mask being used was 32 bits and should have been 64 bits.
//
// Revision 1.3  2008/06/19 17:47:56  acg
//  Andy Goodrich: fixes for bugs. See 2.2.1 RELEASENOTES.
//
// Revision 1.2  2007/11/04 21:27:00  acg
//  Andy Goodrich: changes to make sure the proper value is returned from
//  concat_get_data().
//
// Revision 1.1.1.1  2006/12/15 20:20:05  acg
// SystemC 2.3
//
// Revision 1.3  2006/01/13 18:49:31  acg
// Added $Log command so that CVS check in comments are reproduced in the
// source.
//

#include "sysc/kernel/sc_macros.h"
#include "sysc/datatypes/int/sc_signed.h"
#include "sysc/datatypes/int/sc_unsigned.h"
#include "sysc/datatypes/int/sc_int_base.h"
#include "sysc/datatypes/int/sc_uint_base.h"
#include "sysc/datatypes/int/sc_signed.h"
#include "sysc/datatypes/int/sc_int_ids.h"
#include "sysc/datatypes/bit/sc_bv_base.h"
#include "sysc/datatypes/bit/sc_lv_base.h"
#include "sysc/datatypes/misc/sc_concatref.h"
#include "sysc/datatypes/fx/sc_fix.h"
#include "sysc/datatypes/fx/scfx_other_defs.h"

//---------------------------------------------Farah is working here 
// ----------------------------------------------------------------------------
//  CLASS : sc_int_bitref_r
//
//  Proxy class for sc_int bit selection (r-value only).
// ----------------------------------------------------------------------------
sc_dt::sc_int_bitref_r::sc_int_bitref_r() : sc_value_base(), m_index(), m_obj_p()
        {}

void sc_dt::sc_int_bitref_r::initialize( const sc_int_base* obj_p, int index_ )
    {
	m_obj_p = (sc_int_base*)obj_p;
	m_index = index_;
    }

sc_dt::sc_int_bitref_r::sc_int_bitref_r( const sc_int_bitref_r& a ) :
        sc_value_base(a), m_index(a.m_index), m_obj_p(a.m_obj_p)
        {}

sc_dt::sc_int_bitref_r::~sc_int_bitref_r()
	{}

int sc_dt::sc_int_bitref_r::length() const
	{ return 1; }

int sc_dt::sc_int_bitref_r::concat_length( bool *xz_present_p ) const
	{ if (xz_present_p) *xz_present_p = false; return 1; }

bool sc_dt::sc_int_bitref_r::concat_get_ctrl( sc_digit* dst_p, int low_i ) const
        {
	    int bit_mask = 1 << (low_i % BITS_PER_DIGIT);
	    int word_i = low_i / BITS_PER_DIGIT;

	    dst_p[word_i] &= ~bit_mask;
	    return false;
	}

bool sc_dt::sc_int_bitref_r::concat_get_data( sc_digit* dst_p, int low_i ) const
        {
	    bool non_zero;
	    int bit_mask = 1 << (low_i % BITS_PER_DIGIT);
	    int word_i = low_i / BITS_PER_DIGIT;

	    if ( operator uint64() )
	    {
		dst_p[word_i] |= bit_mask;
		non_zero = true;
	    }
	    else
	    {
		dst_p[word_i] &= ~bit_mask;
		non_zero = false;
	    }
	    return non_zero;
	}

sc_dt::uint64 sc_dt::sc_int_bitref_r::concat_get_uint64() const
	{ return operator uint64(); }

  sc_dt::uint64 sc_dt::sc_int_bitref_r::value() const
	{ return operator uint64(); }

bool sc_dt::sc_int_bitref_r::to_bool() const
	{ return operator uint64(); }

// other methods
void sc_dt::sc_int_bitref_r::print( ::std::ostream& os ) const
	{ os << to_bool(); }

// ----------------------------------------------------------------------------
//  CLASS : sc_int_bitref
//
//  Proxy class for sc_int bit selection (r-value and l-value).
// ----------------------------------------------------------------------------
sc_dt::sc_int_bitref::sc_int_bitref() : sc_int_bitref_r()
      {}
sc_dt::sc_int_bitref::sc_int_bitref( const sc_int_bitref& a ) : sc_int_bitref_r( a )
      {}

// ----------------------------------------------------------------------------
//  CLASS : sc_int_subref_r
//
//  Proxy class for sc_int part selection (r-value only).
// ----------------------------------------------------------------------------
sc_dt::sc_int_subref_r::sc_int_subref_r() : sc_value_base(), m_left(0), m_obj_p(0), m_right(0)
        {}

void sc_dt::sc_int_subref_r::initialize( const sc_int_base* obj_p, int left_i, int right_i )
    {
	m_obj_p = (sc_int_base*)obj_p;
	m_left = left_i;
	m_right = right_i;
    }

sc_dt::sc_int_subref_r::sc_int_subref_r( const sc_int_subref_r& a ) :
        sc_value_base(a), m_left( a.m_left ), m_obj_p( a.m_obj_p ), 
	m_right( a.m_right )
        {}

sc_dt::sc_int_subref_r::~sc_int_subref_r()
	{}

int sc_dt::sc_int_subref_r::length() const
        { return ( m_left - m_right + 1 ); }

int sc_dt::sc_int_subref_r::concat_length(bool* xz_present_p) const
	{ if ( xz_present_p ) *xz_present_p = false; return length(); }

sc_dt::uint64 sc_dt::sc_int_subref_r::concat_get_uint64() const
    {
	int    len = length();
	uint64 val = operator uint_type();
	if ( len < 64 )
	    return (uint64)(val & ~((uint_type)-1 << len));
	else
	    return (uint64)val;
    }

bool sc_dt::sc_int_subref_r::nand_reduce() const
	{ return ( ! and_reduce() ); }

bool sc_dt::sc_int_subref_r::nor_reduce() const
	{ return ( ! or_reduce() ); }

bool sc_dt::sc_int_subref_r::xnor_reduce() const
	{ return ( ! xor_reduce() ); }

sc_dt::uint_type sc_dt::sc_int_subref_r::value() const
	{ return operator uint_type(); }

void sc_dt::sc_int_subref_r::print( ::std::ostream& os) const
	{ os << to_string(sc_io_base(os,SC_DEC),sc_io_show_base(os)); }

// ----------------------------------------------------------------------------
//  CLASS : sc_int_subref
//
//  Proxy class for sc_int part selection (r-value and l-value).
// ----------------------------------------------------------------------------
sc_dt::sc_int_subref:: sc_int_subref() : sc_int_subref_r()
        {}

sc_dt::sc_int_subref::sc_int_subref( const sc_int_subref& a ) : sc_int_subref_r( a )
        {}

sc_dt::sc_int_subref& sc_dt::sc_int_subref::operator = ( const sc_int_subref_r& a )
	{ return operator = ( a.operator uint_type() ); }

sc_dt::sc_int_subref& sc_dt::sc_int_subref::operator = ( const sc_int_subref& a )
	{ return operator = ( a.operator uint_type() ); }

sc_dt::sc_int_subref& sc_dt::sc_int_subref::operator = ( unsigned long a )
	{ return operator = ( (int_type) a ); }

sc_dt::sc_int_subref& sc_dt::sc_int_subref::operator = ( long a )
	{ return operator = ( (int_type) a ); }

sc_dt::sc_int_subref& sc_dt::sc_int_subref::operator = ( unsigned int a )
	{ return operator = ( (int_type) a ); }

sc_dt::sc_int_subref& sc_dt::sc_int_subref::operator = ( int a )
	{ return operator = ( (int_type) a ); }

sc_dt::sc_int_subref& sc_dt::sc_int_subref::operator = ( uint64 a )
	{ return operator = ( (int_type) a ); }

sc_dt::sc_int_subref& sc_dt::sc_int_subref::operator = ( double a )
	{ return operator = ( (int_type) a ); }


// ----------------------------------------------------------------------------
//  class : sc_int_base
//
//  Base class for sc_int.
// ----------------------------------------------------------------------------
  
void sc_dt::sc_int_base::check_length() const
	{ if( m_len <= 0 || m_len > SC_INTWIDTH ) { invalid_length(); } }

void sc_dt::sc_int_base::check_index( int i ) const
	{ if( i < 0 || i >= m_len ) { invalid_index( i ); } }

void sc_dt::sc_int_base::check_range( int l, int r ) const
	{ if( r < 0 || l >= m_len || l < r ) { invalid_range( l, r ); } }

sc_dt::sc_int_base::sc_int_base( int w )
	: m_val( 0 ), m_len( w ), m_ulen( SC_INTWIDTH - m_len )
	{ check_length(); }

sc_dt::sc_int_base::sc_int_base( int_type v, int w )
	: m_val( v ), m_len( w ), m_ulen( SC_INTWIDTH - m_len )
	{ check_length(); extend_sign(); }

sc_dt::sc_int_base::sc_int_base( const sc_int_base& a )
	: sc_value_base(a), m_val( a.m_val ), m_len( a.m_len ), 
	  m_ulen( a.m_ulen )
	{}

sc_dt::sc_int_base::sc_int_base( const sc_int_subref_r& a )
  : m_val( a ), m_len( a.length() ), m_ulen( SC_INTWIDTH - m_len )
  { extend_sign(); }

sc_dt::sc_int_base::~sc_int_base()
	{}

sc_dt::sc_int_base& sc_dt::sc_int_base::operator = ( int_type v )
	{ m_val = v; extend_sign(); return *this; }

sc_dt::sc_int_base& sc_dt::sc_int_base::operator = ( const sc_int_base& a )
	{ m_val = a.m_val; extend_sign(); return *this; }

sc_dt::sc_int_base& sc_dt::sc_int_base::operator = ( const sc_int_subref_r& a )
  { m_val = a; extend_sign(); return *this; }

sc_dt::sc_int_base& sc_dt::sc_int_base::operator = ( unsigned long a )
	{ m_val = a; extend_sign(); return *this; }

sc_dt::sc_int_base& sc_dt::sc_int_base::operator = ( long a )
	{ m_val = a; extend_sign(); return *this; }

sc_dt::sc_int_base& sc_dt::sc_int_base::operator = ( unsigned int a )
	{ m_val = a; extend_sign(); return *this; }

sc_dt::sc_int_base& sc_dt::sc_int_base::operator = ( int a )
	{ m_val = a; extend_sign(); return *this; }

sc_dt::sc_int_base& sc_dt::sc_int_base::operator = ( uint64 a )
	{ m_val = a; extend_sign(); return *this; }

sc_dt::sc_int_base& sc_dt::sc_int_base::operator = ( double a )
	{ m_val = (int_type) a; extend_sign(); return *this; }

// arithmetic assignment operators

sc_dt::sc_int_base& sc_dt::sc_int_base::operator += ( int_type v )
	{ m_val += v; extend_sign(); return *this; }

sc_dt::sc_int_base& sc_dt::sc_int_base::operator -= ( int_type v )
	{ m_val -= v; extend_sign(); return *this; }

sc_dt::sc_int_base& sc_dt::sc_int_base::operator *= ( int_type v )
	{ m_val *= v; extend_sign(); return *this; }

sc_dt::sc_int_base& sc_dt::sc_int_base::operator /= ( int_type v )
	{ m_val /= v; extend_sign(); return *this; }

sc_dt::sc_int_base& sc_dt::sc_int_base::operator %= ( int_type v )
	{ m_val %= v; extend_sign(); return *this; }

// bitwise assignment operators

sc_dt::sc_int_base& sc_dt::sc_int_base::operator &= ( int_type v )
	{ m_val &= v; extend_sign(); return *this; }

sc_dt::sc_int_base& sc_dt::sc_int_base::operator |= ( int_type v )
	{ m_val |= v; extend_sign(); return *this; }

sc_dt::sc_int_base& sc_dt::sc_int_base::operator ^= ( int_type v )
	{ m_val ^= v; extend_sign(); return *this; }

sc_dt::sc_int_base& sc_dt::sc_int_base::operator <<= ( int_type v )
	{ m_val <<= v; extend_sign(); return *this; }

sc_dt::sc_int_base& sc_dt::sc_int_base::operator >>= ( int_type v )
	{ m_val >>= v; /* no sign extension needed */ return *this; }

// prefix and postfix increment and decrement operators

sc_dt::sc_int_base& sc_dt::sc_int_base::operator ++ () // prefix
	{ ++ m_val; extend_sign(); return *this; }

const sc_dt::sc_int_base sc_dt::sc_int_base::operator ++ ( int ) // postfix
	{ sc_int_base tmp( *this ); ++ m_val; extend_sign(); return tmp; }

sc_dt::sc_int_base& sc_dt::sc_int_base::operator -- () // prefix
	{ -- m_val; extend_sign(); return *this; }

const sc_dt::sc_int_base sc_dt::sc_int_base::operator -- ( int ) // postfix
	{ sc_int_base tmp( *this ); -- m_val; extend_sign(); return tmp; }

// relational operators

bool sc_dt::operator == ( const sc_int_base& a, const sc_int_base& b )
	{ return a.m_val == b.m_val; }

bool sc_dt::operator != ( const sc_int_base& a, const sc_int_base& b )
	{ return a.m_val != b.m_val; }

bool sc_dt::operator <  ( const sc_int_base& a, const sc_int_base& b )
	{ return a.m_val < b.m_val; }

bool sc_dt::operator <= ( const sc_int_base& a, const sc_int_base& b )
	{ return a.m_val <= b.m_val; }

bool sc_dt::operator >  ( const sc_int_base& a, const sc_int_base& b )
	{ return a.m_val > b.m_val; }

bool sc_dt::operator >= ( const sc_int_base& a, const sc_int_base& b )
	{ return a.m_val >= b.m_val; }

bool sc_dt::sc_int_base::test( int i ) const
	{ return ( 0 != (m_val & (UINT_ONE << i)) ); }

void sc_dt::sc_int_base::set( int i )
	{ m_val |= (UINT_ONE << i); }

void sc_dt::sc_int_base::set( int i, bool v )
	{ v ? m_val |= (UINT_ONE << i) : m_val &= ~(UINT_ONE << i); }

// capacity

int sc_dt::sc_int_base::length() const
	{ return m_len; }

int sc_dt::sc_int_base::concat_length(bool* xz_present_p) const
	{ if ( xz_present_p ) *xz_present_p = false; return length(); }

sc_dt::uint64 sc_dt::sc_int_base::concat_get_uint64() const
	{
	    if ( m_len < 64 )
		return (uint64)(m_val & ~((uint_type)-1 << m_len));
	    else
		return (uint64)m_val;
	}

bool sc_dt::sc_int_base::nand_reduce() const
	{ return ( ! and_reduce() ); }

bool sc_dt::sc_int_base::nor_reduce() const
	{ return ( ! or_reduce() ); }

bool sc_dt::sc_int_base::xnor_reduce() const
	{ return ( ! xor_reduce() ); }

sc_dt::sc_int_base::operator int_type() const
	{ return m_val; }

    // explicit conversions

sc_dt::int_type sc_dt::sc_int_base::value() const
	{ return operator int_type(); }

int sc_dt::sc_int_base::to_int() const
	{ return (int) m_val; }

unsigned int sc_dt::sc_int_base::to_uint() const
	{ return (unsigned int) m_val; }

long sc_dt::sc_int_base::to_long() const
	{ return (long) m_val; }

unsigned long sc_dt::sc_int_base::to_ulong() const
	{ return (unsigned long) m_val; }

sc_dt::int64 sc_dt::sc_int_base::to_int64() const
	{ return (int64) m_val; }

sc_dt::uint64 sc_dt::sc_int_base::to_uint64() const
	{ return (uint64) m_val; }

double sc_dt::sc_int_base::to_double() const
	{ return (double) m_val; }

void sc_dt::sc_int_base::print( ::std::ostream& os) const
	{ os << to_string(sc_io_base(os,SC_DEC),sc_io_show_base(os)); }


// ----------------------------------------------------------------------------
//  CLASS : sc_int_bitref_r
//
//  Proxy class for sc_int bit selection (r-value only).
// ----------------------------------------------------------------------------

// implicit conversion to uint64

sc_dt::sc_int_bitref_r::operator uint64 () const
{
    return m_obj_p->test( m_index );
}

bool
sc_dt::sc_int_bitref_r::operator ! () const
{
    return ! m_obj_p->test( m_index );
}

bool
sc_dt::sc_int_bitref_r::operator ~ () const
{
    return ! m_obj_p->test( m_index );
}

::std::ostream&
sc_dt::operator << ( ::std::ostream& os, const sc_int_bitref_r& a )
{
    a.print( os );
    return os;
}
// ----------------------------------------------------------------------------
//  CLASS : sc_int_bitref
//
//  Proxy class for sc_int bit selection (r-value and l-value).
// ----------------------------------------------------------------------------

// assignment operators

sc_dt::sc_int_bitref&
sc_dt::sc_int_bitref::operator = ( const sc_int_bitref_r& b )
{
    m_obj_p->set( m_index, (bool) b );
    m_obj_p->extend_sign();
    return *this;
}


sc_dt::sc_int_bitref&
sc_dt::sc_int_bitref::operator = ( const sc_int_bitref& b )
{
    m_obj_p->set( m_index, (bool) b );
    m_obj_p->extend_sign();
    return *this;
}


sc_dt::sc_int_bitref&
sc_dt::sc_int_bitref::operator = ( bool b )
{
    m_obj_p->set( m_index, b );
    m_obj_p->extend_sign();
    return *this;
}



sc_dt::sc_int_bitref&
sc_dt::sc_int_bitref::operator &= ( bool b )
{
    if( ! b ) {
	m_obj_p->set( m_index, b );
	m_obj_p->extend_sign();
    }
    return *this;
}


sc_dt::sc_int_bitref&
sc_dt::sc_int_bitref::operator |= ( bool b )
{
    if( b ) {
	m_obj_p->set( m_index, b );
	m_obj_p->extend_sign();
    }
    return *this;
}

sc_dt::sc_int_bitref&
sc_dt::sc_int_bitref::operator ^= ( bool b )
{
    if( b ) {
	m_obj_p->m_val ^= (UINT_ONE << m_index);
	m_obj_p->extend_sign();
    }
    return *this;
}

::std::istream&
sc_dt::operator >> ( ::std::istream& is, sc_int_bitref& a )
{
    a.scan( is );
    return is;
}
// ----------------------------------------------------------------------------
//  CLASS : sc_int_subref_r
//
//  Proxy class for sc_int part selection (r-value only).
// ----------------------------------------------------------------------------

// implicit conversion to int_type

sc_dt::sc_int_subref_r::operator uint_type() const
{
    uint_type /*int_type*/ val = m_obj_p->m_val;
    int uleft = SC_INTWIDTH - (m_left + 1);
    int uright = uleft + m_right;
    return ( val << uleft >> uright );
}

// reduce methods

bool
sc_dt::sc_int_subref_r::and_reduce() const
{
    sc_int_base a( *this );
    return a.and_reduce();
}


bool
sc_dt::sc_int_subref_r::or_reduce() const
{
    sc_int_base a( *this );
    return a.or_reduce();
}


bool
sc_dt::sc_int_subref_r::xor_reduce() const
{
    sc_int_base a( *this );
    return a.xor_reduce();
}


// explicit conversions


int
sc_dt::sc_int_subref_r::to_int() const
{
	int result = static_cast<int>(operator uint_type());
	return result;
}


unsigned int
sc_dt::sc_int_subref_r::to_uint() const
{
	unsigned int result = static_cast<unsigned int>(operator uint_type());
	return result;
}


long
sc_dt::sc_int_subref_r::to_long() const
{
	long result = static_cast<long>(operator uint_type());
	return result;
}


unsigned long
sc_dt::sc_int_subref_r::to_ulong() const
{
	unsigned long result = static_cast<unsigned long>(operator uint_type());
	return result;
}


sc_dt::int64
sc_dt::sc_int_subref_r::to_int64() const
{
	int64 result = operator uint_type();
	return result;
}


sc_dt::uint64
sc_dt::sc_int_subref_r::to_uint64() const
{
	uint64 result = operator uint_type();
	return result;
}


double
sc_dt::sc_int_subref_r::to_double() const
{
	double result = static_cast<double>(operator uint_type());
	return result;
}

// explicit conversion to character string

const std::string
sc_dt::sc_int_subref_r::to_string( sc_numrep numrep ) const
{
	sc_uint_base a(length());
    a = operator uint_type();
    return a.to_string( numrep );
}


const std::string
sc_dt::sc_int_subref_r::to_string( sc_numrep numrep, bool w_prefix ) const
{
	sc_uint_base a(length());
    a = operator uint_type();
    return a.to_string( numrep, w_prefix );
}

// functional notation for the reduce methods


bool
sc_dt::and_reduce( const sc_int_subref_r& a )
{
    return a.and_reduce();
}


bool
sc_dt::nand_reduce( const sc_int_subref_r& a )
{
    return a.nand_reduce();
}


bool
sc_dt::or_reduce( const sc_int_subref_r& a )
{
    return a.or_reduce();
}


bool
sc_dt::nor_reduce( const sc_int_subref_r& a )
{
    return a.nor_reduce();
}


bool
sc_dt::xor_reduce( const sc_int_subref_r& a )
{
    return a.xor_reduce();
}


bool
sc_dt::xnor_reduce( const sc_int_subref_r& a )
{
    return a.xnor_reduce();
}

::std::ostream&
sc_dt::operator << ( ::std::ostream& os, const sc_int_subref_r& a )
{
    a.print( os );
    return os;
}
// ----------------------------------------------------------------------------
//  CLASS : sc_int_subref
//
//  Proxy class for sc_int part selection (r-value and l-value).
// ----------------------------------------------------------------------------

// assignment operators


sc_dt::sc_int_subref&
sc_dt::sc_int_subref::operator = ( const sc_int_base& a )
{
    return operator = ( a.operator int_type() );
}


sc_dt::sc_int_subref&
sc_dt::sc_int_subref::operator = ( const char* a )
{
    sc_int_base aa( length() );
    return ( *this = aa = a );
}

::std::istream&
sc_dt::operator >> ( ::std::istream& is, sc_int_subref& a )
{
    a.scan( is );
    return is;
}
// ----------------------------------------------------------------------------
//  CLASS : sc_int_base
//
//  Base class for sc_int.
// ----------------------------------------------------------------------------
// bit selection


sc_dt::sc_int_bitref&
sc_dt::sc_int_base::operator [] ( int i )
{
    check_index( i );
    sc_int_bitref* result_p = sc_int_bitref::m_pool.allocate();
    result_p->initialize(this, i);
    return *result_p;
}


const sc_dt::sc_int_bitref_r&
sc_dt::sc_int_base::operator [] ( int i ) const
{
    check_index( i );
    sc_int_bitref* result_p = sc_int_bitref::m_pool.allocate();
    result_p->initialize(this, i);
    return *result_p;
}



sc_dt::sc_int_bitref&
sc_dt::sc_int_base::bit( int i )
{
    check_index( i );
    sc_int_bitref* result_p = sc_int_bitref::m_pool.allocate();
    result_p->initialize(this, i);
    return *result_p;
}


const sc_dt::sc_int_bitref_r&
sc_dt::sc_int_base::bit( int i ) const
{
    check_index( i );
    sc_int_bitref* result_p = sc_int_bitref::m_pool.allocate();
    result_p->initialize(this, i);
    return *result_p;
}


// part selection


sc_dt::sc_int_subref&
sc_dt::sc_int_base::operator () ( int left, int right )
{
    check_range( left, right );
    sc_int_subref* result_p = sc_int_subref::m_pool.allocate();
    result_p->initialize(this, left, right);
    return *result_p;
}


const sc_dt::sc_int_subref_r&
sc_dt::sc_int_base::operator () ( int left, int right ) const
{
    check_range( left, right );
    sc_int_subref* result_p = sc_int_subref::m_pool.allocate();
    result_p->initialize(this, left, right);
    return *result_p;
}



sc_dt::sc_int_subref&
sc_dt::sc_int_base::range( int left, int right )
{
    check_range( left, right );
    sc_int_subref* result_p = sc_int_subref::m_pool.allocate();
    result_p->initialize(this, left, right);
    return *result_p;
}


const sc_dt::sc_int_subref_r&
sc_dt::sc_int_base::range( int left, int right ) const
{
    check_range( left, right );
    sc_int_subref* result_p = sc_int_subref::m_pool.allocate();
    result_p->initialize(this, left, right);
    return *result_p;
}



bool
sc_dt::and_reduce( const sc_int_base& a )
{
    return a.and_reduce();
}


bool
sc_dt::nand_reduce( const sc_int_base& a )
{
    return a.nand_reduce();
}


bool
sc_dt::or_reduce( const sc_int_base& a )
{
    return a.or_reduce();
}


bool
sc_dt::nor_reduce( const sc_int_base& a )
{
    return a.nor_reduce();
}


bool
sc_dt::xor_reduce( const sc_int_base& a )
{
    return a.xor_reduce();
}


bool
sc_dt::xnor_reduce( const sc_int_base& a )
{
    return a.xnor_reduce();
}

::std::istream&
sc_dt::operator >> ( ::std::istream& is, sc_int_base& a )
{
    a.scan( is );
    return is;
}


//-----------------------------------Farah is done working here
namespace sc_dt
{

// to avoid code bloat in sc_int_concref<T1,T2>

void
sc_int_concref_invalid_length( int length )
{
    char msg[BUFSIZ];
    std::sprintf( msg,
	     "sc_int_concref<T1,T2> initialization: length = %d "
	     "violates 1 <= length <= %d",
	     length, SC_INTWIDTH );
    SC_REPORT_ERROR( sc_core::SC_ID_OUT_OF_BOUNDS_, msg );
}


// ----------------------------------------------------------------------------
//  CLASS : sc_int_bitref
//
//  Proxy class for sc_int bit selection (r-value and l-value).
// ----------------------------------------------------------------------------

sc_core::sc_vpool<sc_int_bitref> sc_int_bitref::m_pool(9);

// concatenation methods:

// #### OPTIMIZE
void sc_int_bitref::concat_set(int64 src, int low_i)
{
    sc_int_base aa( 1 );
    *this = aa = (low_i < 64) ? src >> low_i : src >> 63;
}

void sc_int_bitref::concat_set(const sc_signed& src, int low_i)
{
    sc_int_base aa( 1 );
    if ( low_i < src.length() )
	*this = aa = 1 & (src >> low_i);
    else
	*this = aa = (src < 0) ? (int_type)-1 : 0;
}

void sc_int_bitref::concat_set(const sc_unsigned& src, int low_i)
{
    sc_int_base aa( 1 );
    if ( low_i < src.length() )
	*this = aa = 1 & (src >> low_i);
    else
	*this = aa = 0;
}

void sc_int_bitref::concat_set(uint64 src, int low_i)
{
    sc_int_base aa( 1 );
    *this = aa = (low_i < 64) ? src >> low_i : 0;
}


// other methods

void
sc_int_bitref::scan( ::std::istream& is )
{
    bool b;
    is >> b;
    *this = b;
}


// ----------------------------------------------------------------------------
//  CLASS : sc_int_subref_r
//
//  Proxy class for sc_int part selection (l-value).
// ----------------------------------------------------------------------------

bool sc_int_subref_r::concat_get_ctrl( sc_digit* dst_p, int low_i ) const
{    
    int       dst_i;       // Word in dst_p now processing.
    int       end_i;       // Highest order word in dst_p to process.
    int       high_i;      // Index of high order bit in dst_p to set.
    uint_type mask;        // Mask for bits to extract or keep.

    dst_i = low_i / BITS_PER_DIGIT;
    high_i = low_i + (m_left-m_right);
    end_i = high_i / BITS_PER_DIGIT;
    mask = ~mask_int[m_left][m_right];


    // PROCESS THE FIRST WORD:

    dst_p[dst_i] = (sc_digit)(dst_p[dst_i] & mask);
    switch ( end_i - dst_i )
    {
     // BITS ARE ACROSS TWO WORDS:

     case 1:
        dst_i++;
        dst_p[dst_i] = 0;
        break;

     // BITS ARE ACROSS THREE WORDS:

     case 2:
        dst_i++;
        dst_p[dst_i++] = 0;
        dst_p[dst_i] = 0;
        break;

     // BITS ARE ACROSS FOUR WORDS:

     case 3:
        dst_i++;
        dst_p[dst_i++] = 0;
        dst_p[dst_i++] = 0;
        dst_p[dst_i] = 0;
        break;
    }
    return false;
}


bool sc_int_subref_r::concat_get_data( sc_digit* dst_p, int low_i ) const
{    
    int       dst_i;      // Word in dst_p now processing.
    int       end_i;      // Highest order word in dst_p to process.
    int       high_i;     // Index of high order bit in dst_p to set.
    int       left_shift; // Left shift for val.
    uint_type mask;       // Mask for bits to extract or keep.
    bool      non_zero;	  // True if value inserted is non-zero.
    uint_type val;        // Selection value extracted from m_obj_p.

    dst_i = low_i / BITS_PER_DIGIT;
    left_shift = low_i % BITS_PER_DIGIT;
    high_i = low_i + (m_left-m_right);
    end_i = high_i / BITS_PER_DIGIT;
    mask = ~mask_int[m_left][m_right];
    val = (m_obj_p->m_val & mask) >> m_right;
    non_zero = val != 0;


    // PROCESS THE FIRST WORD:

    mask = ~(-1 << left_shift);
    dst_p[dst_i] = (sc_digit)((dst_p[dst_i] & mask) | 
		((val << left_shift) & DIGIT_MASK));

    switch ( end_i - dst_i )
    {
     // BITS ARE ACROSS TWO WORDS:

     case 1:
        dst_i++;
        val >>= (BITS_PER_DIGIT-left_shift);
        dst_p[dst_i] = (sc_digit)(val & DIGIT_MASK);
        break;

     // BITS ARE ACROSS THREE WORDS:

     case 2:
        dst_i++;
        val >>= (BITS_PER_DIGIT-left_shift);
        dst_p[dst_i++] = (sc_digit)(val & DIGIT_MASK);
        val >>= BITS_PER_DIGIT;
        dst_p[dst_i] = (sc_digit)val;
        break;

     // BITS ARE ACROSS FOUR WORDS:

     case 3:
        dst_i++;
        val >>= (BITS_PER_DIGIT-left_shift);
        dst_p[dst_i++] = (sc_digit)(val & DIGIT_MASK);
        val >>= BITS_PER_DIGIT;
        dst_p[dst_i++] = (sc_digit)(val & DIGIT_MASK);
        val >>= BITS_PER_DIGIT;
        dst_p[dst_i] = (sc_digit)val;
        break;
    }
    return non_zero;
}

// ----------------------------------------------------------------------------
//  CLASS : sc_int_subref
//
//  Proxy class for sc_int part selection (r-value and l-value).
// ----------------------------------------------------------------------------

sc_core::sc_vpool<sc_int_subref> sc_int_subref::m_pool(9);

// assignment operators
  
sc_int_subref& 
sc_int_subref::operator = ( int_type v )
{
    int_type val = m_obj_p->m_val;
    uint_type mask = mask_int[m_left][m_right];
    val &= mask;
    val |= (v << m_right) & ~mask;
    m_obj_p->m_val = val;
    m_obj_p->extend_sign();
    return *this;
}

sc_int_subref&
sc_int_subref::operator = ( const sc_signed& a )
{
    sc_int_base aa( length() );
    return ( *this = aa = a );
}

sc_int_subref&
sc_int_subref::operator = ( const sc_unsigned& a )
{
    sc_int_base aa( length() );
    return ( *this = aa = a );
}

sc_int_subref&
sc_int_subref::operator = ( const sc_bv_base& a )
{
    sc_int_base aa( length() );
    return ( *this = aa = a );
}

sc_int_subref&
sc_int_subref::operator = ( const sc_lv_base& a )
{
    sc_int_base aa( length() );
    return ( *this = aa = a );
}


// concatenation methods:

// #### OPTIMIZE
void sc_int_subref::concat_set(int64 src, int low_i)
{
    sc_int_base aa ( length() );
    *this = aa = (low_i < 64) ? src >> low_i : src >> 63;
}

void sc_int_subref::concat_set(const sc_signed& src, int low_i)
{
    sc_int_base aa( length() );
    if ( low_i < src.length() )
	*this = aa = src >> low_i;
    else
	*this = (src < 0) ? (int_type)-1 : 0;
}

void sc_int_subref::concat_set(const sc_unsigned& src, int low_i)
{
    sc_int_base aa( length() );
    if ( low_i < src.length() )
	*this = aa = src >> low_i;
    else
	*this = 0;
}

void sc_int_subref::concat_set(uint64 src, int low_i)
{
    sc_int_base aa ( length() );
    *this = aa = (low_i < 64) ? src >> low_i : 0;
}


// other methods

void
sc_int_subref::scan( ::std::istream& is )
{
    std::string s;
    is >> s;
    *this = s.c_str();
}


// ----------------------------------------------------------------------------
//  CLASS : sc_int_base
//
//  Base class for sc_int.
// ----------------------------------------------------------------------------

// support methods

void
sc_int_base::invalid_length() const
{
    char msg[BUFSIZ];
    std::sprintf( msg,
	     "sc_int[_base] initialization: length = %d violates "
	     "1 <= length <= %d",
	     m_len, SC_INTWIDTH );
    SC_REPORT_ERROR( sc_core::SC_ID_OUT_OF_BOUNDS_, msg );
}

void
sc_int_base::invalid_index( int i ) const
{
    char msg[BUFSIZ];
    std::sprintf( msg,
	     "sc_int[_base] bit selection: index = %d violates "
	     "0 <= index <= %d",
	     i, m_len - 1 );
    SC_REPORT_ERROR( sc_core::SC_ID_OUT_OF_BOUNDS_, msg );
}

void
sc_int_base::invalid_range( int l, int r ) const
{
    char msg[BUFSIZ];
    std::sprintf( msg,
	     "sc_int[_base] part selection: left = %d, right = %d violates "
	     "%d >= left >= right >= 0",
	     l, r, m_len - 1 );
    SC_REPORT_ERROR( sc_core::SC_ID_OUT_OF_BOUNDS_, msg );
}


void
sc_int_base::check_value() const
{
    int_type limit = (int_type) 1 << ( m_len - 1 );
    if( m_val < -limit || m_val >= limit ) {
	char msg[BUFSIZ];
	std::sprintf( msg, "sc_int[_base]: value does not fit into a length of %d",
		 m_len );
	SC_REPORT_WARNING( sc_core::SC_ID_OUT_OF_BOUNDS_, msg );
    }
}


// constructors
sc_int_base::sc_int_base( const sc_bv_base& v ) 
    : m_val(0), m_len( v.length() ), m_ulen( SC_INTWIDTH - m_len )
{
    check_length();
    *this = v;
}
sc_int_base::sc_int_base( const sc_lv_base& v )
    : m_val(0), m_len( v.length() ), m_ulen( SC_INTWIDTH - m_len )
{
    check_length();
    *this = v;
}
sc_int_base::sc_int_base( const sc_uint_subref_r& v )
    : m_val(0), m_len( v.length() ), m_ulen( SC_INTWIDTH - m_len )
{
    check_length();
    *this = v.to_uint64();
}
sc_int_base::sc_int_base( const sc_signed_subref_r& v )
    : m_val(0), m_len( v.length() ), m_ulen( SC_INTWIDTH - m_len )
{
    check_length();
    *this = v.to_uint64();
}
sc_int_base::sc_int_base( const sc_unsigned_subref_r& v )
    : m_val(0), m_len( v.length() ), m_ulen( SC_INTWIDTH - m_len )
{
    check_length();
    *this = v.to_uint64();
}

sc_int_base::sc_int_base( const sc_signed& a )
    : m_val( 0 ), m_len( a.length() ), m_ulen( SC_INTWIDTH - m_len )
{
    check_length();
#if 0
    for( int i = m_len - 1; i >= 0; -- i ) {
	set( i, a.test( i ) );
    }
    extend_sign();
#else
    *this = a.to_int64();
#endif
}

sc_int_base::sc_int_base( const sc_unsigned& a )
    : m_val( 0 ), m_len( a.length() ), m_ulen( SC_INTWIDTH - m_len )
{
    check_length();
#if 0
    for( int i = m_len - 1; i >= 0; -- i ) {
	set( i, a.test( i ) );
    }
    extend_sign();
#else
    *this = a.to_int64();
#endif
}


// assignment operators

sc_int_base& 
sc_int_base::operator = ( const sc_signed& a )
{
    int minlen = sc_min( m_len, a.length() );
    int i = 0;
    for( ; i < minlen; ++ i ) {
	set( i, a.test( i ) );
    }
    bool sgn = a.sign();
    for( ; i < m_len; ++ i ) {
	// sign extension
	set( i, sgn );
    }
    extend_sign();
    return *this;
}

sc_int_base& 
sc_int_base::operator = ( const sc_unsigned& a )
{
    int minlen = sc_min( m_len, a.length() );
    int i = 0;
    for( ; i < minlen; ++ i ) {
	set( i, a.test( i ) );
    }
    for( ; i < m_len; ++ i ) {
	// zero extension
	set( i, 0 );
    }
    extend_sign();
    return *this;
}


sc_int_base&
sc_int_base::operator = ( const sc_bv_base& a )
{
    int minlen = sc_min( m_len, a.length() );
    int i = 0;
    for( ; i < minlen; ++ i ) {
	set( i, a.get_bit( i ) );
    }
    for( ; i < m_len; ++ i ) {
	// zero extension
	set( i, 0 );
    }
    extend_sign();
    return *this;
}

sc_int_base&
sc_int_base::operator = ( const sc_lv_base& a )
{
    int minlen = sc_min( m_len, a.length() );
    int i = 0;
    for( ; i < minlen; ++ i ) {
	set( i, sc_logic( a.get_bit( i ) ).to_bool() );
    }
    for( ; i < m_len; ++ i ) {
	// zero extension
	set( i, 0 );
    }
    extend_sign();
    return *this;
}

sc_int_base&
sc_int_base::operator = ( const char* a )
{
    if( a == 0 ) {
	SC_REPORT_ERROR( sc_core::SC_ID_CONVERSION_FAILED_,
			 "character string is zero" );
    }
    if( *a == 0 ) {
	SC_REPORT_ERROR( sc_core::SC_ID_CONVERSION_FAILED_,
			 "character string is empty" );
    }
    try {
	int len = m_len;
	sc_fix aa( a, len, len, SC_TRN, SC_WRAP, 0, SC_ON );
	return this->operator = ( aa );
    } catch( sc_core::sc_report ) {
	char msg[BUFSIZ];
	std::sprintf( msg, "character string '%s' is not valid", a );
	SC_REPORT_ERROR( sc_core::SC_ID_CONVERSION_FAILED_, msg );
	// never reached
	return *this;
    }
}

// explicit conversion to character string

const std::string
sc_int_base::to_string( sc_numrep numrep ) const
{
    int len = m_len;
    sc_fix aa( *this, len, len, SC_TRN, SC_WRAP, 0, SC_ON );
    return aa.to_string( numrep );
}

const std::string
sc_int_base::to_string( sc_numrep numrep, bool w_prefix ) const
{
    int len = m_len;
    sc_fix aa( *this, len, len, SC_TRN, SC_WRAP, 0, SC_ON );
    return aa.to_string( numrep, w_prefix );
}


// reduce methods

bool
sc_int_base::and_reduce() const
{
    return ( m_val == int_type( -1 ) );
}

bool
sc_int_base::or_reduce() const
{
    return ( m_val != int_type( 0 ) );
}

bool
sc_int_base::xor_reduce() const
{
    uint_type mask = ~UINT_ZERO;
    uint_type val = m_val & (mask >> m_ulen);
    int n = SC_INTWIDTH;
    do {
	n >>= 1;
	mask >>= n;
	val = ((val & (mask << n)) >> n) ^ (val & mask);
    } while( n != 1 );
    return ( val != uint_type( 0 ) );
}


bool sc_int_base::concat_get_ctrl( sc_digit* dst_p, int low_i ) const
{    
    int        dst_i;       // Word in dst_p now processing.
    int        end_i;       // Highest order word in dst_p to process.
    int        left_shift;  // Left shift for val.
    uint_type  mask;        // Mask for bits to extract or keep.

    dst_i = low_i / BITS_PER_DIGIT;
    left_shift = low_i % BITS_PER_DIGIT;
    end_i = (low_i + (m_len-1)) / BITS_PER_DIGIT;

    mask = ~(-1 << left_shift);
    dst_p[dst_i] = (sc_digit)(dst_p[dst_i] & mask);
	dst_i++;
	for ( ; dst_i <= end_i; dst_i++ ) dst_p[dst_i] = 0;
	return false;
}

//------------------------------------------------------------------------------
//"sc_int_base::concat_get_data"
//
// This method transfers the value of this object instance to the supplied
// array of sc_unsigned digits starting with the bit specified by low_i within
// the array of digits.
//
// Notes:
//   (1) we don't worry about masking the high order data we transfer since
//       concat_get_data() is called from low order bit to high order bit. So
//       the bits above where we place ours will be filled in by someone else.
//
//   dst_p -> array of sc_unsigned digits to be filled in.
//   low_i =  first bit within dst_p to be set.
//------------------------------------------------------------------------------
bool sc_int_base::concat_get_data( sc_digit* dst_p, int low_i ) const
{    
    int        dst_i;       // Word in dst_p now processing.
    int        end_i;       // Highest order word in dst_p to process.
    int        high_i;      // Index of high order bit in dst_p to set.
    int        left_shift;  // Left shift for val.
    uint_type  mask;        // Mask for bits to extract or keep.
    bool       non_zero;    // True if value inserted is non-zero.
    uint_type  val;         // Value for this object.

    dst_i = low_i / BITS_PER_DIGIT;
    left_shift = low_i % BITS_PER_DIGIT;
    high_i = low_i + (m_len-1);
    end_i = high_i / BITS_PER_DIGIT;
    val = m_val;
    non_zero = val != 0;

    // MASK OFF DATA TO BE TRANSFERRED BASED ON WIDTH:

    if ( m_len < 64 )
    {
	mask = ~((uint_type)-1 << m_len);
        val &=  mask;
    }

    // PROCESS THE FIRST WORD:

    mask = (-1 << left_shift);
    dst_p[dst_i] = (sc_digit)((dst_p[dst_i] & ~mask) | 
		((val <<left_shift) & DIGIT_MASK));
    switch ( end_i - dst_i )
    {
     // BITS ARE ACROSS TWO WORDS:

     case 1:
	dst_i++;
	val >>= (BITS_PER_DIGIT-left_shift);
	dst_p[dst_i] = (sc_digit)val;
	break;

     // BITS ARE ACROSS THREE WORDS:

     case 2:
	dst_i++;
	val >>= (BITS_PER_DIGIT-left_shift);
	dst_p[dst_i++] = ((sc_digit)val) & DIGIT_MASK;
	val >>= BITS_PER_DIGIT;
	dst_p[dst_i] = (sc_digit)val;
	break;

     // BITS ARE ACROSS FOUR WORDS:

     case 3:
	dst_i++;
	val >>= (BITS_PER_DIGIT-left_shift);
	dst_p[dst_i++] = (sc_digit)(val & DIGIT_MASK);
	val >>= BITS_PER_DIGIT;
	dst_p[dst_i++] = (sc_digit)(val & DIGIT_MASK);
	val >>= BITS_PER_DIGIT;
	dst_p[dst_i] = (sc_digit)val;
	break;
    }
    return non_zero;
}

// #### OPTIMIZE
void sc_int_base::concat_set(int64 src, int low_i)
{
    *this = (low_i < 64) ? src >> low_i : src >> 63;
}

void sc_int_base::concat_set(const sc_signed& src, int low_i)
{
    if ( low_i < src.length() )
	*this = src >> low_i;
    else
        *this = (src < 0) ? (int_type)-1 : 0;
}

void sc_int_base::concat_set(const sc_unsigned& src, int low_i)
{
    if ( low_i < src.length() )
	*this = src >> low_i;
    else
        *this = 0;
}

void sc_int_base::concat_set(uint64 src, int low_i)
{
    *this = (low_i < 64) ? src >> low_i : 0;
}

// other methods

void
sc_int_base::scan( ::std::istream& is )
{
    std::string s;
    is >> s;
    *this = s.c_str();
}

} // namespace sc_dt;


// Taf!
