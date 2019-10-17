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

    sc_concatref.cpp -- Concatenation support.

    Original Author: Andy Goodrich, Forte Design Systems, Inc.

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date:
  Description of Modification:

 *****************************************************************************/

// $Log: sc_concatref.cpp,v $
// Revision 1.1.1.1  2006/12/15 20:20:05  acg
// SystemC 2.3
//
// Revision 1.3  2006/01/13 18:54:01  acg
// Andy Goodrich: added $Log command so that CVS comments are reproduced in
// the source.
//

#include "sysc/datatypes/misc/sc_concatref.h"
#include "sysc/utils/sc_temporary.h"
//-----------------------------------------------Farah is working here
sc_dt::sc_concatref::~sc_concatref()
{}

// capacity

unsigned int sc_dt::sc_concatref::length() const
    { return m_len; }

// concatenation

int sc_dt::sc_concatref::concat_length( bool* xz_present_p ) const
{ 
    if ( xz_present_p ) 
        *xz_present_p = m_flags & cf_xz_present ? true : false;
    return m_len; 
}

void sc_dt::sc_concatref::concat_clear_data( bool to_ones )
{ 
    m_left_p->concat_clear_data(to_ones); 
    m_right_p->concat_clear_data(to_ones); 
}

bool sc_dt::sc_concatref::concat_get_ctrl( sc_digit* dst_p, int low_i ) const
{
    bool rnz = m_right_p->concat_get_ctrl( dst_p, low_i );
    bool lnz = m_left_p->concat_get_ctrl( dst_p, low_i+m_len_r );
    return rnz || lnz;
}

bool sc_dt::sc_concatref::concat_get_data( sc_digit* dst_p, int low_i ) const
{
    bool rnz = m_right_p->concat_get_data( dst_p, low_i );
    bool lnz = m_left_p->concat_get_data( dst_p, low_i+m_len_r );
    return rnz || lnz;
}

sc_dt::uint64 sc_dt::sc_concatref::concat_get_uint64() const
{
    if ( m_len_r >= 64 )
        return m_right_p->concat_get_uint64();
    else
    {
        return (m_left_p->concat_get_uint64() << m_len_r) | 
            m_right_p->concat_get_uint64();
    }
}

void sc_dt::sc_concatref::concat_set( int64 src, int low_i ) 
{ 
    m_right_p->concat_set( src, low_i );
    m_left_p->concat_set( src, low_i+m_len_r);
}

void sc_dt::sc_concatref::concat_set( const sc_signed& src, int low_i ) 
{
    m_right_p->concat_set( src, low_i );
    m_left_p->concat_set( src, low_i+m_len_r);
}

void sc_dt::sc_concatref::concat_set( const sc_unsigned& src, int low_i ) 
{ 
    m_right_p->concat_set( src, low_i );
    m_left_p->concat_set( src, low_i+m_len_r);
}

void sc_dt::sc_concatref::concat_set( uint64 src, int low_i )
{ 
    m_right_p->concat_set( src, low_i );
    m_left_p->concat_set( src, low_i+m_len_r);
}


// explicit conversions

sc_dt::uint64 sc_dt::sc_concatref::to_uint64() const 
    {
        uint64 mask;
        uint64 result;

        result = m_right_p->concat_get_uint64();
        if ( m_len_r < 64 )
        {
            mask = (uint64)~0;
            result = (m_left_p->concat_get_uint64() << m_len_r) | 
                        (result & ~(mask << m_len_r));
        }
        if ( m_len < 64 )
        {
            mask = (uint64)~0;
            result = result & ~(mask << m_len);
        }
        return result;
    }

sc_dt::int64 sc_dt::sc_concatref::to_int64() const
    { 
        return (int64)to_uint64();
    }
int sc_dt::sc_concatref::to_int() const
{ return (int)to_int64(); }

unsigned int  sc_dt::sc_concatref::to_uint() const
{ return (unsigned int)to_uint64(); }

long sc_dt::sc_concatref::to_long() const
{ return (long)to_int64(); }

unsigned long sc_dt::sc_concatref::to_ulong() const
{ return (unsigned long)to_uint64(); }

double sc_dt::sc_concatref::to_double() const
{ return value().to_double(); }


void sc_dt::sc_concatref::to_sc_signed( sc_signed& target ) const
{ target = value(); }


void sc_dt::sc_concatref::to_sc_unsigned( sc_unsigned& target ) const
{ target = value(); }

sc_dt::sc_concatref::operator  uint64 () const 
    { return to_uint64(); }

sc_dt::sc_concatref::operator const sc_unsigned& () const
    { return value(); }

sc_dt::sc_unsigned sc_dt::sc_concatref::operator + () const
    { return value(); } 

sc_dt::sc_signed sc_dt::sc_concatref::operator - () const
    { return -value(); } 

sc_dt::sc_unsigned sc_dt::sc_concatref::operator ~ () const
    { return ~value(); } 

// explicit conversion to character string

const std::string sc_dt::sc_concatref::to_string( sc_numrep numrep ) const
    { return value().to_string(numrep); }

const std::string sc_dt::sc_concatref::to_string( sc_numrep numrep, bool w_prefix ) const
    { return value().to_string(numrep,w_prefix); }

const sc_dt::sc_concatref& sc_dt::sc_concatref::operator = ( const sc_concatref& v )
{
    sc_unsigned temp(v.length());
    temp = v.value();
    m_right_p->concat_set(temp, 0);
    m_left_p->concat_set(temp, m_len_r);
    return *this;
}

const sc_dt::sc_concatref& sc_dt::sc_concatref::operator = ( const sc_signed& v )
{
    m_right_p->concat_set(v, 0);
    m_left_p->concat_set(v, m_len_r);
    return *this;
}

const sc_dt::sc_concatref& sc_dt::sc_concatref::operator = ( const sc_unsigned& v )
{
    m_right_p->concat_set(v, 0);
    m_left_p->concat_set(v, m_len_r);
    return *this;
}

const sc_dt::sc_concatref& sc_dt::sc_concatref::operator = ( const char* v_p )
{
    sc_unsigned v(m_len);
    v = v_p;
    m_right_p->concat_set(v, 0);
    m_left_p->concat_set(v, m_len_r);
    return *this;
}

const sc_dt::sc_concatref& sc_dt::sc_concatref::operator = ( const sc_bv_base& v )
{
    sc_unsigned temp(v.length());
    temp = v;
    m_right_p->concat_set(temp, 0);
    m_left_p->concat_set(temp, m_len_r);
    return *this;
}

const sc_dt::sc_concatref& sc_dt::sc_concatref::operator = ( const sc_lv_base& v )
{
    sc_unsigned data(v.length());
    data = v;
    m_right_p->concat_set(data, 0);
    m_left_p->concat_set(data, m_len_r);
    return *this;
}


// reduce methods

bool sc_dt::sc_concatref::and_reduce() const
    { return value().and_reduce(); }

bool sc_dt::sc_concatref::nand_reduce() const
    { return value().nand_reduce(); }

bool sc_dt::sc_concatref::or_reduce() const
    { return value().or_reduce(); }

bool sc_dt::sc_concatref::nor_reduce() const
    { return value().nor_reduce(); }

bool sc_dt::sc_concatref::xor_reduce() const
    { return value().xor_reduce(); }

bool sc_dt::sc_concatref::xnor_reduce() const
    { return value().xnor_reduce(); }

// other methods

void sc_dt::sc_concatref::print( ::std::ostream& os) const
    { os << this->value(); }

void sc_dt::sc_concatref::scan( ::std::istream& is ) 
{ 
    std::string s; 
    is >> s; 
    *this = s.c_str(); 
} 

sc_dt::sc_concatref::sc_concatref() : m_left_p(0), m_right_p(0), m_len(0), m_len_r(0), m_flags()
  {}


bool
sc_dt::and_reduce( const sc_concatref& a )
{
    return a.and_reduce();
}

bool
sc_dt::nand_reduce( const sc_concatref& a )
{
    return a.nand_reduce();
}

bool
sc_dt::or_reduce( const sc_concatref& a )
{
    return a.or_reduce();
}

bool
sc_dt::nor_reduce( const sc_concatref& a )
{
    return a.nor_reduce();
}

bool
sc_dt::xor_reduce( const sc_concatref& a )
{
    return a.xor_reduce();
}

bool
sc_dt::xnor_reduce( const sc_concatref& a )
{
    return a.xnor_reduce();
}


const sc_dt::sc_unsigned sc_dt::operator << (const sc_concatref& target, uint64 shift)
{
    return target.value() << (int)shift;
}

const sc_dt::sc_unsigned sc_dt::operator << (const sc_concatref& target, int64 shift)
{
    return target.value() << (int)shift;
}

const sc_dt::sc_unsigned sc_dt::operator << ( 
    const sc_concatref& target, unsigned long shift )
{
    return target.value() << (int)shift;
}

const sc_dt::sc_unsigned sc_dt::operator << ( 
    const sc_concatref& target, int shift )
{
    return target.value() << shift;
}

const sc_dt::sc_unsigned sc_dt::operator << ( 
    const sc_concatref& target, unsigned int shift )
{
    return target.value() << (int)shift;
}

const sc_dt::sc_unsigned sc_dt::operator << ( const sc_concatref& target, long shift )
{
    return target.value() << (int)shift;
}

const sc_dt::sc_unsigned sc_dt::operator >> (const sc_concatref& target, uint64 shift)
{
    return target.value() >> (int)shift;
}

const sc_dt::sc_unsigned sc_dt::operator >> (const sc_concatref& target, int64 shift)
{
    return target.value() >> (int)shift;
}

const sc_dt::sc_unsigned sc_dt::operator >> ( 
    const sc_concatref& target, unsigned long shift )
{
    return target.value() >> (int)shift;
}

const sc_dt::sc_unsigned sc_dt::operator >> ( 
    const sc_concatref& target, int shift )
{
    return target.value() >> shift;
}

const sc_dt::sc_unsigned sc_dt::operator >> ( 
    const sc_concatref& target, unsigned int shift )
{
    return target.value() >> (int)shift;
}

const sc_dt::sc_unsigned sc_dt::operator >> ( const sc_concatref& target, long shift )
{
    return target.value() >> (int)shift;
}


// STREAM OPERATORS FOR sc_concatref OBJECT INSTANCES:


::std::ostream&
sc_dt::operator << ( ::std::ostream& os, const sc_concatref& v )
{ 
    return os << v.value();
}


::std::istream&
sc_dt::operator >> ( ::std::istream& is, sc_concatref& a )
{
    sc_unsigned temp(a.concat_length(0));   
    temp.scan( is );
    a = temp;
    return is;
}

sc_dt::sc_concat_bool::sc_concat_bool()
: sc_value_base(), m_value()
{}

sc_dt::sc_concat_bool::~sc_concat_bool() 
    { }

int sc_dt::sc_concat_bool::concat_length( bool* xz_present_p ) const
{ 
    if ( xz_present_p ) *xz_present_p = false;
    return 1; 
}

bool sc_dt::sc_concat_bool::concat_get_ctrl( sc_digit* dst_p, int low_i ) const
{
    int bit = 1 << (low_i % BITS_PER_DIGIT); 
    int word_i = low_i / BITS_PER_DIGIT;
    dst_p[word_i] &= ~bit;
    return false;
}

bool sc_dt::sc_concat_bool::concat_get_data( sc_digit* dst_p, int low_i ) const
{
    int bit = 1 << (low_i % BITS_PER_DIGIT); 
    int word_i = low_i / BITS_PER_DIGIT;
    if ( m_value )
        dst_p[word_i] |= bit;
    else 
        dst_p[word_i] &= ~bit;
    return m_value;
}

sc_dt::uint64 sc_dt::sc_concat_bool::concat_get_uint64() const
{
    return m_value ? 1 : 0;
}

// ----------------------------------------------------------------------------
// CONCATENATION FUNCTION AND OPERATOR FOR STANDARD SYSTEM C DATA TYPES:
// ----------------------------------------------------------------------------

sc_dt::sc_concatref& sc_dt::concat(
    sc_dt::sc_value_base& a, sc_dt::sc_value_base& b)
{
    sc_dt::sc_concatref* result_p;     // Proxy for the concatenation.

    result_p = sc_dt::sc_concatref::m_pool.allocate();
    result_p->initialize( a, b );
    return *result_p;
}


const
sc_dt::sc_concatref& sc_dt::concat(
    const sc_dt::sc_value_base& a, const sc_dt::sc_value_base& b)
{
    sc_dt::sc_concatref* result_p;     // Proxy for the concatenation.

    result_p = sc_dt::sc_concatref::m_pool.allocate();
    result_p->initialize( a, b );
    return *result_p;
}


const
sc_dt::sc_concatref& sc_dt::concat(const sc_dt::sc_value_base& a, bool b)
{
    const sc_dt::sc_concat_bool* b_p;        // Proxy for boolean value.
    sc_dt::sc_concatref*         result_p;   // Proxy for the concatenation.

    b_p = sc_dt::sc_concat_bool::allocate(b);
    result_p = sc_dt::sc_concatref::m_pool.allocate();
    result_p->initialize( a, *b_p );
    return *result_p;
}


const
sc_dt::sc_concatref& sc_dt::concat(bool a, const sc_dt::sc_value_base& b)
{
    const sc_dt::sc_concat_bool* a_p;        // Proxy for boolean value.
    sc_dt::sc_concatref*         result_p;   // Proxy for the concatenation.

    a_p = sc_dt::sc_concat_bool::allocate(a);
    result_p = sc_dt::sc_concatref::m_pool.allocate();
    result_p->initialize( *a_p, b );
    return *result_p;
}

sc_dt::sc_concatref& sc_dt::operator , (
    sc_dt::sc_value_base& a, sc_dt::sc_value_base& b)
{
    sc_dt::sc_concatref* result_p;     // Proxy for the concatenation.

    result_p = sc_dt::sc_concatref::m_pool.allocate();
    result_p->initialize( a, b );
    return *result_p;
}

const
sc_dt::sc_concatref& sc_dt::operator , (
    const sc_dt::sc_value_base& a, const sc_dt::sc_value_base& b)
{
    sc_dt::sc_concatref* result_p;     // Proxy for the concatenation.

    result_p = sc_dt::sc_concatref::m_pool.allocate();
    result_p->initialize( a, b );
    return *result_p;
}

const
sc_dt::sc_concatref& sc_dt::operator , (const sc_dt::sc_value_base& a, bool b)
{
    const sc_dt::sc_concat_bool* b_p;      // Proxy for boolean value.
    sc_dt::sc_concatref*         result_p; // Proxy for the concatenation.

    b_p = sc_dt::sc_concat_bool::allocate(b);
    result_p = sc_dt::sc_concatref::m_pool.allocate();
    result_p->initialize( a, *b_p );
    return *result_p;
}

const
sc_dt::sc_concatref& sc_dt::operator , (bool a, const sc_dt::sc_value_base& b)
{
    const sc_dt::sc_concat_bool* a_p;      // Proxy for boolean value.
    sc_dt::sc_concatref*         result_p; // Proxy for the concatenation.

    a_p = sc_dt::sc_concat_bool::allocate(a);
    result_p = sc_dt::sc_concatref::m_pool.allocate();
    result_p->initialize( *a_p, b );
    return *result_p;
}

void sc_dt::sc_concatref::initialize( 
    sc_value_base& left, sc_value_base& right )
    {    
        bool left_xz;   // true if x's and/or z's found in left.
        bool right_xz;  // True if x's and/or z's found in right.
        
        m_left_p = (sc_value_base*)&left;
        m_right_p = (sc_value_base*)&right;
        m_len_r = right.concat_length(&right_xz);
        m_len = left.concat_length(&left_xz) + m_len_r;
        m_flags = ( left_xz || right_xz ) ? cf_xz_present : cf_none;
    }


void sc_dt::sc_concatref::initialize( 
    const sc_value_base& left, const sc_value_base& right )
    {    
        bool left_xz;   // True if x's and/or z's found in left.
        bool right_xz;  // True if x's and/or z's found in right.

        m_left_p = (sc_value_base*)&left;
        m_right_p = (sc_value_base*)&right;
        m_len_r = right.concat_length(&right_xz);
        m_len = left.concat_length(&left_xz) + m_len_r;
        m_flags = ( left_xz || right_xz ) ? cf_xz_present : cf_none;
    }


const sc_dt::sc_concatref& sc_dt::sc_concatref::operator = ( int v )
{
    m_right_p->concat_set((int64)v, 0);
    m_left_p->concat_set((int64)v, m_len_r);
    return *this;
}

const sc_dt::sc_concatref& sc_dt::sc_concatref::operator = ( long v )
{
    m_right_p->concat_set((int64)v, 0);
    m_left_p->concat_set((int64)v, m_len_r);
    return *this;
}

const sc_dt::sc_concatref& sc_dt::sc_concatref::operator = ( int64 v )
{
    m_right_p->concat_set(v, 0);
    m_left_p->concat_set(v, m_len_r);
    return *this;
}

const sc_dt::sc_concatref& sc_dt::sc_concatref::operator = ( unsigned int v )
{
    m_right_p->concat_set((uint64)v, 0);
    m_left_p->concat_set((uint64)v, m_len_r);
    return *this;
}

const sc_dt::sc_concatref& sc_dt::sc_concatref::operator = ( unsigned long v )
{
    m_right_p->concat_set((uint64)v, 0);
    m_left_p->concat_set((uint64)v, m_len_r);
    return *this;
}

const sc_dt::sc_concatref& sc_dt::sc_concatref::operator = ( uint64 v )
{
    m_right_p->concat_set(v, 0);
    m_left_p->concat_set(v, m_len_r);
    return *this;
}

sc_dt::sc_concat_bool* sc_dt::sc_concat_bool::allocate( bool v )
{
    sc_concat_bool* result_p = m_pool.allocate();
    result_p->m_value = v;
    return result_p;
}
//-------------------------------------------Farah is done working here
// STORAGE POOLS USED BY sc_concatref:

namespace sc_dt {
    sc_core::sc_vpool<sc_concat_bool> sc_concat_bool::m_pool(9);
    sc_core::sc_vpool<sc_concatref>   sc_concatref::m_pool(9);
} // namespace sc_dt

namespace sc_core {
    sc_byte_heap             sc_temp_heap(0x300000);
} // namespace sc_core
