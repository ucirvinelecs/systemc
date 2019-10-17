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

  sc_fxtype_params.h - 

  Original Author: Martin Janssen, Synopsys, Inc.

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date:
  Description of Modification:

 *****************************************************************************/

// $Log: sc_fxtype_params.h,v $
// Revision 1.2  2011/08/24 22:05:43  acg
//  Torsten Maehne: initialization changes to remove warnings.
//
// Revision 1.1.1.1  2006/12/15 20:20:04  acg
// SystemC 2.3
//
// Revision 1.3  2006/01/13 18:53:58  acg
// Andy Goodrich: added $Log command so that CVS comments are reproduced in
// the source.
//

#ifndef SC_FXTYPE_PARAMS_H
#define SC_FXTYPE_PARAMS_H


#include "sysc/datatypes/fx/sc_context.h"


namespace sc_dt
{

// classes defined in this module
class sc_fxtype_params;


// ----------------------------------------------------------------------------
//  CLASS : sc_fxtype_params
//
//  Fixed-point type parameters class.
// ----------------------------------------------------------------------------

class sc_fxtype_params
{
public:

             sc_fxtype_params();
             sc_fxtype_params( int, int );
             sc_fxtype_params(           sc_q_mode, sc_o_mode, int = 0 );
             sc_fxtype_params( int, int, sc_q_mode, sc_o_mode, int = 0 );
             sc_fxtype_params( const sc_fxtype_params& );
	     sc_fxtype_params( const sc_fxtype_params&,
			       int, int );
	     sc_fxtype_params( const sc_fxtype_params&,
			                 sc_q_mode, sc_o_mode, int = 0 );
    explicit sc_fxtype_params( sc_without_context );

    sc_fxtype_params& operator = ( const sc_fxtype_params& );

    friend bool operator == ( const sc_fxtype_params&,
                              const sc_fxtype_params& );
    friend bool operator != ( const sc_fxtype_params&,
			      const sc_fxtype_params& );

    int wl() const;
    void wl( int );

    int iwl() const;
    void iwl( int );

    sc_q_mode q_mode() const;
    void q_mode( sc_q_mode );

    sc_o_mode o_mode() const;
    void o_mode( sc_o_mode );

    int n_bits() const;
    void n_bits( int );

    const std::string to_string() const;

    void print( ::std::ostream& = ::std::cout ) const;
    void dump( ::std::ostream& = ::std::cout ) const;

private:

    int       m_wl;
    int       m_iwl;
    sc_q_mode m_q_mode;
    sc_o_mode m_o_mode;
    int       m_n_bits;
};


// ----------------------------------------------------------------------------
//  TYPEDEF : sc_fxtype_context
//
//  Context type for the fixed-point type parameters.
// ----------------------------------------------------------------------------

typedef sc_context<sc_fxtype_params> sc_fxtype_context;


// IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII

bool
operator == ( const sc_fxtype_params& a, const sc_fxtype_params& b );

bool
operator != ( const sc_fxtype_params& a, const sc_fxtype_params& b );

::std::ostream&
operator << ( ::std::ostream& os, const sc_fxtype_params& a );
} // namespace sc_dt


#endif

// Taf!
