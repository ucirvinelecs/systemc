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

  sc_fxtype_params.cpp - 

  Original Author: Martin Janssen, Synopsys, Inc.

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date:
  Description of Modification:

 *****************************************************************************/


// $Log: sc_fxtype_params.cpp,v $
// Revision 1.1.1.1  2006/12/15 20:20:04  acg
// SystemC 2.3
//
// Revision 1.3  2006/01/13 18:53:58  acg
// Andy Goodrich: added $Log command so that CVS comments are reproduced in
// the source.
//

#include "sysc/datatypes/fx/sc_fxtype_params.h"
//------------------------------------------------Farah is working here /


sc_dt::sc_fxtype_params::sc_fxtype_params() 
: m_wl(), m_iwl(), m_q_mode(), m_o_mode(), m_n_bits()
{
    *this = sc_fxtype_context::default_value();
}


sc_dt::sc_fxtype_params::sc_fxtype_params( int wl_, int iwl_ )
: m_wl(), m_iwl(), m_q_mode(), m_o_mode(), m_n_bits()
{
    *this = sc_fxtype_context::default_value();

    SC_CHECK_WL_( wl_ );
    m_wl  = wl_;
    m_iwl = iwl_;
}


sc_dt::sc_fxtype_params::sc_fxtype_params( sc_q_mode q_mode_,
                                    sc_o_mode o_mode_, int n_bits_ )
: m_wl(), m_iwl(), m_q_mode(), m_o_mode(), m_n_bits()
{
    *this = sc_fxtype_context::default_value();

    SC_CHECK_N_BITS_( n_bits_ );
    m_q_mode = q_mode_;
    m_o_mode = o_mode_;
    m_n_bits = n_bits_;
}


sc_dt::sc_fxtype_params::sc_fxtype_params( int wl_, int iwl_,
                                    sc_q_mode q_mode_,
                                    sc_o_mode o_mode_, int n_bits_ )
: m_wl(), m_iwl(), m_q_mode(), m_o_mode(), m_n_bits()
{
    SC_CHECK_WL_( wl_ );
    SC_CHECK_N_BITS_( n_bits_ );
    m_wl     = wl_;
    m_iwl    = iwl_;
    m_q_mode = q_mode_;
    m_o_mode = o_mode_;
    m_n_bits = n_bits_;
}


sc_dt::sc_fxtype_params::sc_fxtype_params( const sc_fxtype_params& a )
: m_wl( a.m_wl ), m_iwl( a.m_iwl ),
  m_q_mode( a.m_q_mode ),
  m_o_mode( a.m_o_mode ), m_n_bits( a.m_n_bits )
{}


sc_dt::sc_fxtype_params::sc_fxtype_params( const sc_fxtype_params& a,
				    int wl_, int iwl_ )
: m_wl( wl_ ), m_iwl( iwl_ ),
  m_q_mode( a.m_q_mode ),
  m_o_mode( a.m_o_mode ), m_n_bits( a.m_n_bits )
{}


sc_dt::sc_fxtype_params::sc_fxtype_params( const sc_fxtype_params& a,
				    sc_q_mode q_mode_,
				    sc_o_mode o_mode_, int n_bits_ )
: m_wl( a.m_wl ), m_iwl( a.m_iwl ),
  m_q_mode( q_mode_ ),
  m_o_mode( o_mode_ ), m_n_bits( n_bits_ )
{}


sc_dt::sc_fxtype_params::sc_fxtype_params( sc_without_context )
: m_wl    ( SC_DEFAULT_WL_ ),
  m_iwl   ( SC_DEFAULT_IWL_ ),
  m_q_mode( SC_DEFAULT_Q_MODE_ ),
  m_o_mode( SC_DEFAULT_O_MODE_ ),
  m_n_bits( SC_DEFAULT_N_BITS_ )
{}


sc_dt::sc_fxtype_params&
sc_dt::sc_fxtype_params::operator = ( const sc_fxtype_params& a )
{
    if( &a != this )
    {
        m_wl     = a.m_wl;
	m_iwl    = a.m_iwl;
	m_q_mode = a.m_q_mode;
	m_o_mode = a.m_o_mode;
	m_n_bits = a.m_n_bits;
    }
    return *this;
}

// the two operator functions go here


int
sc_dt::sc_fxtype_params::wl() const
{
    return m_wl;
}


void
sc_dt::sc_fxtype_params::wl( int wl_ )
{
    SC_CHECK_WL_( wl_ );
    m_wl = wl_;
}



int
sc_dt::sc_fxtype_params::iwl() const
{
    return m_iwl;
}


void
sc_dt::sc_fxtype_params::iwl( int iwl_ )
{
    m_iwl = iwl_;
}



sc_dt::sc_q_mode
sc_dt::sc_fxtype_params::q_mode() const
{
    return m_q_mode;
}


void
sc_dt::sc_fxtype_params::q_mode( sc_q_mode q_mode_ )
{
    m_q_mode = q_mode_;
}



sc_dt::sc_o_mode
sc_dt::sc_fxtype_params::o_mode() const
{
    return m_o_mode;
}


void
sc_dt::sc_fxtype_params::o_mode( sc_o_mode o_mode_ )
{
    m_o_mode = o_mode_;
}



int
sc_dt::sc_fxtype_params::n_bits() const
{
    return m_n_bits;
}


void
sc_dt::sc_fxtype_params::n_bits( int n_bits_ )
{
    SC_CHECK_N_BITS_( n_bits_ );
    m_n_bits = n_bits_;
}

bool
sc_dt::operator == ( const sc_fxtype_params& a, const sc_fxtype_params& b )
{
    return ( a.m_wl     == b.m_wl     &&
	     a.m_iwl    == b.m_iwl    &&
	     a.m_q_mode == b.m_q_mode &&
	     a.m_o_mode == b.m_o_mode &&
	     a.m_n_bits == b.m_n_bits );
}


bool
sc_dt::operator != ( const sc_fxtype_params& a, const sc_fxtype_params& b )
{
    return ( a.m_wl     != b.m_wl     ||
	     a.m_iwl    != b.m_iwl    ||
	     a.m_q_mode != b.m_q_mode ||
	     a.m_o_mode != b.m_o_mode ||
	     a.m_n_bits != b.m_n_bits );
}

::std::ostream&
sc_dt::operator << ( ::std::ostream& os, const sc_fxtype_params& a )
{
    a.print( os );
    return os;
}

//-----------------------------------------------Farah is done working here
namespace sc_dt
{

// ----------------------------------------------------------------------------
//  CLASS : sc_fxtype_params
//
//  Fixed-point type parameters class.
// ----------------------------------------------------------------------------

const std::string
sc_fxtype_params::to_string() const
{
    std::string s;

    char buf[BUFSIZ];

    s += "(";
    std::sprintf( buf, "%d", m_wl );
    s += buf;
    s += ",";
    std::sprintf( buf, "%d", m_iwl );
    s += buf;
    s += ",";
    s += sc_dt::to_string( m_q_mode );
    s += ",";
    s += sc_dt::to_string( m_o_mode );
    s += ",";
    std::sprintf( buf, "%d", m_n_bits );
    s += buf;
    s += ")";

    return s;
}


void
sc_fxtype_params::print( ::std::ostream& os ) const
{
    os << to_string();
}

void
sc_fxtype_params::dump( ::std::ostream& os ) const
{
    os << "sc_fxtype_params" << ::std::endl;
    os << "(" << ::std::endl;
    os << "wl     = " << m_wl << ::std::endl;
    os << "iwl    = " << m_iwl << ::std::endl;
    os << "q_mode = " << m_q_mode << ::std::endl;
    os << "o_mode = " << m_o_mode << ::std::endl;
    os << "n_bits = " << m_n_bits << ::std::endl;
    os << ")" << ::std::endl;
}

} // namespace sc_dt


// Taf!
