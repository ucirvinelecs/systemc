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

  scfx_rep.h - 

  Original Author: Robert Graulich, Synopsys, Inc.
                   Martin Janssen,  Synopsys, Inc.

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date:
  Description of Modification:

 *****************************************************************************/

// $Log: scfx_rep.h,v $
// Revision 1.6  2011/08/24 22:05:43  acg
//  Torsten Maehne: initialization changes to remove warnings.
//
// Revision 1.5  2011/07/25 10:20:29  acg
//  Andy Goodrich: check in aftermath of call to automake.
//
// Revision 1.4  2010/12/07 20:09:08  acg
// Andy Goodrich: Philipp Hartmann's constructor disambiguation fix
//
// Revision 1.3  2010/08/03 15:54:52  acg
//  Andy Goodrich: formatting.
//
// Revision 1.2  2010/03/15 18:29:01  acg
//  Andy Goodrich: Moved default argument specifications from friend
//  declarations to the actual function signatures.
//
// Revision 1.1.1.1  2006/12/15 20:20:04  acg
// SystemC 2.3
//
// Revision 1.4  2006/03/13 20:24:27  acg
//  Andy Goodrich: Addition of function declarations, e.g., neg_scfx_rep(),
//  to keep gcc 4.x happy.
//
// Revision 1.3  2006/01/13 18:53:58  acg
// Andy Goodrich: added $Log command so that CVS comments are reproduced in
// the source.
//

#ifndef SCFX_REP_H
#define SCFX_REP_H


#include <climits>

#include "sysc/datatypes/fx/scfx_mant.h"
#include "sysc/datatypes/fx/scfx_params.h"
#include "sysc/datatypes/fx/scfx_string.h"


namespace sc_dt
{

// classes defined in this module
class scfx_index;
class scfx_rep;

// forward class declarations
class sc_bv_base;
class sc_signed;
class sc_unsigned;

// function declarations
void multiply( scfx_rep&, const scfx_rep&, const scfx_rep&, 
	       int max_wl = SC_DEFAULT_MAX_WL_ );
scfx_rep*  neg_scfx_rep( const scfx_rep& );
scfx_rep*  mult_scfx_rep( const scfx_rep&, const scfx_rep&, 
	                  int max_wl = SC_DEFAULT_MAX_WL_ );
scfx_rep*  div_scfx_rep( const scfx_rep&, const scfx_rep&, 
	                 int max_wl = SC_DEFAULT_DIV_WL_ );
scfx_rep*  add_scfx_rep( const scfx_rep&, const scfx_rep&, 
	                 int max_wl = SC_DEFAULT_MAX_WL_ );
scfx_rep*  sub_scfx_rep( const scfx_rep&, const scfx_rep&, 
	                 int max_wl = SC_DEFAULT_MAX_WL_ );
scfx_rep*  lsh_scfx_rep( const scfx_rep&, int );
scfx_rep*  rsh_scfx_rep( const scfx_rep&, int );
int        cmp_scfx_rep( const scfx_rep&, const scfx_rep& );


const int min_mant = 4;

const int bits_in_int  = sizeof(int)  * CHAR_BIT;
const int bits_in_word = sizeof(word) * CHAR_BIT;

// 08/03/2015 GL: scoped mutex for static scfx_rep_node* list
struct scfx_rep_list_lock {
    static pthread_mutex_t m_mutex;
    explicit scfx_rep_list_lock();
    ~scfx_rep_list_lock();
};

// 08/03/2015 GL: scoped mutex for static scfx_pow10 pow10_fx
struct scfx_rep_pow10_fx_lock {
    static pthread_mutex_t m_mutex;
    explicit scfx_rep_pow10_fx_lock();
    ~scfx_rep_pow10_fx_lock();
};

// 08/04/2015 GL: scoped mutex for scfx_rep::to_string
struct scfx_rep_scfx_string_lock {
    static pthread_mutex_t m_mutex;
    explicit scfx_rep_scfx_string_lock();
    ~scfx_rep_scfx_string_lock();
};


// ----------------------------------------------------------------------------
//  CLASS : scfx_index
// ----------------------------------------------------------------------------

class scfx_index
{

public:

    scfx_index( int wi_, int bi_ );

    int wi() const;
    int bi() const;

    void wi( int wi_ );

private:

    int m_wi;
    int m_bi;

};


// ----------------------------------------------------------------------------
//  CLASS : scfx_rep
//
//  Arbitrary-precision fixed-point implementation class.
// ----------------------------------------------------------------------------

class scfx_rep
{
    enum state
    {
        normal,
        infinity,
        not_a_number
    };

public:

    // constructors

             scfx_rep();
    explicit scfx_rep( int );
    explicit scfx_rep( unsigned int );
    explicit scfx_rep( long );
    explicit scfx_rep( unsigned long );
    explicit scfx_rep( double );
    explicit scfx_rep( const char* );
    explicit scfx_rep( int64 );
    explicit scfx_rep( uint64 );
    explicit scfx_rep( const sc_signed& );
    explicit scfx_rep( const sc_unsigned& );


    // copy constructor

             scfx_rep( const scfx_rep& );


    // destructor

    ~scfx_rep();


    void* operator new( std::size_t );
    void  operator delete( void*, std::size_t );


    void from_string( const char*, int );

    double to_double() const;

    const char* to_string( sc_numrep,
			   int,
			   sc_fmt,
			   const scfx_params* = 0 ) const;


    // assignment operator

    void operator = ( const scfx_rep& );

    friend void multiply( scfx_rep&, const scfx_rep&, const scfx_rep&, int );

    friend scfx_rep* neg_scfx_rep( const scfx_rep& );
    friend scfx_rep* mult_scfx_rep( const scfx_rep&, const scfx_rep&, int );
    friend scfx_rep* div_scfx_rep( const scfx_rep&, const scfx_rep&, int );
    friend scfx_rep* add_scfx_rep( const scfx_rep&, const scfx_rep&, int );
    friend scfx_rep* sub_scfx_rep( const scfx_rep&, const scfx_rep&, int );
    friend scfx_rep* lsh_scfx_rep( const scfx_rep&, int );
    friend scfx_rep* rsh_scfx_rep( const scfx_rep&, int );

    void lshift( int );
    void rshift( int );

    friend int        cmp_scfx_rep( const scfx_rep&, const scfx_rep& );

    void cast( const scfx_params&, bool&, bool& );

    bool is_neg() const;
    bool is_zero() const;
    bool is_nan() const;
    bool is_inf() const;
    bool is_normal() const;

    void set_zero( int = 1 );
    void set_nan();
    void set_inf( int );

    bool   get_bit( int ) const;
    bool   set( int, const scfx_params& );
    bool clear( int, const scfx_params& );

    bool get_slice( int, int, const scfx_params&, sc_bv_base& ) const;
    bool set_slice( int, int, const scfx_params&, const sc_bv_base& );

    void print( ::std::ostream& ) const;
    void dump( ::std::ostream& ) const;

    void get_type( int&, int&, sc_enc& ) const;

    friend scfx_rep* quantization_scfx_rep( const scfx_rep&,
					     const scfx_params&,
					     bool& );
    friend scfx_rep*     overflow_scfx_rep( const scfx_rep&,
					     const scfx_params&,
					     bool& );

    bool rounding_flag() const;

private:

    friend void  align( const scfx_rep&, const scfx_rep&, int&, int&,
			scfx_mant_ref&, scfx_mant_ref& );
    friend int   compare_msw( const scfx_rep&, const scfx_rep& );
    friend int   compare_msw_ff( const scfx_rep& lhs, const scfx_rep& rhs );
    unsigned int divide_by_ten();
    int          find_lsw() const;
    int          find_msw() const;
    void         find_sw();
    void         multiply_by_ten();
    void         normalize( int );
    scfx_mant*   resize( int, int ) const;
    void         set_bin( int );
    void         set_oct( int, int );
    void         set_hex( int, int );
    void         shift_left( int );
    void         shift_right( int );

    const scfx_index calc_indices( int ) const;

    void o_extend( const scfx_index&, sc_enc );
    bool o_bit_at( const scfx_index& ) const;
    bool o_zero_left( const scfx_index& ) const;
    bool o_zero_right( const scfx_index& ) const;
    void o_set_low( const scfx_index&, sc_enc );
    void o_set_high( const scfx_index&, const scfx_index&, sc_enc, int = 1 );
    void o_set( const scfx_index&, const scfx_index&, sc_enc, bool );
    void o_invert( const scfx_index& );
    bool q_bit( const scfx_index& ) const;
    void q_clear( const scfx_index& );
    void q_incr( const scfx_index& );
    bool q_odd( const scfx_index& ) const;
    bool q_zero( const scfx_index& ) const;

    void resize_to( int, int = 0 );
    int  size() const;
    void toggle_tc();

    friend void print_dec( scfx_string&, const scfx_rep&, int, sc_fmt );
    friend void print_other( scfx_string&, const scfx_rep&, sc_numrep, int,
			     sc_fmt, const scfx_params* );

    void quantization( const scfx_params&, bool& );
    void     overflow( const scfx_params&, bool& );

    friend int compare_abs( const scfx_rep&, const scfx_rep& );

    void round( int );

private:

    scfx_mant m_mant;     // mantissa (bits of the value).
    int       m_wp;       // index of highest order word in value.
    int       m_sign;     // sign of value.
    state     m_state;    // value state, e.g., normal, inf, etc.
    int       m_msw;      // index of most significant non-zero word.
    int       m_lsw;      // index of least significant non-zero word.
    bool      m_r_flag;   // true if founding occurred.

};


// IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII


scfx_rep*
quantization_scfx_rep( const scfx_rep& a,
			const scfx_params& params,
			bool& q_flag );

scfx_rep*
overflow_scfx_rep( const scfx_rep& a,
		    const scfx_params& params,
		    bool& o_flag );

} // namespace sc_dt


#endif

// Taf!
