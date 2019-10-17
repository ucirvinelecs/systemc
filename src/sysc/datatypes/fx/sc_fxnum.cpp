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

  sc_fxnum.cpp - 

  Original Author: Martin Janssen, Synopsys, Inc.

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date:
  Description of Modification:

 *****************************************************************************/


// $Log: sc_fxnum.cpp,v $
// Revision 1.3  2011/01/19 18:57:40  acg
//  Andy Goodrich: changes for IEEE_1666_2011.
//
// Revision 1.2  2010/12/07 20:09:08  acg
// Andy Goodrich: Philipp Hartmann's constructor disambiguation fix
//
// Revision 1.1.1.1  2006/12/15 20:20:04  acg
// SystemC 2.3
//
// Revision 1.3  2006/01/13 18:53:57  acg
// Andy Goodrich: added $Log command so that CVS comments are reproduced in
// the source.
//

#include <math.h>

#include "sysc/datatypes/fx/sc_fxnum.h"
//--------------------------------------------------Farah is working here 
// ----------------------------------------------------------------------------
//  CLASS : sc_fxnum_bitref
//
//  Proxy class for bit-selection in class sc_fxnum, behaves like sc_bit.
// ----------------------------------------------------------------------------
sc_dt::sc_fxnum_bitref::sc_fxnum_bitref( sc_fxnum& num_, int idx_ )
	: m_num( num_ ), m_idx( idx_ )
{}

sc_dt::sc_fxnum_bitref::sc_fxnum_bitref( const sc_fxnum_bitref& a )
	: m_num( a.m_num ), m_idx( a.m_idx )
{}

sc_dt::sc_fxnum_bitref&
sc_dt::sc_fxnum_bitref::operator = ( const sc_fxnum_bitref& a )
{
    if( &a != this )
    {
	SC_FXNUM_OBSERVER_READ_( a.m_num )
	set( a.get() );
	SC_FXNUM_OBSERVER_WRITE_( m_num )
    }
    return *this;
}

sc_dt::sc_fxnum_bitref&
sc_dt::sc_fxnum_bitref::operator = ( const sc_fxnum_fast_bitref& a )
{
    SC_FXNUM_FAST_OBSERVER_READ_( a.m_num )
    set( a.get() );
    SC_FXNUM_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_bitref&
sc_dt::sc_fxnum_bitref::operator = ( const sc_bit& a )
{
    set( static_cast<bool>( a ) );
    SC_FXNUM_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_bitref&
sc_dt::sc_fxnum_bitref::operator = ( bool a )
{
    set( a );
    SC_FXNUM_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_bitref&
sc_dt::sc_fxnum_bitref::operator &= ( const sc_fxnum_bitref& b )
{
    SC_FXNUM_OBSERVER_READ_( m_num )
    SC_FXNUM_OBSERVER_READ_( b.m_num )
    set( get() && b.get() );
    SC_FXNUM_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_bitref&
sc_dt::sc_fxnum_bitref::operator &= ( const sc_fxnum_fast_bitref& b )
{
    SC_FXNUM_OBSERVER_READ_( m_num )
    SC_FXNUM_FAST_OBSERVER_READ_( b.m_num )
    set( get() && b.get() );
    SC_FXNUM_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_bitref&
sc_dt::sc_fxnum_bitref::operator &= ( const sc_bit& b )
{
    SC_FXNUM_OBSERVER_READ_( m_num )
    set( get() && static_cast<bool>( b ) );
    SC_FXNUM_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_bitref&
sc_dt::sc_fxnum_bitref::operator &= ( bool b )
{
    SC_FXNUM_OBSERVER_READ_( m_num )
    set( get() && b );
    SC_FXNUM_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_bitref&
sc_dt::sc_fxnum_bitref::operator |= ( const sc_fxnum_bitref& b )
{
    SC_FXNUM_OBSERVER_READ_( m_num )
    SC_FXNUM_OBSERVER_READ_( b.m_num )
    set( get() || b.get() );
    SC_FXNUM_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_bitref&
sc_dt::sc_fxnum_bitref::operator |= ( const sc_fxnum_fast_bitref& b )
{
    SC_FXNUM_OBSERVER_READ_( m_num )
    SC_FXNUM_FAST_OBSERVER_READ_( b.m_num )
    set( get() || b.get() );
    SC_FXNUM_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_bitref&
sc_dt::sc_fxnum_bitref::operator |= ( const sc_bit& b )
{
    SC_FXNUM_OBSERVER_READ_( m_num )
    set( get() || static_cast<bool>( b ) );
    SC_FXNUM_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_bitref&
sc_dt::sc_fxnum_bitref::operator |= ( bool b )
{
    SC_FXNUM_OBSERVER_READ_( m_num )
    set( get() || b );
    SC_FXNUM_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_bitref&
sc_dt::sc_fxnum_bitref::operator ^= ( const sc_fxnum_bitref& b )
{
    SC_FXNUM_OBSERVER_READ_( m_num )
    SC_FXNUM_OBSERVER_READ_( b.m_num )
    set( get() != b.get() );
    SC_FXNUM_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_bitref&
sc_dt::sc_fxnum_bitref::operator ^= ( const sc_fxnum_fast_bitref& b )
{
    SC_FXNUM_OBSERVER_READ_( m_num )
    SC_FXNUM_FAST_OBSERVER_READ_( b.m_num )
    set( get() != b.get() );
    SC_FXNUM_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_bitref&
sc_dt::sc_fxnum_bitref::operator ^= ( const sc_bit& b )
{
    SC_FXNUM_OBSERVER_READ_( m_num )
    set( get() != static_cast<bool>( b ) );
    SC_FXNUM_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_bitref&
sc_dt::sc_fxnum_bitref::operator ^= ( bool b )
{
    SC_FXNUM_OBSERVER_READ_( m_num )
    set( get() != b );
    SC_FXNUM_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_bitref::operator bool() const
{
    SC_FXNUM_OBSERVER_READ_( m_num )
    return get();
}

::std::ostream&
sc_dt::operator << ( ::std::ostream& os, const sc_fxnum_bitref& a )
{
    a.print( os );
    return os;
}

::std::istream&
sc_dt::operator >> ( ::std::istream& is, sc_fxnum_bitref& a )
{
    a.scan( is );
    return is;
}

// ----------------------------------------------------------------------------
//  CLASS : sc_fxnum_fast_bitref
//
//  Proxy class for bit-selection in class sc_fxnum_fast, behaves like sc_bit.
// ----------------------------------------------------------------------------


sc_dt::sc_fxnum_fast_bitref::sc_fxnum_fast_bitref( sc_fxnum_fast& num_, int idx_ )
    : m_num( num_ ), m_idx( idx_ )
{}

sc_dt::sc_fxnum_fast_bitref::sc_fxnum_fast_bitref( const sc_fxnum_fast_bitref& a )
    : m_num( a.m_num ), m_idx( a.m_idx )
{}

sc_dt::sc_fxnum_fast_bitref&
sc_dt::sc_fxnum_fast_bitref::operator = ( const sc_fxnum_bitref& a )
{
    SC_FXNUM_OBSERVER_READ_( a.m_num )
    set( a.get() );
    SC_FXNUM_FAST_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_fast_bitref&
sc_dt::sc_fxnum_fast_bitref::operator = ( const sc_fxnum_fast_bitref& a )
{
    if( &a != this )
    {
	SC_FXNUM_FAST_OBSERVER_READ_( a.m_num )
	set( a.get() );
	SC_FXNUM_FAST_OBSERVER_WRITE_( m_num )
    }
    return *this;
}

sc_dt::sc_fxnum_fast_bitref&
sc_dt::sc_fxnum_fast_bitref::operator = ( const sc_bit& a )
{
    set( static_cast<bool>( a ) );
    SC_FXNUM_FAST_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_fast_bitref&
sc_dt::sc_fxnum_fast_bitref::operator = ( bool a )
{
    set( a );
    SC_FXNUM_FAST_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_fast_bitref&
sc_dt::sc_fxnum_fast_bitref::operator &= ( const sc_fxnum_bitref& b )
{
    SC_FXNUM_FAST_OBSERVER_READ_( m_num )
    SC_FXNUM_OBSERVER_READ_( b.m_num )
    set( get() && b.get() );
    SC_FXNUM_FAST_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_fast_bitref&
sc_dt::sc_fxnum_fast_bitref::operator &= ( const sc_fxnum_fast_bitref& b )
{
    SC_FXNUM_FAST_OBSERVER_READ_( m_num )
    SC_FXNUM_FAST_OBSERVER_READ_( b.m_num )
    set( get() && b.get() );
    SC_FXNUM_FAST_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_fast_bitref&
sc_dt::sc_fxnum_fast_bitref::operator &= ( const sc_bit& b )
{
    SC_FXNUM_FAST_OBSERVER_READ_( m_num )
    set( get() && static_cast<bool>( b ) );
    SC_FXNUM_FAST_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_fast_bitref&
sc_dt::sc_fxnum_fast_bitref::operator &= ( bool b )
{
    SC_FXNUM_FAST_OBSERVER_READ_( m_num )
    set( get() && b );
    SC_FXNUM_FAST_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_fast_bitref&
sc_dt::sc_fxnum_fast_bitref::operator |= ( const sc_fxnum_bitref& b )
{
    SC_FXNUM_FAST_OBSERVER_READ_( m_num )
    SC_FXNUM_OBSERVER_READ_( b.m_num )
    set( get() || b.get() );
    SC_FXNUM_FAST_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_fast_bitref&
sc_dt::sc_fxnum_fast_bitref::operator |= ( const sc_fxnum_fast_bitref& b )
{
    SC_FXNUM_FAST_OBSERVER_READ_( m_num )
    SC_FXNUM_FAST_OBSERVER_READ_( b.m_num )
    set( get() || b.get() );
    SC_FXNUM_FAST_OBSERVER_WRITE_( m_num )
    return *this;
}


sc_dt::sc_fxnum_fast_bitref&
sc_dt::sc_fxnum_fast_bitref::operator |= ( const sc_bit& b )
{
    SC_FXNUM_FAST_OBSERVER_READ_( m_num )
    set( get() || static_cast<bool>( b ) );
    SC_FXNUM_FAST_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_fast_bitref&
sc_dt::sc_fxnum_fast_bitref::operator |= ( bool b )
{
    SC_FXNUM_FAST_OBSERVER_READ_( m_num )
    set( get() || b );
    SC_FXNUM_FAST_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_fast_bitref&
sc_dt::sc_fxnum_fast_bitref::operator ^= ( const sc_fxnum_bitref& b )
{
    SC_FXNUM_FAST_OBSERVER_READ_( m_num )
    SC_FXNUM_OBSERVER_READ_( b.m_num )
    set( get() != b.get() );
    SC_FXNUM_FAST_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_fast_bitref&
sc_dt::sc_fxnum_fast_bitref::operator ^= ( const sc_fxnum_fast_bitref& b )
{
    SC_FXNUM_FAST_OBSERVER_READ_( m_num )
    SC_FXNUM_FAST_OBSERVER_READ_( b.m_num )
    set( get() != b.get() );
    SC_FXNUM_FAST_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_fast_bitref&
sc_dt::sc_fxnum_fast_bitref::operator ^= ( const sc_bit& b )
{
    SC_FXNUM_FAST_OBSERVER_READ_( m_num )
    set( get() != static_cast<bool>( b ) );
    SC_FXNUM_FAST_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_fast_bitref&
sc_dt::sc_fxnum_fast_bitref::operator ^= ( bool b )
{
    SC_FXNUM_FAST_OBSERVER_READ_( m_num )
    set( get() != b );
    SC_FXNUM_FAST_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_fast_bitref::sc_fxnum_fast_bitref::operator bool() const
{
    SC_FXNUM_FAST_OBSERVER_READ_( m_num )
    return get();
}

::std::ostream&
sc_dt::operator << ( ::std::ostream& os, const sc_fxnum_fast_bitref& a )
{
    a.print( os );
    return os;
}

::std::istream&
sc_dt::operator >> ( ::std::istream& is, sc_fxnum_fast_bitref& a )
{
    a.scan( is );
    return is;
}

// ----------------------------------------------------------------------------
//  CLASS : sc_fxnum_subref
//
//  Proxy class for part-selection in class sc_fxnum,
//  behaves like sc_bv_base.
// ----------------------------------------------------------------------------
sc_dt::sc_fxnum_subref::sc_fxnum_subref( sc_fxnum& num_, int from_, int to_ )
    : m_num( num_ ), m_from( from_ ), m_to( to_ ),
      m_bv( *new sc_bv_base( sc_max( m_from, m_to ) -
			     sc_min( m_from, m_to ) + 1 ) )
{}

sc_dt::sc_fxnum_subref::sc_fxnum_subref( const sc_fxnum_subref& a )
    : m_num( a.m_num ), m_from( a.m_from ), m_to( a.m_to ),
      m_bv( *new sc_bv_base( a.m_bv ) )
{}

sc_dt::sc_fxnum_subref::~sc_fxnum_subref()
{
    delete &m_bv;
}

sc_dt::sc_fxnum_subref&
sc_dt::sc_fxnum_subref::operator = ( const sc_fxnum_subref& a )
{
    if( &a != this )
    {
	m_bv = static_cast<sc_bv_base>( a );
	set();
	SC_FXNUM_OBSERVER_WRITE_( m_num )
    }
    return *this;
}

sc_dt::sc_fxnum_subref&
sc_dt::sc_fxnum_subref::operator = ( const sc_fxnum_fast_subref& a )
{
    m_bv = static_cast<sc_bv_base>( a );
    set();
    SC_FXNUM_OBSERVER_WRITE_( m_num )
    return *this;
}

int
sc_dt::sc_fxnum_subref::length() const
{
    return m_bv.length();
}

int
sc_dt::sc_fxnum_subref::to_int() const
{
    SC_FXNUM_OBSERVER_READ_( m_num )
    get();
    return m_bv.to_int();
}

sc_dt::int64
sc_dt::sc_fxnum_subref::to_int64() const
{
    SC_FXNUM_OBSERVER_READ_( m_num )
    get();
    return m_bv.to_int64();
}

unsigned int
sc_dt::sc_fxnum_subref::to_uint() const
{
    SC_FXNUM_OBSERVER_READ_( m_num )
    get();
    return m_bv.to_uint();
}

sc_dt::uint64
sc_dt::sc_fxnum_subref::to_uint64() const
{
    SC_FXNUM_OBSERVER_READ_( m_num )
    get();
    return m_bv.to_uint64();
}

long
sc_dt::sc_fxnum_subref::to_long() const
{
    SC_FXNUM_OBSERVER_READ_( m_num )
    get();
    return m_bv.to_long();
}

unsigned long
sc_dt::sc_fxnum_subref::to_ulong() const
{
    SC_FXNUM_OBSERVER_READ_( m_num )
    get();
    return m_bv.to_ulong();
}

sc_dt::sc_fxnum_subref::operator sc_bv_base () const
{
    SC_FXNUM_OBSERVER_READ_( m_num )
    get();
    return m_bv;
}

const std::string
sc_dt::sc_fxnum_subref::to_string() const
{
    get();
    return m_bv.to_string();
}

const std::string
sc_dt::sc_fxnum_subref::to_string( sc_numrep numrep ) const
{
    get();
    return m_bv.to_string( numrep );
}

const std::string
sc_dt::sc_fxnum_subref::to_string( sc_numrep numrep, bool w_prefix ) const
{
    get();
    return m_bv.to_string( numrep, w_prefix );
}

::std::ostream&
sc_dt::operator << ( ::std::ostream& os, const sc_fxnum_subref& a )
{
    a.print( os );
    return os;
}

::std::istream&
sc_dt::operator >> ( ::std::istream& is, sc_fxnum_subref& a )
{
    a.scan( is );
    return is;
}
// ----------------------------------------------------------------------------
//  CLASS : sc_fxnum_fast_subref
//
//  Proxy class for part-selection in class sc_fxnum_fast,
//  behaves like sc_bv_base.
// ----------------------------------------------------------------------------

sc_dt::sc_fxnum_fast_subref::sc_fxnum_fast_subref( sc_fxnum_fast& num_,
					    int from_, int to_ )
    : m_num( num_ ), m_from( from_ ), m_to( to_ ),
      m_bv( *new sc_bv_base( sc_max( m_from, m_to ) -
			     sc_min( m_from, m_to ) + 1 ) )
{}

  sc_dt::sc_fxnum_fast_subref::sc_fxnum_fast_subref( const sc_fxnum_fast_subref& a )
    : m_num( a.m_num ), m_from( a.m_from ), m_to( a.m_to ),
      m_bv( *new sc_bv_base( a.m_bv ) )
{}

sc_dt::sc_fxnum_fast_subref::~sc_fxnum_fast_subref()
{
    delete &m_bv;
}

sc_dt::sc_fxnum_fast_subref&
sc_dt::sc_fxnum_fast_subref::operator = ( const sc_fxnum_subref& a )
{
    m_bv = static_cast<sc_bv_base>( a );
    set();
    SC_FXNUM_FAST_OBSERVER_WRITE_( m_num )
    return *this;
}

sc_dt::sc_fxnum_fast_subref&
sc_dt::sc_fxnum_fast_subref::operator = ( const sc_fxnum_fast_subref& a )
{
    if( &a != this )
    {
	m_bv = static_cast<sc_bv_base>( a );
	set();
	SC_FXNUM_FAST_OBSERVER_WRITE_( m_num )
    }
    return *this;
}

int
sc_dt::sc_fxnum_fast_subref::length() const
{
    return m_bv.length();
}

int
sc_dt::sc_fxnum_fast_subref::to_int() const
{
    SC_FXNUM_FAST_OBSERVER_READ_( m_num )
    get();
    return m_bv.to_int();
}

sc_dt::int64
sc_dt::sc_fxnum_fast_subref::to_int64() const
{
    SC_FXNUM_FAST_OBSERVER_READ_( m_num )
    get();
    return m_bv.to_int64();
}

unsigned int
sc_dt::sc_fxnum_fast_subref::to_uint() const
{
    SC_FXNUM_FAST_OBSERVER_READ_( m_num )
    get();
    return m_bv.to_uint();
}

sc_dt::uint64
sc_dt::sc_fxnum_fast_subref::to_uint64() const
{
    SC_FXNUM_FAST_OBSERVER_READ_( m_num )
    get();
    return m_bv.to_uint64();
}

long
sc_dt::sc_fxnum_fast_subref::to_long() const
{
    SC_FXNUM_FAST_OBSERVER_READ_( m_num )
    get();
    return m_bv.to_long();
}

unsigned long
sc_dt::sc_fxnum_fast_subref::to_ulong() const
{
    SC_FXNUM_FAST_OBSERVER_READ_( m_num )
    get();
    return m_bv.to_ulong();
}

const std::string
sc_dt::sc_fxnum_fast_subref::to_string() const
{
    get();
    return m_bv.to_string();
}

const std::string
sc_dt::sc_fxnum_fast_subref::to_string( sc_numrep numrep ) const
{
    get();
    return m_bv.to_string( numrep );
}

const std::string
sc_dt::sc_fxnum_fast_subref::to_string( sc_numrep numrep, bool w_prefix ) const
{
    get();
    return m_bv.to_string( numrep, w_prefix );
}

sc_dt::sc_fxnum_fast_subref::operator sc_bv_base () const
{
    SC_FXNUM_FAST_OBSERVER_READ_( m_num )
    get();
    return m_bv;
}

::std::ostream&
sc_dt::operator << ( ::std::ostream& os, const sc_fxnum_fast_subref& a )
{
    a.print( os );
    return os;
}

::std::istream&
sc_dt::operator >> ( ::std::istream& is, sc_fxnum_fast_subref& a )
{
    a.scan( is );
    return is;
}

// ----------------------------------------------------------------------------
//  CLASS : sc_fxnum
//
//  Base class for the fixed-point types; arbitrary precision.
// ----------------------------------------------------------------------------

// unary functions

void
sc_dt::neg( sc_fxval& c, const sc_fxnum& a )
{
    SC_FXNUM_OBSERVER_READ_( a )
    c.set_rep( sc_dt::neg_scfx_rep( *a.m_rep ) );
}

void
sc_dt::neg( sc_fxnum& c, const sc_fxnum& a )
{
    SC_FXNUM_OBSERVER_READ_( a )
    delete c.m_rep;
    c.m_rep = sc_dt::neg_scfx_rep( *a.m_rep );
    c.cast();
    SC_FXNUM_OBSERVER_WRITE_( c )
}

sc_dt::sc_fxnum_observer*
sc_dt::sc_fxnum::observer() const
{
    return m_observer;
}

void
sc_dt::lshift( sc_fxval& c, const sc_fxnum& a, int b )
{
    SC_FXNUM_OBSERVER_READ_( a )
    c.set_rep( sc_dt::lsh_scfx_rep( *a.m_rep, b ) );
}

void
sc_dt::rshift( sc_fxval& c, const sc_fxnum& a, int b )
{
    SC_FXNUM_OBSERVER_READ_( a )
    c.set_rep( sc_dt::rsh_scfx_rep( *a.m_rep, b ) );
}

void
sc_dt::lshift( sc_fxnum& c, const sc_fxnum& a, int b )
{
    SC_FXNUM_OBSERVER_READ_( a )
    delete c.m_rep;
    c.m_rep = sc_dt::lsh_scfx_rep( *a.m_rep, b );
    c.cast();
    SC_FXNUM_OBSERVER_WRITE_( c )
}

void
sc_dt::rshift( sc_fxnum& c, const sc_fxnum& a, int b )
{
    SC_FXNUM_OBSERVER_READ_( a )
    delete c.m_rep;
    c.m_rep = sc_dt::rsh_scfx_rep( *a.m_rep, b );
    c.cast();
    SC_FXNUM_OBSERVER_WRITE_( c )
}

void
sc_dt::sc_fxnum::cast()
{
    SC_ERROR_IF_( ! m_rep->is_normal(), sc_core::SC_ID_INVALID_FX_VALUE_ );

    if( m_params.cast_switch() == SC_ON )
	m_rep->cast( m_params, m_q_flag, m_o_flag );
}

sc_dt::sc_fxnum::sc_fxnum( const sc_fxtype_params& type_params_,
		    sc_enc enc_,
		    const sc_fxcast_switch& cast_sw,
		    sc_fxnum_observer* observer_ )
: m_rep( new scfx_rep ),
  m_params( type_params_, enc_, cast_sw ),
  m_q_flag( false ),
  m_o_flag( false ),
  m_observer( observer_ )
{
    SC_FXNUM_OBSERVER_DEFAULT_
    SC_FXNUM_OBSERVER_CONSTRUCT_( *this )
}

sc_dt::sc_fxnum::~sc_fxnum()
{
    SC_FXNUM_OBSERVER_DESTRUCT_( *this )
    delete m_rep;
}

// internal use only;

const sc_dt::scfx_rep*
sc_dt::sc_fxnum::get_rep() const
{
    SC_FXNUM_OBSERVER_READ_( *this )
    return m_rep;
}

// unary operators

const sc_dt::sc_fxval
sc_dt::sc_fxnum::operator - () const
{
    SC_FXNUM_OBSERVER_READ_( *this )
    return sc_fxval( sc_dt::neg_scfx_rep( *m_rep ) );
}

const sc_dt::sc_fxval
sc_dt::sc_fxnum::operator + () const
{
    SC_FXNUM_OBSERVER_READ_( *this )
    return sc_fxval( new scfx_rep( *m_rep ) );
}

// assignment operators

sc_dt::sc_fxnum&
sc_dt::sc_fxnum::operator = ( const sc_fxnum& a )
{
    if( &a != this )
    {
        SC_FXNUM_OBSERVER_READ_( a )
	*m_rep = *a.m_rep;
	cast();
	SC_FXNUM_OBSERVER_WRITE_( *this )
    }
    return *this;
}

sc_dt::sc_fxnum&
sc_dt::sc_fxnum::operator = ( const sc_fxval& a )
{
    *m_rep = *a.get_rep();
    cast();
    SC_FXNUM_OBSERVER_WRITE_( *this )
    return *this;
}

sc_dt::sc_fxnum&
sc_dt::sc_fxnum::operator <<= ( int b )
{
    SC_FXNUM_OBSERVER_READ_( *this )
    m_rep->lshift( b );
    cast();
    SC_FXNUM_OBSERVER_WRITE_( *this )
    return *this;
}

sc_dt::sc_fxnum&
sc_dt::sc_fxnum::operator >>= ( int b )
{
    SC_FXNUM_OBSERVER_READ_( *this )
    m_rep->rshift( b );
    cast();
    SC_FXNUM_OBSERVER_WRITE_( *this )
    return *this;
}

// auto-increment and auto-decrement

const sc_dt::sc_fxval
sc_dt::sc_fxnum::operator ++ ( int )
{
    sc_fxval c( *this );
    (*this) += 1;
    return c;
}

const sc_dt::sc_fxval
sc_dt::sc_fxnum::operator -- ( int )
{
    sc_fxval c( *this );
    (*this) -= 1;
    return c;
}

sc_dt::sc_fxnum&
sc_dt::sc_fxnum::operator ++ ()
{
    (*this) += 1;
    return *this;
}

sc_dt::sc_fxnum&
sc_dt::sc_fxnum::operator -- ()
{
    (*this) -= 1;
    return *this;
}

// bit selection

const sc_dt::sc_fxnum_bitref
sc_dt::sc_fxnum::operator [] ( int i ) const
{
    SC_ERROR_IF_( i < 0 || i >= m_params.wl(), sc_core::SC_ID_OUT_OF_RANGE_ );
    return sc_fxnum_bitref( const_cast<sc_fxnum&>( *this ),
			    i - m_params.fwl() );
}

sc_dt::sc_fxnum_bitref
sc_dt::sc_fxnum::operator [] ( int i )
{
    SC_ERROR_IF_( i < 0 || i >= m_params.wl(), sc_core::SC_ID_OUT_OF_RANGE_ );
    return sc_fxnum_bitref( *this, i - m_params.fwl() );
}

const sc_dt::sc_fxnum_bitref
sc_dt::sc_fxnum::bit( int i ) const
{
    SC_ERROR_IF_( i < 0 || i >= m_params.wl(), sc_core::SC_ID_OUT_OF_RANGE_ );
    return sc_fxnum_bitref( const_cast<sc_fxnum&>( *this ),
			    i - m_params.fwl() );
}

sc_dt::sc_fxnum_bitref
sc_dt::sc_fxnum::bit( int i )
{
    SC_ERROR_IF_( i < 0 || i >= m_params.wl(), sc_core::SC_ID_OUT_OF_RANGE_ );
    return sc_fxnum_bitref( *this, i - m_params.fwl() );
}

// part selection

const sc_dt::sc_fxnum_subref
sc_dt::sc_fxnum::operator () ( int i, int j ) const
{
    SC_ERROR_IF_( i < 0 || i >= m_params.wl(), sc_core::SC_ID_OUT_OF_RANGE_ );
    SC_ERROR_IF_( j < 0 || j >= m_params.wl(), sc_core::SC_ID_OUT_OF_RANGE_ );

    return sc_fxnum_subref( const_cast<sc_fxnum&>( *this ),
			    i - m_params.fwl(), j - m_params.fwl() );
}

sc_dt::sc_fxnum_subref
sc_dt::sc_fxnum::operator () ( int i, int j )
{
    SC_ERROR_IF_( i < 0 || i >= m_params.wl(), sc_core::SC_ID_OUT_OF_RANGE_ );
    SC_ERROR_IF_( j < 0 || j >= m_params.wl(), sc_core::SC_ID_OUT_OF_RANGE_ );

    return sc_fxnum_subref( *this, i - m_params.fwl(), j - m_params.fwl() );
}

const sc_dt::sc_fxnum_subref
sc_dt::sc_fxnum::range( int i, int j ) const
{
    SC_ERROR_IF_( i < 0 || i >= m_params.wl(), sc_core::SC_ID_OUT_OF_RANGE_ );
    SC_ERROR_IF_( j < 0 || j >= m_params.wl(), sc_core::SC_ID_OUT_OF_RANGE_ );

    return sc_fxnum_subref( const_cast<sc_fxnum&>( *this ),
			    i - m_params.fwl(), j - m_params.fwl() );
}

sc_dt::sc_fxnum_subref
sc_dt::sc_fxnum::range( int i, int j )
{
    SC_ERROR_IF_( i < 0 || i >= m_params.wl(), sc_core::SC_ID_OUT_OF_RANGE_ );
    SC_ERROR_IF_( j < 0 || j >= m_params.wl(), sc_core::SC_ID_OUT_OF_RANGE_ );

    return sc_fxnum_subref( *this, i - m_params.fwl(), j - m_params.fwl() );
}

const sc_dt::sc_fxnum_subref
sc_dt::sc_fxnum::operator () () const
{
    return this->operator () ( m_params.wl() - 1, 0 );
}

sc_dt::sc_fxnum_subref
sc_dt::sc_fxnum::operator () ()
{
    return this->operator () ( m_params.wl() - 1, 0 );
}

const sc_dt::sc_fxnum_subref
sc_dt::sc_fxnum::range() const
{
    return this->range( m_params.wl() - 1, 0 );
}

sc_dt::sc_fxnum_subref
sc_dt::sc_fxnum::range()
{
    return this->range( m_params.wl() - 1, 0 );
}

// implicit conversion

sc_dt::sc_fxnum::operator double() const
{
    SC_FXNUM_OBSERVER_READ_( *this )
    return m_rep->to_double();
}

// explicit conversion to primitive types

short
sc_dt::sc_fxnum::to_short() const
{
    SC_FXNUM_OBSERVER_READ_( *this )
    return static_cast<short>( m_rep->to_double() );
}

unsigned short
sc_dt::sc_fxnum::to_ushort() const
{
    SC_FXNUM_OBSERVER_READ_( *this )
    return static_cast<unsigned short>( m_rep->to_double() );
}

int
sc_dt::sc_fxnum::to_int() const
{
    SC_FXNUM_OBSERVER_READ_( *this )
    return static_cast<int>( m_rep->to_double() );
}

sc_dt::int64
sc_dt::sc_fxnum::to_int64() const
{
    SC_FXNUM_OBSERVER_READ_( *this )
    return static_cast<int64>( m_rep->to_double() );
}

unsigned int
sc_dt::sc_fxnum::to_uint() const
{
    SC_FXNUM_OBSERVER_READ_( *this )
    return static_cast<unsigned int>( m_rep->to_double() );
}

sc_dt::uint64
sc_dt::sc_fxnum::to_uint64() const
{
    SC_FXNUM_OBSERVER_READ_( *this )
    return static_cast<uint64>( m_rep->to_double() );
}

long
sc_dt::sc_fxnum::to_long() const
{
    SC_FXNUM_OBSERVER_READ_( *this )
    return static_cast<long>( m_rep->to_double() );
}

unsigned long
sc_dt::sc_fxnum::to_ulong() const
{
    SC_FXNUM_OBSERVER_READ_( *this )
    return static_cast<unsigned long>( m_rep->to_double() );
}

float
sc_dt::sc_fxnum::to_float() const
{
    SC_FXNUM_OBSERVER_READ_( *this )
    return static_cast<float>( m_rep->to_double() );
}

double
sc_dt::sc_fxnum::to_double() const
{
    SC_FXNUM_OBSERVER_READ_( *this )
    return m_rep->to_double();
}

// query value

bool
sc_dt::sc_fxnum::is_neg() const
{
    SC_FXNUM_OBSERVER_READ_( *this )
    return m_rep->is_neg();
}

bool
sc_dt::sc_fxnum::is_zero() const
{
    SC_FXNUM_OBSERVER_READ_( *this )
    return m_rep->is_zero();
}

// internal use only;

bool
sc_dt::sc_fxnum::is_normal() const
{
    SC_FXNUM_OBSERVER_READ_( *this )
    return m_rep->is_normal();
}

bool
sc_dt::sc_fxnum::quantization_flag() const
{
    return m_q_flag;
}

bool
sc_dt::sc_fxnum::overflow_flag() const
{
    return m_o_flag;
}

const sc_dt::sc_fxval
sc_dt::sc_fxnum::value() const
{
    SC_FXNUM_OBSERVER_READ_( *this )
    return sc_fxval( new scfx_rep( *m_rep ) );
}

// query parameters

int
sc_dt::sc_fxnum::wl() const
{
    return m_params.wl();
}

int
sc_dt::sc_fxnum::iwl() const
{
    return m_params.iwl();
}

sc_dt::sc_q_mode
sc_dt::sc_fxnum::q_mode() const
{
    return m_params.q_mode();
}

sc_dt::sc_o_mode
sc_dt::sc_fxnum::o_mode() const
{
    return m_params.o_mode();
}

int
sc_dt::sc_fxnum::n_bits() const
{
    return m_params.n_bits();
}

const sc_dt::sc_fxtype_params&
sc_dt::sc_fxnum::type_params() const
{
    return m_params.type_params();
}

const sc_dt::sc_fxcast_switch&
sc_dt::sc_fxnum::cast_switch() const
{
    return m_params.cast_switch();
}

// internal use only;

void
sc_dt::sc_fxnum::observer_read() const
{
    SC_FXNUM_OBSERVER_READ_( *this );
}

// internal use only;

bool
sc_dt::sc_fxnum::get_bit( int i ) const
{
    return m_rep->get_bit( i );
}

// protected methods and friend functions

bool
sc_dt::sc_fxnum::set_bit( int i, bool high )
{
    if( high )
        return m_rep->set( i, m_params );
    else
        return m_rep->clear( i, m_params );
}

bool
sc_dt::sc_fxnum::get_slice( int i, int j, sc_bv_base& bv ) const
{
    return m_rep->get_slice( i, j, m_params, bv );
}

bool
sc_dt::sc_fxnum::set_slice( int i, int j, const sc_bv_base& bv )
{
    return m_rep->set_slice( i, j, m_params, bv );
}

const sc_dt::sc_fxval
sc_dt::operator / ( const sc_fxnum& a, const sc_fxnum& b )
{
    SC_FXNUM_OBSERVER_READ_( a )
    SC_FXNUM_OBSERVER_READ_( b )
    return sc_fxval( sc_dt::div_scfx_rep( *a.m_rep, *b.m_rep ) );
}

const sc_dt::sc_fxval
sc_dt::operator / ( const sc_fxnum& a, const sc_fxval& b )
{
    SC_FXNUM_OBSERVER_READ_( a )
    return sc_fxval( sc_dt::div_scfx_rep( *a.m_rep, *b.get_rep() ) );
}

const sc_dt::sc_fxval
sc_dt::operator / ( const sc_fxval& a, const sc_fxnum& b )
{
    SC_FXNUM_OBSERVER_READ_( b )
    return sc_fxval( sc_dt::div_scfx_rep( *a.get_rep(), *b.m_rep ) );
}

const sc_dt::sc_fxval
sc_dt::operator << ( const sc_fxnum& a, int b )
{
    SC_FXNUM_OBSERVER_READ_( a )
    return sc_fxval( sc_dt::lsh_scfx_rep( *a.m_rep, b ) );
}

const sc_dt::sc_fxval
sc_dt::operator >> ( const sc_fxnum& a, int b )
{
    SC_FXNUM_OBSERVER_READ_( a )
    return sc_fxval( sc_dt::rsh_scfx_rep( *a.m_rep, b ) );
}

::std::ostream&
sc_dt::operator << ( ::std::ostream& os, const sc_fxnum& a )
{
    a.print( os );
    return os;
}

::std::istream&
sc_dt::operator >> ( ::std::istream& is, sc_fxnum& a )
{
    a.scan( is );
    return is;
}
// ----------------------------------------------------------------------------
//  CLASS : sc_fxnum_fast
//
//  Base class for the fixed-point types; limited precision.
// ----------------------------------------------------------------------------

sc_dt::sc_fxnum_fast_observer*
sc_dt::sc_fxnum_fast::observer() const
{
    return m_observer;
}

// constructors

sc_dt::sc_fxnum_fast::sc_fxnum_fast( const sc_fxtype_params& type_params_,
			      sc_enc enc_,
			      const sc_fxcast_switch& cast_sw,
			      sc_fxnum_fast_observer* observer_ )
: m_val( 0.0 ),
  m_params( type_params_, enc_, cast_sw ),
  m_q_flag( false ),
  m_o_flag( false ),
  m_observer( observer_ )
{
    SC_FXNUM_FAST_OBSERVER_DEFAULT_
    SC_FXNUM_FAST_OBSERVER_CONSTRUCT_(*this)
}

sc_dt::sc_fxnum_fast::sc_fxnum_fast( const sc_fxnum_fast& a,
			      const sc_fxtype_params& type_params_,
			      sc_enc enc_,
			      const sc_fxcast_switch& cast_sw,
			      sc_fxnum_fast_observer* observer_ )
: m_val( a.m_val ),
  m_params( type_params_, enc_, cast_sw ),
  m_q_flag( false ),
  m_o_flag( false ),
  m_observer( observer_ )
{
    SC_FXNUM_FAST_OBSERVER_DEFAULT_
    SC_FXNUM_FAST_OBSERVER_READ_( a )
    cast();
    SC_FXNUM_FAST_OBSERVER_CONSTRUCT_( *this )
    SC_FXNUM_FAST_OBSERVER_WRITE_( *this )
}

sc_dt::sc_fxnum_fast::~sc_fxnum_fast()
{
    SC_FXNUM_FAST_OBSERVER_DESTRUCT_( *this )
}

// internal use only;

double
sc_dt::sc_fxnum_fast::get_val() const
{
    SC_FXNUM_FAST_OBSERVER_READ_( *this )
    return m_val;
}

// unary operators

const sc_dt::sc_fxval_fast
sc_dt::sc_fxnum_fast::operator - () const
{
    SC_FXNUM_FAST_OBSERVER_READ_( *this )
    return sc_fxval_fast( - m_val );
}

const sc_dt::sc_fxval_fast
sc_dt::sc_fxnum_fast::operator + () const
{
    SC_FXNUM_FAST_OBSERVER_READ_( *this )
    return sc_fxval_fast( m_val );
}

// unary functions

void
sc_dt::neg( sc_fxval_fast& c, const sc_fxnum_fast& a )
{
    SC_FXNUM_FAST_OBSERVER_READ_( a )
    c.set_val( - a.m_val );
}

void
sc_dt::neg( sc_fxnum_fast& c, const sc_fxnum_fast& a )
{
    SC_FXNUM_FAST_OBSERVER_READ_( a )
    c.m_val = - a.m_val;
    c.cast();
    SC_FXNUM_FAST_OBSERVER_WRITE_( c )
}

void
sc_dt::lshift( sc_fxval_fast& c, const sc_fxnum_fast& a, int b )
{
    SC_FXNUM_FAST_OBSERVER_READ_( a )
    c.set_val( a.m_val * scfx_pow2( b ) );
}

void
sc_dt::rshift( sc_fxval_fast& c, const sc_fxnum_fast& a, int b )
{
    SC_FXNUM_FAST_OBSERVER_READ_( a )
    c.set_val( a.m_val * scfx_pow2( -b ) );
}

void
sc_dt::lshift( sc_fxnum_fast& c, const sc_fxnum_fast& a, int b )
{
    SC_FXNUM_FAST_OBSERVER_READ_( a )
    c.m_val = a.m_val * scfx_pow2( b );
    c.cast();
    SC_FXNUM_FAST_OBSERVER_WRITE_( c )
}

void
sc_dt::rshift( sc_fxnum_fast& c, const sc_fxnum_fast& a, int b )
{
    SC_FXNUM_FAST_OBSERVER_READ_( a )
    c.m_val = a.m_val * scfx_pow2( -b );
    c.cast();
    SC_FXNUM_FAST_OBSERVER_WRITE_( c )
}

// assignment operators

sc_dt::sc_fxnum_fast&
sc_dt::sc_fxnum_fast::operator = ( const sc_fxnum_fast& a )
{
    if( &a != this )
    {
	SC_FXNUM_FAST_OBSERVER_READ_( a )
	m_val = a.m_val;
	cast();
	SC_FXNUM_FAST_OBSERVER_WRITE_( *this )
    }
    return *this;
}

sc_dt::sc_fxnum_fast&
sc_dt::sc_fxnum_fast::operator = ( const sc_fxval_fast& a )
{
    m_val = a.get_val();
    cast();
    SC_FXNUM_FAST_OBSERVER_WRITE_( *this )
    return *this;
}

sc_dt::sc_fxnum_fast&
sc_dt::sc_fxnum_fast::operator <<= ( int b )
{
    SC_FXNUM_FAST_OBSERVER_READ_( *this )
    m_val *= scfx_pow2( b );
    cast();
    SC_FXNUM_FAST_OBSERVER_WRITE_( *this )
    return *this;
}

sc_dt::sc_fxnum_fast&
sc_dt::sc_fxnum_fast::operator >>= ( int b )
{
    SC_FXNUM_FAST_OBSERVER_READ_( *this )
    m_val *= scfx_pow2( -b );
    cast();
    SC_FXNUM_FAST_OBSERVER_WRITE_( *this )
    return *this;
}

// auto-increment and auto-decrement

const sc_dt::sc_fxval_fast
sc_dt::sc_fxnum_fast::operator ++ ( int )
{
    SC_FXNUM_FAST_OBSERVER_READ_( *this )
    SC_FXNUM_FAST_OBSERVER_READ_( *this )
    double c = m_val;
    m_val = m_val + 1;
    cast();
    SC_FXNUM_FAST_OBSERVER_WRITE_( *this )
    return sc_fxval_fast( c );
}

const sc_dt::sc_fxval_fast
sc_dt::sc_fxnum_fast::operator -- ( int )
{
    SC_FXNUM_FAST_OBSERVER_READ_( *this )
    SC_FXNUM_FAST_OBSERVER_READ_( *this )
    double c = m_val;
    m_val = m_val - 1;
    cast();
    SC_FXNUM_FAST_OBSERVER_WRITE_( *this )
    return sc_fxval_fast( c );
}

sc_dt::sc_fxnum_fast&
sc_dt::sc_fxnum_fast::operator ++ ()
{
    SC_FXNUM_FAST_OBSERVER_READ_( *this )
    m_val = m_val + 1;
    cast();
    SC_FXNUM_FAST_OBSERVER_WRITE_( *this )
    return *this;
}

sc_dt::sc_fxnum_fast&
sc_dt::sc_fxnum_fast::operator -- ()
{
    SC_FXNUM_FAST_OBSERVER_READ_( *this )
    m_val = m_val - 1;
    cast();
    SC_FXNUM_FAST_OBSERVER_WRITE_( *this )
    return *this;
}

// bit selection

const sc_dt::sc_fxnum_fast_bitref
sc_dt::sc_fxnum_fast::operator [] ( int i ) const
{
    SC_ERROR_IF_( i < 0 || i >= m_params.wl(), sc_core::SC_ID_OUT_OF_RANGE_ );
    return sc_fxnum_fast_bitref( const_cast<sc_fxnum_fast&>( *this ),
				 i - m_params.fwl() );
}

sc_dt::sc_fxnum_fast_bitref
sc_dt::sc_fxnum_fast::operator [] ( int i )
{
    SC_ERROR_IF_( i < 0 || i >= m_params.wl(), sc_core::SC_ID_OUT_OF_RANGE_ );
    return sc_fxnum_fast_bitref( *this, i - m_params.fwl() );
}

const sc_dt::sc_fxnum_fast_bitref
sc_dt::sc_fxnum_fast::bit( int i ) const
{
    SC_ERROR_IF_( i < 0 || i >= m_params.wl(), sc_core::SC_ID_OUT_OF_RANGE_ );
    return sc_fxnum_fast_bitref( const_cast<sc_fxnum_fast&>( *this ),
				 i - m_params.fwl() );
}

sc_dt::sc_fxnum_fast_bitref
sc_dt::sc_fxnum_fast::bit( int i )
{
    SC_ERROR_IF_( i < 0 || i >= m_params.wl(), sc_core::SC_ID_OUT_OF_RANGE_ );
    return sc_fxnum_fast_bitref( *this, i - m_params.fwl() );
}

// part selection

const sc_dt::sc_fxnum_fast_subref
sc_dt::sc_fxnum_fast::operator () ( int i, int j ) const
{
    SC_ERROR_IF_( i < 0 || i >= m_params.wl(), sc_core::SC_ID_OUT_OF_RANGE_ );
    SC_ERROR_IF_( j < 0 || j >= m_params.wl(), sc_core::SC_ID_OUT_OF_RANGE_ );

    return sc_fxnum_fast_subref( const_cast<sc_fxnum_fast&>( *this ),
				 i - m_params.fwl(), j - m_params.fwl() );
}

sc_dt::sc_fxnum_fast_subref
sc_dt::sc_fxnum_fast::operator () ( int i, int j )
{
    SC_ERROR_IF_( i < 0 || i >= m_params.wl(), sc_core::SC_ID_OUT_OF_RANGE_ );
    SC_ERROR_IF_( j < 0 || j >= m_params.wl(), sc_core::SC_ID_OUT_OF_RANGE_ );

    return sc_fxnum_fast_subref( *this,
				 i - m_params.fwl(), j - m_params.fwl() );
}

const sc_dt::sc_fxnum_fast_subref
sc_dt::sc_fxnum_fast::range( int i, int j ) const
{
    SC_ERROR_IF_( i < 0 || i >= m_params.wl(), sc_core::SC_ID_OUT_OF_RANGE_ );
    SC_ERROR_IF_( j < 0 || j >= m_params.wl(), sc_core::SC_ID_OUT_OF_RANGE_ );

    return sc_fxnum_fast_subref( const_cast<sc_fxnum_fast&>( *this ),
				 i - m_params.fwl(), j - m_params.fwl() );
}

sc_dt::sc_fxnum_fast_subref
sc_dt::sc_fxnum_fast::range( int i, int j )
{
    SC_ERROR_IF_( i < 0 || i >= m_params.wl(), sc_core::SC_ID_OUT_OF_RANGE_ );
    SC_ERROR_IF_( j < 0 || j >= m_params.wl(), sc_core::SC_ID_OUT_OF_RANGE_ );

    return sc_fxnum_fast_subref( *this,
				 i - m_params.fwl(), j - m_params.fwl() );
}

const sc_dt::sc_fxnum_fast_subref
sc_dt::sc_fxnum_fast::operator () () const
{
    return this->operator () ( m_params.wl() - 1, 0 );
}

sc_dt::sc_fxnum_fast_subref
sc_dt::sc_fxnum_fast::operator () ()
{
    return this->operator () ( m_params.wl() - 1, 0 );
}

const sc_dt::sc_fxnum_fast_subref
sc_dt::sc_fxnum_fast::range() const
{
    return this->range( m_params.wl() - 1, 0 );
}

sc_dt::sc_fxnum_fast_subref
sc_dt::sc_fxnum_fast::range()
{
    return this->range( m_params.wl() - 1, 0 );
}

// implicit conversion

sc_dt::sc_fxnum_fast::operator double() const
{
    SC_FXNUM_FAST_OBSERVER_READ_( *this )
    return m_val;
}

// explicit conversion to primitive types

short
sc_dt::sc_fxnum_fast::to_short() const
{
    SC_FXNUM_FAST_OBSERVER_READ_( *this )
    return static_cast<short>( m_val );
}

unsigned short
sc_dt::sc_fxnum_fast::to_ushort() const
{
    SC_FXNUM_FAST_OBSERVER_READ_( *this )
    return static_cast<unsigned short>( m_val );
}

int
sc_dt::sc_fxnum_fast::to_int() const
{
    SC_FXNUM_FAST_OBSERVER_READ_( *this )
    return static_cast<int>( m_val );
}

sc_dt::int64
sc_dt::sc_fxnum_fast::to_int64() const
{
    SC_FXNUM_FAST_OBSERVER_READ_( *this )
    return static_cast<int64>( m_val );
}

unsigned int
sc_dt::sc_fxnum_fast::to_uint() const
{
    SC_FXNUM_FAST_OBSERVER_READ_( *this )
    return static_cast<unsigned int>( m_val );
}

sc_dt::uint64
sc_dt::sc_fxnum_fast::to_uint64() const
{
    SC_FXNUM_FAST_OBSERVER_READ_( *this )
    return static_cast<uint64>( m_val );
}

long
sc_dt::sc_fxnum_fast::to_long() const
{
    SC_FXNUM_FAST_OBSERVER_READ_( *this )
    return static_cast<long>( m_val );
}

unsigned long
sc_dt::sc_fxnum_fast::to_ulong() const
{
    SC_FXNUM_FAST_OBSERVER_READ_( *this )
    return static_cast<unsigned long>( m_val );
}

float
sc_dt::sc_fxnum_fast::to_float() const
{
    SC_FXNUM_FAST_OBSERVER_READ_( *this )
    return static_cast<float>( m_val );
}

double
sc_dt::sc_fxnum_fast::to_double() const
{
    SC_FXNUM_FAST_OBSERVER_READ_( *this )
    return m_val;
}

// query value

bool
sc_dt::sc_fxnum_fast::is_neg() const
{
    SC_FXNUM_FAST_OBSERVER_READ_( *this )
    scfx_ieee_double id( m_val );
    return ( id.negative() != 0 );
}

bool
sc_dt::sc_fxnum_fast::is_zero() const
{
    SC_FXNUM_FAST_OBSERVER_READ_( *this )
    scfx_ieee_double id( m_val );
    return id.is_zero();
}

// internal use only;

bool
sc_dt::sc_fxnum_fast::is_normal() const
{
    SC_FXNUM_FAST_OBSERVER_READ_( *this )
    scfx_ieee_double id( m_val );
    return ( id.is_normal() || id.is_subnormal() || id.is_zero() );
}

bool
sc_dt::sc_fxnum_fast::quantization_flag() const
{
    return m_q_flag;
}

bool
sc_dt::sc_fxnum_fast::overflow_flag() const
{
    return m_o_flag;
}

const sc_dt::sc_fxval_fast
sc_dt::sc_fxnum_fast::value() const
{
    SC_FXNUM_FAST_OBSERVER_READ_( *this )
    return sc_fxval_fast( m_val );
}

// query parameters

int
sc_dt::sc_fxnum_fast::wl() const
{
    return m_params.wl();
}

int
sc_dt::sc_fxnum_fast::iwl() const
{
    return m_params.iwl();
}

sc_dt::sc_q_mode
sc_dt::sc_fxnum_fast::q_mode() const
{
    return m_params.q_mode();
}

sc_dt::sc_o_mode
sc_dt::sc_fxnum_fast::o_mode() const
{
    return m_params.o_mode();
}

int
sc_dt::sc_fxnum_fast::n_bits() const
{
    return m_params.n_bits();
}


const sc_dt::sc_fxtype_params&
sc_dt::sc_fxnum_fast::type_params() const
{
    return m_params.type_params();
}

const sc_dt::sc_fxcast_switch&
sc_dt::sc_fxnum_fast::cast_switch() const
{
    return m_params.cast_switch();
}

// internal use only;
void
sc_dt::sc_fxnum_fast::observer_read() const
{
    SC_FXNUM_OBSERVER_READ_( *this );
}

const sc_dt::sc_fxval_fast
sc_dt::operator / ( const sc_fxnum_fast& a, const sc_fxnum_fast& b )
{
    SC_FXNUM_FAST_OBSERVER_READ_( a )
    SC_FXNUM_FAST_OBSERVER_READ_( b )
    return sc_fxval_fast( a.m_val / b.m_val );
}

const sc_dt::sc_fxval_fast
sc_dt::operator / ( const sc_fxnum_fast& a, const sc_fxval_fast& b )
{
    SC_FXNUM_FAST_OBSERVER_READ_( a )
    return sc_fxval_fast( a.m_val / b.get_val() );
}

const sc_dt::sc_fxval_fast
sc_dt::operator / ( const sc_fxval_fast& a, const sc_fxnum_fast& b )
{
    SC_FXNUM_FAST_OBSERVER_READ_( b )
    return sc_fxval_fast( a.get_val() / b.m_val );
}

const sc_dt::sc_fxval_fast
sc_dt::operator << ( const sc_fxnum_fast& a, int b )
{
    SC_FXNUM_FAST_OBSERVER_READ_( a )
    return sc_fxval_fast( a.m_val * scfx_pow2( b ) );
}

const sc_dt::sc_fxval_fast
sc_dt::operator >> ( const sc_fxnum_fast& a, int b )
{
    SC_FXNUM_FAST_OBSERVER_READ_( a )
    return sc_fxval_fast( a.m_val * scfx_pow2( -b ) );
}

::std::ostream&
sc_dt::operator << ( ::std::ostream& os, const sc_fxnum_fast& a )
{
    a.print( os );
    return os;
}

::std::istream&
sc_dt::operator >> ( ::std::istream& is, sc_fxnum_fast& a )
{
    a.scan( is );
    return is;
}
// ----------------------------------------------------------------------------
//  CLASS : sc_fxval
//
//  Fixed-point value type; arbitrary precision.
// ----------------------------------------------------------------------------

// public constructors

sc_dt::sc_fxval::sc_fxval( const sc_fxnum& a,
		    sc_fxval_observer* observer_ )
: m_rep( new scfx_rep( *a.get_rep() ) ),
  m_observer( observer_ )
{
    SC_FXVAL_OBSERVER_DEFAULT_
    SC_FXVAL_OBSERVER_CONSTRUCT_( *this )
    SC_FXVAL_OBSERVER_WRITE_( *this )
}

sc_dt::sc_fxval::sc_fxval( const sc_fxnum_fast& a,
		    sc_fxval_observer* observer_ )
: m_rep( new scfx_rep( a.to_double() ) ),
  m_observer( observer_ )
{
    SC_FXVAL_OBSERVER_DEFAULT_
    SC_FXVAL_OBSERVER_CONSTRUCT_( *this )
    SC_FXVAL_OBSERVER_WRITE_( *this )
}

// assignment operators

sc_dt::sc_fxval&
sc_dt::sc_fxval::operator = ( const sc_fxnum& a )
{
    *m_rep = *a.get_rep();
    SC_FXVAL_OBSERVER_WRITE_( *this )
    return *this;
}

// ----------------------------------------------------------------------------
//  CLASS : sc_fxval_fast
//
//  Fixed-point value types; limited precision.
// ----------------------------------------------------------------------------

// public constructors

sc_dt::sc_fxval_fast::sc_fxval_fast( const sc_fxnum& a,
			      sc_fxval_fast_observer* observer_ )
: m_val( a.to_double() ),
  m_observer( observer_ )
{
    SC_FXVAL_FAST_OBSERVER_DEFAULT_
    SC_FXVAL_FAST_OBSERVER_CONSTRUCT_( *this )
    SC_FXVAL_FAST_OBSERVER_WRITE_( *this )
}

sc_dt::sc_fxval_fast::sc_fxval_fast( const sc_fxnum_fast& a,
			      sc_fxval_fast_observer* observer_ )
: m_val( a.get_val() ),
  m_observer( observer_ )
{
    SC_FXVAL_FAST_OBSERVER_DEFAULT_
    SC_FXVAL_FAST_OBSERVER_CONSTRUCT_( *this )
    SC_FXVAL_FAST_OBSERVER_WRITE_( *this )
}

// assignment operators

sc_dt::sc_fxval_fast&
sc_dt::sc_fxval_fast::operator = ( const sc_fxnum_fast& a )
{
    m_val = a.get_val();
    SC_FXVAL_FAST_OBSERVER_WRITE_( *this )
    return *this;
}

//--------------------------------------------------Farah is done working here
namespace sc_dt
{

// ----------------------------------------------------------------------------
//  CLASS : sc_fxnum_bitref
//
//  Proxy class for bit-selection in class sc_fxnum, behaves like sc_bit.
// ----------------------------------------------------------------------------

bool
sc_fxnum_bitref::get() const
{
    return m_num.get_bit( m_idx );
}

void
sc_fxnum_bitref::set( bool high )
{
    m_num.set_bit( m_idx, high );
}


// print or dump content

void
sc_fxnum_bitref::print( ::std::ostream& os ) const
{
    os << get();
}

void
sc_fxnum_bitref::scan( ::std::istream& is )
{
    bool b;
    is >> b;
    *this = b;
}

void
sc_fxnum_bitref::dump( ::std::ostream& os ) const
{
    os << "sc_fxnum_bitref" << ::std::endl;
    os << "(" << ::std::endl;
    os << "num = ";
    m_num.dump( os );
    os << "idx = " << m_idx << ::std::endl;
    os << ")" << ::std::endl;
}


// ----------------------------------------------------------------------------
//  CLASS : sc_fxnum_fast_bitref
//
//  Proxy class for bit-selection in class sc_fxnum_fast, behaves like sc_bit.
// ----------------------------------------------------------------------------

bool
sc_fxnum_fast_bitref::get() const
{
    return m_num.get_bit( m_idx );
}

void
sc_fxnum_fast_bitref::set( bool high )
{
    m_num.set_bit( m_idx, high );
}


// print or dump content

void
sc_fxnum_fast_bitref::print( ::std::ostream& os ) const
{
    os << get();
}

void
sc_fxnum_fast_bitref::scan( ::std::istream& is )
{
    bool b;
    is >> b;
    *this = b;
}

void
sc_fxnum_fast_bitref::dump( ::std::ostream& os ) const
{
    os << "sc_fxnum_fast_bitref" << ::std::endl;
    os << "(" << ::std::endl;
    os << "num = ";
    m_num.dump( os );
    os << "idx = " << m_idx << ::std::endl;
    os << ")" << ::std::endl;
}


// ----------------------------------------------------------------------------
//  CLASS : sc_fxnum_subref
//
//  Proxy class for part-selection in class sc_fxnum,
//  behaves like sc_bv_base.
// ----------------------------------------------------------------------------

bool
sc_fxnum_subref::get() const
{
    return m_num.get_slice( m_from, m_to, m_bv );
}

bool
sc_fxnum_subref::set()
{
    return m_num.set_slice( m_from, m_to, m_bv );
}


// print or dump content

void
sc_fxnum_subref::print( ::std::ostream& os ) const
{
    get();
    m_bv.print( os );
}

void
sc_fxnum_subref::scan( ::std::istream& is )
{
    m_bv.scan( is );
    set();
}

void
sc_fxnum_subref::dump( ::std::ostream& os ) const
{
    os << "sc_fxnum_subref" << ::std::endl;
    os << "(" << ::std::endl;
    os << "num  = ";
    m_num.dump( os );
    os << "from = " << m_from << ::std::endl;
    os << "to   = " << m_to << ::std::endl;
    os << ")" << ::std::endl;
}


// ----------------------------------------------------------------------------
//  CLASS : sc_fxnum_fast_subref
//
//  Proxy class for part-selection in class sc_fxnum_fast,
//  behaves like sc_bv_base.
// ----------------------------------------------------------------------------

bool
sc_fxnum_fast_subref::get() const
{
    return m_num.get_slice( m_from, m_to, m_bv );
}

bool
sc_fxnum_fast_subref::set()
{
    return m_num.set_slice( m_from, m_to, m_bv );
}


// print or dump content

void
sc_fxnum_fast_subref::print( ::std::ostream& os ) const
{
    get();
    m_bv.print( os );
}

void
sc_fxnum_fast_subref::scan( ::std::istream& is )
{
    m_bv.scan( is );
    set();
}

void
sc_fxnum_fast_subref::dump( ::std::ostream& os ) const
{
    os << "sc_fxnum_fast_subref" << ::std::endl;
    os << "(" << ::std::endl;
    os << "num  = ";
    m_num.dump( os );
    os << "from = " << m_from << ::std::endl;
    os << "to   = " << m_to << ::std::endl;
    os << ")" << ::std::endl;
}


// ----------------------------------------------------------------------------
//  CLASS : sc_fxnum
//
//  Base class for the fixed-point types; arbitrary precision.
// ----------------------------------------------------------------------------

// explicit conversion to character string

const std::string
sc_fxnum::to_string() const
{
    return std::string( m_rep->to_string( SC_DEC, -1, SC_F, &m_params ) );
}

const std::string
sc_fxnum::to_string( sc_numrep numrep ) const
{
    return std::string( m_rep->to_string( numrep, -1, SC_F, &m_params ) );
}

const std::string
sc_fxnum::to_string( sc_numrep numrep, bool w_prefix ) const
{
    return std::string( m_rep->to_string( numrep, (w_prefix ? 1 : 0),
					SC_F, &m_params ) );
}

const std::string
sc_fxnum::to_string( sc_fmt fmt ) const
{
    return std::string( m_rep->to_string( SC_DEC, -1, fmt, &m_params ) );
}

const std::string
sc_fxnum::to_string( sc_numrep numrep, sc_fmt fmt ) const
{
    return std::string( m_rep->to_string( numrep, -1, fmt, &m_params ) );
}

const std::string
sc_fxnum::to_string( sc_numrep numrep, bool w_prefix, sc_fmt fmt ) const
{
    return std::string( m_rep->to_string( numrep, (w_prefix ? 1 : 0),
					fmt, &m_params ) );
}


const std::string
sc_fxnum::to_dec() const
{
    return std::string( m_rep->to_string( SC_DEC, -1, SC_F, &m_params ) );
}

const std::string
sc_fxnum::to_bin() const
{
    return std::string( m_rep->to_string( SC_BIN, -1, SC_F, &m_params ) );
}

const std::string
sc_fxnum::to_oct() const
{
    return std::string( m_rep->to_string( SC_OCT, -1, SC_F, &m_params ) );
}

const std::string
sc_fxnum::to_hex() const
{
    return std::string( m_rep->to_string( SC_HEX, -1, SC_F, &m_params ) );
}


// print or dump content

void
sc_fxnum::print( ::std::ostream& os ) const
{
    os << m_rep->to_string( SC_DEC, -1, SC_F, &m_params );
}

void
sc_fxnum::scan( ::std::istream& is )
{
    std::string s;
    is >> s;
    *this = s.c_str();
}

void
sc_fxnum::dump( ::std::ostream& os ) const
{
    os << "sc_fxnum" << ::std::endl;
    os << "(" << ::std::endl;
    os << "rep      = ";
    m_rep->dump( os );
    os << "params   = ";
    m_params.dump( os );
    os << "q_flag   = " << m_q_flag << ::std::endl;
    os << "o_flag   = " << m_o_flag << ::std::endl;
    // TO BE COMPLETED
    // os << "observer = ";
    // if( m_observer != 0 )
    //     m_observer->dump( os );
    // else
    //     os << "0" << ::std::endl;
    os << ")" << ::std::endl;
}


sc_fxnum_observer*
sc_fxnum::lock_observer() const
{
    SC_ASSERT_( m_observer != 0, "lock observer failed" );
    sc_fxnum_observer* tmp = m_observer;
    m_observer = 0;
    return tmp;
}

void
sc_fxnum::unlock_observer( sc_fxnum_observer* observer_ ) const
{
    SC_ASSERT_( observer_ != 0, "unlock observer failed" );
    m_observer = observer_;
}


// ----------------------------------------------------------------------------
//  CLASS : sc_fxnum_fast
//
//  Base class for the fixed-point types; limited precision.
// ----------------------------------------------------------------------------

static
void
quantization( double& c, const scfx_params& params, bool& q_flag )
{
    int fwl = params.wl() - params.iwl();
    double scale = scfx_pow2( fwl );
    double val = scale * c;
    double int_part;
    double frac_part = modf( val, &int_part );

    q_flag = ( frac_part != 0.0 );

    if( q_flag )
    {
        val = int_part;

	switch( params.q_mode() )
	{
            case SC_TRN:			// truncation
	    {
	        if( c < 0.0 )
		    val -= 1.0;
		break;
	    }
            case SC_RND:			// rounding to plus infinity
	    {
		if( frac_part >= 0.5 )
		    val += 1.0;
		else if( frac_part < -0.5 )
		    val -= 1.0;
		break;
	    }
            case SC_TRN_ZERO:			// truncation to zero
	    {
	        break;
	    }
            case SC_RND_INF:			// rounding to infinity
	    {
		if( frac_part >= 0.5 )
		    val += 1.0;
		else if( frac_part <= -0.5 )
		    val -= 1.0;
		break;
	    }
            case SC_RND_CONV:			// convergent rounding
	    {
		if( frac_part > 0.5 ||
		    ( frac_part == 0.5 && fmod( int_part, 2.0 ) != 0.0 ) )
		    val += 1.0;
		else if( frac_part < -0.5 ||
			 ( frac_part == -0.5 && fmod( int_part, 2.0 ) != 0.0 ) )
		    val -= 1.0;
		break;
	    }
            case SC_RND_ZERO:			// rounding to zero
	    {
		if( frac_part > 0.5 )
		    val += 1.0;
		else if( frac_part < -0.5 )
		    val -= 1.0;
		break;
	    }
            case SC_RND_MIN_INF:		// rounding to minus infinity
	    {
		if( frac_part > 0.5 )
		    val += 1.0;
		else if( frac_part <= -0.5 )
		    val -= 1.0;
		break;
	    }
            default:
	        ;
	}
    }

    val /= scale;
    c = val;
}

static
void
overflow( double& c, const scfx_params& params, bool& o_flag )
{
    int iwl = params.iwl();
    int fwl = params.wl() - iwl;
    double full_circle = scfx_pow2( iwl );
    double resolution = scfx_pow2( -fwl );
    double low, high;
    if( params.enc() == SC_TC_ )
    {
	high = full_circle / 2.0 - resolution;
	if( params.o_mode() == SC_SAT_SYM )
	    low = - high;
	else
	    low = - full_circle / 2.0;
    }
    else
    {
	low = 0.0;
	high = full_circle - resolution;
    }
    double val = c;
    sc_fxval_fast c2(c);

    bool under = ( val < low );
    bool over = ( val > high );

    o_flag = ( under || over );

    if( o_flag )
    {
        switch( params.o_mode() )
	{
            case SC_WRAP:			// wrap-around
	    {
		int n_bits = params.n_bits();

	        if( n_bits == 0 )
		{
		    // wrap-around all 'wl' bits
		    val -= floor( val / full_circle ) * full_circle;
		    if( val > high )
			val -= full_circle;
		}
		else if( n_bits < params.wl() )
		{
		    double X = scfx_pow2( iwl - n_bits );

		    // wrap-around least significant 'wl - n_bits' bits
		    val -= floor( val / X ) * X;
		    if( val > ( X - resolution ) )
			val -= X;
		    
		    // saturate most significant 'n_bits' bits
		    if( under )
		        val += low;
		    else
		    {
		        if( params.enc() == SC_TC_ )
			    val += full_circle / 2.0 - X;
			else
			    val += full_circle - X;
		    }
		}
		else
		{
		    // saturate all 'wl' bits
		    if( under )
			val = low;
		    else
			val = high;
		}
		break;
	    }
            case SC_SAT:			// saturation
            case SC_SAT_SYM:			// symmetrical saturation
	    {
	        if( under )
		    val = low;
		else
		    val = high;
		break;
	    }
            case SC_SAT_ZERO:			// saturation to zero
	    {
	        val = 0.0;
		break;
	    }
            case SC_WRAP_SM:			// sign magnitude wrap-around
	    {
		SC_ERROR_IF_( params.enc() == SC_US_,
			      sc_core::SC_ID_WRAP_SM_NOT_DEFINED_ );
	
		int n_bits = params.n_bits();

		if( n_bits == 0 )
		{
		    // invert conditionally
		    if( c2.get_bit( iwl ) != c2.get_bit( iwl - 1 ) )
			val = -val - resolution;

		    // wrap-around all 'wl' bits
		    val -= floor( val / full_circle ) * full_circle;
		    if( val > high )
			val -= full_circle;
		}
		else if( n_bits == 1 )
		{
		    // invert conditionally
		    if( c2.is_neg() != c2.get_bit( iwl - 1 ) )
			val = -val - resolution;

		    // wrap-around all 'wl' bits
		    val -= floor( val / full_circle ) * full_circle;
		    if( val > high )
			val -= full_circle;
		}
		else if( n_bits < params.wl() )
		{
		    // invert conditionally
		    if( c2.is_neg() == c2.get_bit( iwl - n_bits ) )
			val = -val - resolution;
		    
		    double X = scfx_pow2( iwl - n_bits );

		    // wrap-around least significant 'wl - n_bits' bits
		    val -= floor( val / X ) * X;
		    if( val > ( X - resolution ) )
			val -= X;

		    // saturate most significant 'n_bits' bits
		    if( under )
		        val += low;
		    else
			val += full_circle / 2.0 - X;
		} else {
		    // saturate all 'wl' bits
		    if( under )
			val = low;
		    else
			val = high;
		}
	        break;
	    }
            default:
	        ;
	}

	c = val;
    }
}


void
sc_fxnum_fast::cast()
{
    scfx_ieee_double id( m_val );
    SC_ERROR_IF_( id.is_nan() || id.is_inf(), sc_core::SC_ID_INVALID_FX_VALUE_);

    if( m_params.cast_switch() == SC_ON )
    {
        m_q_flag = false;
	m_o_flag = false;

	// check for special cases

	if( id.is_zero() )
	{
	    if( id.negative() != 0 )
	        m_val = -m_val;
	    return;
	}

	// perform casting

	sc_dt::quantization( m_val, m_params, m_q_flag );
	sc_dt::overflow( m_val, m_params, m_o_flag );

	// check for special case: -0

	id = m_val;
	if( id.is_zero() && id.negative() != 0 ) {
	    m_val = -m_val;
	}

	// check for special case: NaN of Inf

	if( id.is_nan() || id.is_inf() ) {
	    m_val = 0.0;
	}
    }
}


// defined in sc_fxval.cpp;
extern
const char*
to_string( const scfx_ieee_double&,
	   sc_numrep,
	   int,
	   sc_fmt,
	   const scfx_params* = 0 );


// explicit conversion to character string

const std::string
sc_fxnum_fast::to_string() const
{
    return std::string( sc_dt::to_string( m_val, SC_DEC, -1, SC_F, &m_params ) );
}

const std::string
sc_fxnum_fast::to_string( sc_numrep numrep ) const
{
    return std::string( sc_dt::to_string( m_val, numrep, -1, SC_F, &m_params ) );
}

const std::string
sc_fxnum_fast::to_string( sc_numrep numrep, bool w_prefix ) const
{
    return std::string( sc_dt::to_string( m_val, numrep, (w_prefix ? 1 : 0),
					SC_F, &m_params ) );
}

const std::string
sc_fxnum_fast::to_string( sc_fmt fmt ) const
{
    return std::string( sc_dt::to_string( m_val, SC_DEC, -1, fmt, &m_params ) );
}

const std::string
sc_fxnum_fast::to_string( sc_numrep numrep, sc_fmt fmt ) const
{
    return std::string( sc_dt::to_string( m_val, numrep, -1, fmt, &m_params ) );
}

const std::string
sc_fxnum_fast::to_string( sc_numrep numrep, bool w_prefix, sc_fmt fmt ) const
{
    return std::string( sc_dt::to_string( m_val, numrep, (w_prefix ? 1 : 0),
					fmt, &m_params ) );
}


const std::string
sc_fxnum_fast::to_dec() const
{
    return std::string( sc_dt::to_string( m_val, SC_DEC, -1, SC_F, &m_params ) );
}

const std::string
sc_fxnum_fast::to_bin() const
{
    return std::string( sc_dt::to_string( m_val, SC_BIN, -1, SC_F, &m_params ) );
}

const std::string
sc_fxnum_fast::to_oct() const
{
    return std::string( sc_dt::to_string( m_val, SC_OCT, -1, SC_F, &m_params ) );
}

const std::string
sc_fxnum_fast::to_hex() const
{
    return std::string( sc_dt::to_string( m_val, SC_HEX, -1, SC_F, &m_params ) );
}


// print or dump content

void
sc_fxnum_fast::print( ::std::ostream& os ) const
{
    os << sc_dt::to_string( m_val, SC_DEC, -1, SC_F, &m_params );
}

void
sc_fxnum_fast::scan( ::std::istream& is )
{
    std::string s;
    is >> s;
    *this = s.c_str();
}

void
sc_fxnum_fast::dump( ::std::ostream& os ) const
{
    os << "sc_fxnum_fast" << ::std::endl;
    os << "(" << ::std::endl;
    os << "val      = " << m_val << ::std::endl;
    os << "params   = ";
    m_params.dump( os );
    os << "q_flag   = " << m_q_flag << ::std::endl;
    os << "o_flag   = " << m_o_flag << ::std::endl;
    // TO BE COMPLETED
    // os << "observer = ";
    // if( m_observer != 0 )
    //     m_observer->dump( os );
    // else
    //     os << "0" << ::std::endl;
    os << ")" << ::std::endl;
}


// internal use only;
bool
sc_fxnum_fast::get_bit( int i ) const
{
    scfx_ieee_double id( m_val );
    if( id.is_zero() || id.is_nan() || id.is_inf() )
        return false;

    // convert to two's complement

    unsigned int m0 = id.mantissa0();
    unsigned int m1 = id.mantissa1();

    if( id.is_normal() )
        m0 += 1U << 20;

    if( id.negative() != 0 )
    {
	m0 = ~ m0;
	m1 = ~ m1;
	unsigned int tmp = m1;
	m1 += 1U;
	if( m1 <= tmp )
	    m0 += 1U;
    }

    // get the right bit

    int j = i - id.exponent();
    if( ( j += 20 ) >= 32 )
        return ( ( m0 & 1U << 31 ) != 0 );
    else if( j >= 0 )
        return ( ( m0 & 1U << j ) != 0 );
    else if( ( j += 32 ) >= 0 )
        return ( ( m1 & 1U << j ) != 0 );
    else
        return false;
}


bool
sc_fxnum_fast::set_bit( int i, bool high )
{
    scfx_ieee_double id( m_val );
    if( id.is_nan() || id.is_inf() )
        return false;

    if( high )
    {
	if( get_bit( i ) )
	    return true;

	if( m_params.enc() == SC_TC_ && i == m_params.iwl() - 1 )
	    m_val -= scfx_pow2( i );
	else
	    m_val += scfx_pow2( i );
    }
    else
    {
	if( ! get_bit( i ) )
	    return true;

	if( m_params.enc() == SC_TC_ && i == m_params.iwl() - 1 )
	    m_val += scfx_pow2( i );
	else
	    m_val -= scfx_pow2( i );
    }

    return true;
}


bool
sc_fxnum_fast::get_slice( int i, int j, sc_bv_base& bv ) const
{
    scfx_ieee_double id( m_val );
    if( id.is_nan() || id.is_inf() )
	return false;

    // convert to two's complement

    unsigned int m0 = id.mantissa0();
    unsigned int m1 = id.mantissa1();

    if( id.is_normal() )
        m0 += 1U << 20;

    if( id.negative() != 0 )
    {
	m0 = ~ m0;
	m1 = ~ m1;
	unsigned int tmp = m1;
	m1 += 1U;
	if( m1 <= tmp )
	    m0 += 1U;
    }

    // get the bits

    int l = j;
    for( int k = 0; k < bv.length(); ++ k )
    {
	bool b = false;

        int n = l - id.exponent();
        if( ( n += 20 ) >= 32 )
	    b = ( ( m0 & 1U << 31 ) != 0 );
	else if( n >= 0 )
	    b = ( ( m0 & 1U << n ) != 0 );
	else if( ( n += 32 ) >= 0 )
	    b = ( ( m1 & 1U << n ) != 0 );

	bv[k] = b;

	if( i >= j )
	    ++ l;
	else
	    -- l;
    }

    return true;
}

bool
sc_fxnum_fast::set_slice( int i, int j, const sc_bv_base& bv )
{
    scfx_ieee_double id( m_val );
    if( id.is_nan() || id.is_inf() )
        return false;

    // set the bits

    int l = j;
    for( int k = 0; k < bv.length(); ++ k )
    {
	if( bv[k].to_bool() )
	{
	    if( ! get_bit( l ) )
	    {
		if( m_params.enc() == SC_TC_ && l == m_params.iwl() - 1 )
		    m_val -= scfx_pow2( l );
		else
		    m_val += scfx_pow2( l );
	    }
	}
	else
	{
	    if( get_bit( l ) )
	    {
		if( m_params.enc() == SC_TC_ && l == m_params.iwl() - 1 )
		    m_val += scfx_pow2( l );
		else
		    m_val -= scfx_pow2( l );
	    }
	}


	if( i >= j )
	    ++ l;
	else
	    -- l;
    }

    return true;
}


sc_fxnum_fast_observer*
sc_fxnum_fast::lock_observer() const
{
    SC_ASSERT_( m_observer != 0, "lock observer failed" );
    sc_fxnum_fast_observer* tmp = m_observer;
    m_observer = 0;
    return tmp;
}

void
sc_fxnum_fast::unlock_observer( sc_fxnum_fast_observer* observer_ ) const
{
    SC_ASSERT_( observer_ != 0, "unlock observer failed" );
    m_observer = observer_;
}

} // namespace sc_dt


// Taf!
