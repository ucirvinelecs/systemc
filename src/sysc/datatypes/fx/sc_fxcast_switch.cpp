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

  sc_fxcast_switch.cpp - 

  Original Author: Martin Janssen, Synopsys, Inc.

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date: Gene Bushuyev, Synopsys, Inc.
  Description of Modification: - fix explicit instantiation syntax.
    
      Name, Affiliation, Date:
  Description of Modification: 

 *****************************************************************************/


// $Log: sc_fxcast_switch.cpp,v $
// Revision 1.1.1.1  2006/12/15 20:20:04  acg
// SystemC 2.3
//
// Revision 1.3  2006/01/13 18:53:57  acg
// Andy Goodrich: added $Log command so that CVS comments are reproduced in
// the source.
//

#include "sysc/datatypes/fx/sc_fxcast_switch.h"

//--------------------------------------------Farah is working here 

sc_dt::sc_fxcast_switch::sc_fxcast_switch() 
: m_sw()
{
    *this = sc_fxcast_context::default_value();
}


sc_dt::sc_fxcast_switch::sc_fxcast_switch( sc_switch sw_ )
: m_sw( sw_ )
{}


sc_dt::sc_fxcast_switch::sc_fxcast_switch( const sc_fxcast_switch& a )
: m_sw( a.m_sw )
{}


sc_dt::sc_fxcast_switch::sc_fxcast_switch( sc_without_context )
: m_sw( SC_DEFAULT_CAST_SWITCH_ )
{}

sc_dt::sc_fxcast_switch&
sc_dt::sc_fxcast_switch::operator = ( const sc_fxcast_switch& a )
{
    if( &a != this )
    {
        m_sw = a.m_sw;
    }
    return *this;
}

bool
sc_dt::operator == ( const sc_fxcast_switch& a, const sc_fxcast_switch& b )
{
    return ( a.m_sw == b.m_sw );
}


bool
sc_dt::operator != ( const sc_fxcast_switch& a, const sc_fxcast_switch& b )
{
    return ( a.m_sw != b.m_sw );
}


::std::ostream&
sc_dt::operator << ( ::std::ostream& os, const sc_fxcast_switch& a )
{
    a.print( os );
    return os;
}
//--------------------------------------------Farah is done working here
namespace sc_dt
{

// ----------------------------------------------------------------------------
//  CLASS : sc_fxcast_switch
//
//  Fixed-point cast switch class.
// ----------------------------------------------------------------------------

const std::string
sc_fxcast_switch::to_string() const
{
    return sc_dt::to_string( m_sw );
}


void
sc_fxcast_switch::print( ::std::ostream& os ) const
{
    os << sc_dt::to_string( m_sw );
}

void
sc_fxcast_switch::dump( ::std::ostream& os ) const
{
    os << "sc_fxcast_switch" << ::std::endl;
    os << "(" << ::std::endl;
    os << "sw = " << sc_dt::to_string( m_sw ) << ::std::endl;
    os << ")" << ::std::endl;
}

} // namespace sc_dt


// Taf!
