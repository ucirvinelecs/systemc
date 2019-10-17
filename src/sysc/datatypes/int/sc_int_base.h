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

  sc_int_base.h -- A sc_int is a signed integer whose length is less than the
              machine's native integer length. We provide two implementations
              (i) sc_int with length between 1 - 64, and (ii) sc_int with
              length between 1 - 32. Implementation (i) is the default
              implementation, while implementation (ii) can be used only if
              the class library is compiled with -D_32BIT_. Unlike arbitrary
              precision, arithmetic and bitwise operations are performed
              using the native types (hence capped at 32/64 bits). The sc_int
              integer is useful when the user does not need arbitrary
              precision and the performance is superior to
              sc_bigint/sc_biguint.

  Original Author: Amit Rao, Synopsys, Inc.

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date: Ali Dasdan, Synopsys, Inc.
  Description of Modification: - Resolved ambiguity with sc_(un)signed.
                               - Merged the code for 64- and 32-bit versions
                                 via the constants in sc_nbdefs.h.
                               - Eliminated redundant file inclusions.

      Name, Affiliation, Date:
  Description of Modification:

 *****************************************************************************/

// $Log: sc_int_base.h,v $
// Revision 1.3  2011/08/24 22:05:45  acg
//  Torsten Maehne: initialization changes to remove warnings.
//
// Revision 1.2  2011/02/18 20:19:15  acg
//  Andy Goodrich: updating Copyright notice.
//
// Revision 1.1.1.1  2006/12/15 20:20:05  acg
// SystemC 2.3
//
// Revision 1.4  2006/05/08 17:50:01  acg
//   Andy Goodrich: Added David Long's declarations for friend operators,
//   functions, and methods, to keep the Microsoft compiler happy.
//
// Revision 1.3  2006/01/13 18:49:31  acg
// Added $Log command so that CVS check in comments are reproduced in the
// source.
//

#ifndef SC_INT_BASE_H
#define SC_INT_BASE_H

#include "sysc/kernel/sc_object.h"
#include "sysc/datatypes/misc/sc_value_base.h"
#include "sysc/datatypes/int/sc_int_ids.h"
#include "sysc/datatypes/int/sc_length_param.h"
#include "sysc/datatypes/int/sc_nbdefs.h"
#include "sysc/datatypes/int/sc_uint_base.h"
#include "sysc/utils/sc_iostream.h"
#include "sysc/utils/sc_temporary.h"


namespace sc_dt
{

class sc_concatref;

// classes defined in this module
class sc_int_bitref_r;
class sc_int_bitref;
class sc_int_subref_r;
class sc_int_subref;
class sc_int_base;
class sc_signed_subref_r;
class sc_unsigned_subref_r;

// forward class declarations
class sc_bv_base;
class sc_lv_base;
class sc_signed;
class sc_unsigned;
class sc_fxval;
class sc_fxval_fast;
class sc_fxnum;
class sc_fxnum_fast;


extern const uint_type mask_int[SC_INTWIDTH][SC_INTWIDTH];

// friend operator declarations
    // relational operators

    bool operator == ( const sc_int_base& a, const sc_int_base& b );

    bool operator != ( const sc_int_base& a, const sc_int_base& b );

    bool operator <  ( const sc_int_base& a, const sc_int_base& b );

    bool operator <= ( const sc_int_base& a, const sc_int_base& b );

    bool operator >  ( const sc_int_base& a, const sc_int_base& b );

    bool operator >= ( const sc_int_base& a, const sc_int_base& b );


// ----------------------------------------------------------------------------
//  CLASS : sc_int_bitref_r
//
//  Proxy class for sc_int bit selection (r-value only).
// ----------------------------------------------------------------------------

class sc_int_bitref_r : public sc_value_base
{
    friend class sc_int_base;

protected:

    // constructor

    sc_int_bitref_r();

    // initializer for sc_core::sc_vpool:

    void initialize( const sc_int_base* obj_p, int index_ );

public:

    // copy constructor

    sc_int_bitref_r( const sc_int_bitref_r& a );

    // destructor

    virtual ~sc_int_bitref_r();

    // capacity

    int length() const;

#ifdef SC_DT_DEPRECATED
    int bitwidth() const
	{ return length(); }
#endif

    // concatenation support

    virtual int concat_length( bool *xz_present_p ) const;
    virtual bool concat_get_ctrl( sc_digit* dst_p, int low_i ) const;
    virtual bool concat_get_data( sc_digit* dst_p, int low_i ) const;
    virtual uint64 concat_get_uint64() const;

    // implicit conversions

    operator uint64 () const;
    bool operator ! () const;
    bool operator ~ () const;


    // explicit conversions

    uint64 value() const;

    bool to_bool() const;


    // other methods

    void print( ::std::ostream& os = ::std::cout ) const;

protected:
    int          m_index;
    sc_int_base* m_obj_p;

private:

    // disabled
    sc_int_bitref_r& operator = ( const sc_int_bitref_r& );
};


inline
::std::ostream&
operator << ( ::std::ostream&, const sc_int_bitref_r& );


// ----------------------------------------------------------------------------
//  CLASS : sc_int_bitref
//
//  Proxy class for sc_int bit selection (r-value and l-value).
// ----------------------------------------------------------------------------

class sc_int_bitref
    : public sc_int_bitref_r
{
    friend class sc_int_base;
    friend class sc_core::sc_vpool<sc_int_bitref>;


    // constructor

    sc_int_bitref();


public:

    // copy constructor

    sc_int_bitref( const sc_int_bitref& a );

    // assignment operators

    sc_int_bitref& operator = ( const sc_int_bitref_r& b );
    sc_int_bitref& operator = ( const sc_int_bitref& b );
    sc_int_bitref& operator = ( bool b );

    sc_int_bitref& operator &= ( bool b );
    sc_int_bitref& operator |= ( bool b );
    sc_int_bitref& operator ^= ( bool b );

	// concatenation methods

    virtual void concat_set(int64 src, int low_i);
    virtual void concat_set(const sc_signed& src, int low_i);
    virtual void concat_set(const sc_unsigned& src, int low_i);
    virtual void concat_set(uint64 src, int low_i);


    // other methods

    void scan( ::std::istream& is = ::std::cin );

public:
    static sc_core::sc_vpool<sc_int_bitref> m_pool;

};



inline
::std::istream&
operator >> ( ::std::istream&, sc_int_bitref& );


// ----------------------------------------------------------------------------
//  CLASS : sc_int_subref_r
//
//  Proxy class for sc_int part selection (r-value only).
// ----------------------------------------------------------------------------

class sc_int_subref_r : public sc_value_base
{
    friend class sc_int_base;
    friend class sc_int_signal;
    friend class sc_int_subref;

protected:

    // constructor

    sc_int_subref_r();

    // initializer for sc_core::sc_vpool:

    void initialize( const sc_int_base* obj_p, int left_i, int right_i );


public:
    // copy constructor

    sc_int_subref_r( const sc_int_subref_r& a );

    // destructor

    virtual ~sc_int_subref_r();

    // capacity

    int length() const;

#ifdef SC_DT_DEPRECATED
    int bitwidth() const
	{ return length(); }
#endif

    // concatenation support

    virtual int concat_length(bool* xz_present_p) const;
    virtual bool concat_get_ctrl( sc_digit* dst_p, int low_i ) const;
    virtual bool concat_get_data( sc_digit* dst_p, int low_i ) const;
    virtual uint64 concat_get_uint64() const;

    // reduce methods

    bool and_reduce() const;

    bool nand_reduce() const;

    bool or_reduce() const;

    bool nor_reduce() const;

    bool xor_reduce() const;

    bool xnor_reduce() const;

    // implicit conversion to uint_type

    operator uint_type () const;

    // explicit conversions

    uint_type value() const;

    int           to_int() const;
    unsigned int  to_uint() const;
    long          to_long() const;
    unsigned long to_ulong() const;
    int64         to_int64() const;
    uint64        to_uint64() const;
    double        to_double() const;


    // explicit conversion to character string

    const std::string to_string( sc_numrep numrep = SC_DEC ) const;
    const std::string to_string( sc_numrep numrep, bool w_prefix ) const;


    // other methods

    void print( ::std::ostream& os = ::std::cout ) const;

protected:

    int          m_left;
    sc_int_base* m_obj_p;
    int          m_right;

private:
    const sc_int_subref_r& operator = ( const sc_int_subref_r& );
};



inline
::std::ostream&
operator << ( ::std::ostream&, const sc_int_subref_r& );


// ----------------------------------------------------------------------------
//  CLASS : sc_int_subref
//
//  Proxy class for sc_int part selection (r-value and l-value).
// ----------------------------------------------------------------------------

class sc_int_subref
    : public sc_int_subref_r
{
    friend class sc_int_base;
    friend class sc_core::sc_vpool<sc_int_subref>;


protected:

    // constructor
    sc_int_subref();

public:

    // copy constructor

    sc_int_subref( const sc_int_subref& a );

    // assignment operators

    sc_int_subref& operator = ( int_type v );
    sc_int_subref& operator = ( const sc_int_base& a );

    sc_int_subref& operator = ( const sc_int_subref_r& a );

    sc_int_subref& operator = ( const sc_int_subref& a );

    template< class T >
    sc_int_subref& operator = ( const sc_generic_base<T>& a )
        { return operator = ( a->to_int64() ); }

    sc_int_subref& operator = ( const char* a );

    sc_int_subref& operator = ( unsigned long a );

    sc_int_subref& operator = ( long a );

    sc_int_subref& operator = ( unsigned int a );

    sc_int_subref& operator = ( int a );

    sc_int_subref& operator = ( uint64 a );

    sc_int_subref& operator = ( double a );

    sc_int_subref& operator = ( const sc_signed& );
    sc_int_subref& operator = ( const sc_unsigned& );
    sc_int_subref& operator = ( const sc_bv_base& );
    sc_int_subref& operator = ( const sc_lv_base& );

	// concatenation methods

    virtual void concat_set(int64 src, int low_i);
    virtual void concat_set(const sc_signed& src, int low_i);
    virtual void concat_set(const sc_unsigned& src, int low_i);
    virtual void concat_set(uint64 src, int low_i);

    // other methods

    void scan( ::std::istream& is = ::std::cin );

public:
    static sc_core::sc_vpool<sc_int_subref> m_pool;

};



inline
::std::istream&
operator >> ( ::std::istream&, sc_int_subref& );


// ----------------------------------------------------------------------------
//  CLASS : sc_int_base
//
//  Base class for sc_int.
// ----------------------------------------------------------------------------

class sc_int_base : public sc_value_base
{
    friend class sc_int_bitref_r;
    friend class sc_int_bitref;
    friend class sc_int_subref_r;
    friend class sc_int_subref;


    // support methods

    void invalid_length() const;
    void invalid_index( int i ) const;
    void invalid_range( int l, int r ) const;

    void check_length() const;

    void check_index( int i ) const;

    void check_range( int l, int r ) const;

    void check_value() const;

    void extend_sign()
	{
#ifdef DEBUG_SYSTEMC
	    check_value();
#endif
	    m_val = ( m_val << m_ulen >> m_ulen );
	}

public:

    // constructors

    explicit sc_int_base( int w = sc_length_param().len() );

    sc_int_base( int_type v, int w );

    sc_int_base( const sc_int_base& a );

    explicit sc_int_base( const sc_int_subref_r& a );

    template< class T >
    explicit sc_int_base( const sc_generic_base<T>& a ) :
        m_val( a->to_int64() ), m_len( a->length() ),
	m_ulen( SC_INTWIDTH - m_len )
	{ check_length(); extend_sign(); }

    explicit sc_int_base( const sc_signed& a );
    explicit sc_int_base( const sc_unsigned& a );
    explicit sc_int_base( const sc_bv_base& v );
    explicit sc_int_base( const sc_lv_base& v );
    explicit sc_int_base( const sc_uint_subref_r& v );
    explicit sc_int_base( const sc_signed_subref_r& v );
    explicit sc_int_base( const sc_unsigned_subref_r& v );



    // destructor

    virtual ~sc_int_base();

    // assignment operators

    sc_int_base& operator = ( int_type v );

    sc_int_base& operator = ( const sc_int_base& a );

    sc_int_base& operator = ( const sc_int_subref_r& a );

    template<class T>
    sc_int_base& operator = ( const sc_generic_base<T>& a )
        { m_val = a->to_int64(); extend_sign(); return *this; }

    sc_int_base& operator = ( const sc_signed& a );
    sc_int_base& operator = ( const sc_unsigned& a );

#ifdef SC_INCLUDE_FX
    sc_int_base& operator = ( const sc_fxval& a );
    sc_int_base& operator = ( const sc_fxval_fast& a );
    sc_int_base& operator = ( const sc_fxnum& a );
    sc_int_base& operator = ( const sc_fxnum_fast& a );
#endif

    sc_int_base& operator = ( const sc_bv_base& a );
    sc_int_base& operator = ( const sc_lv_base& a );

    sc_int_base& operator = ( const char* a );

    sc_int_base& operator = ( unsigned long a );

    sc_int_base& operator = ( long a );

    sc_int_base& operator = ( unsigned int a );

    sc_int_base& operator = ( int a );

    sc_int_base& operator = ( uint64 a );

    sc_int_base& operator = ( double a );

    // arithmetic assignment operators

    sc_int_base& operator += ( int_type v );

    sc_int_base& operator -= ( int_type v );

    sc_int_base& operator *= ( int_type v );

    sc_int_base& operator /= ( int_type v );

    sc_int_base& operator %= ( int_type v );

    // bitwise assignment operators

    sc_int_base& operator &= ( int_type v );

    sc_int_base& operator |= ( int_type v );

    sc_int_base& operator ^= ( int_type v );

    sc_int_base& operator <<= ( int_type v );

    sc_int_base& operator >>= ( int_type v );


    // prefix and postfix increment and decrement operators

    sc_int_base& operator ++ (); // prefix

    const sc_int_base operator ++ ( int ); // postfix

    sc_int_base& operator -- (); // prefix

    const sc_int_base operator -- ( int );// postfix

    // relational operators

    friend bool operator == ( const sc_int_base& a, const sc_int_base& b );

    friend bool operator != ( const sc_int_base& a, const sc_int_base& b );

    friend bool operator <  ( const sc_int_base& a, const sc_int_base& b );

    friend bool operator <= ( const sc_int_base& a, const sc_int_base& b );

    friend bool operator >  ( const sc_int_base& a, const sc_int_base& b );

    friend bool operator >= ( const sc_int_base& a, const sc_int_base& b );


    // bit selection

    sc_int_bitref&         operator [] ( int i );
    const sc_int_bitref_r& operator [] ( int i ) const;

    sc_int_bitref&         bit( int i );
    const sc_int_bitref_r& bit( int i ) const;


    // part selection

    sc_int_subref&         operator () ( int left, int right );
    const sc_int_subref_r& operator () ( int left, int right ) const;

    sc_int_subref&         range( int left, int right );
    const sc_int_subref_r& range( int left, int right ) const;


    // bit access, without bounds checking or sign extension

    bool test( int i ) const;

    void set( int i );

    void set( int i, bool v );

    // capacity

    int length() const;

#ifdef SC_DT_DEPRECATED
    int bitwidth() const
	{ return length(); }
#endif

    // concatenation support

    virtual int concat_length(bool* xz_present_p) const;
    virtual bool concat_get_ctrl( sc_digit* dst_p, int low_i ) const;
    virtual bool concat_get_data( sc_digit* dst_p, int low_i ) const;
    virtual uint64 concat_get_uint64() const;
    virtual void concat_set(int64 src, int low_i);
    virtual void concat_set(const sc_signed& src, int low_i);
    virtual void concat_set(const sc_unsigned& src, int low_i);
    virtual void concat_set(uint64 src, int low_i);


    // reduce methods

    bool and_reduce() const;

    bool nand_reduce() const;

    bool or_reduce() const;

    bool nor_reduce() const;

    bool xor_reduce() const;

    bool xnor_reduce() const;

    // implicit conversion to int_type

    operator int_type() const;

    // explicit conversions

    int_type value() const;

    int to_int() const;

    unsigned int to_uint() const;

    long to_long() const;

    unsigned long to_ulong() const;

    int64 to_int64() const;

    uint64 to_uint64() const;

    double to_double() const;

#ifndef _32BIT_
    long long_low() const
	{ return (long) (m_val & UINT64_32ONES); }

    long long_high() const
	{ return (long) ((m_val >> 32) & UINT64_32ONES); }
#endif


    // explicit conversion to character string

    const std::string to_string( sc_numrep numrep = SC_DEC ) const;
    const std::string to_string( sc_numrep numrep, bool w_prefix ) const;


    // other methods

    void print( ::std::ostream& os = ::std::cout ) const;

    void scan( ::std::istream& is = ::std::cin );

protected:

    int_type m_val;   // value
    int      m_len;   // length
    int      m_ulen;  // unused length
};



inline
::std::ostream&
operator << ( ::std::ostream&, const sc_int_base& );

inline
::std::istream&
operator >> ( ::std::istream&, sc_int_base& );

// ----------------------------------------------------------------------------
//  CLASS : sc_int_bitref_r
//
//  Proxy class for sc_int bit selection (r-value only).
// ----------------------------------------------------------------------------
::std::ostream&
operator << ( ::std::ostream& os, const sc_int_bitref_r& a );
// ----------------------------------------------------------------------------
//  CLASS : sc_int_bitref
//
//  Proxy class for sc_int bit selection (r-value and l-value).
// ----------------------------------------------------------------------------
::std::istream&
operator >> ( ::std::istream& is, sc_int_bitref& a );
// ----------------------------------------------------------------------------
//  CLASS : sc_int_subref_r
//
//  Proxy class for sc_int part selection (r-value only).
// ----------------------------------------------------------------------------

// functional notation for the reduce methods

bool
and_reduce( const sc_int_subref_r& a );


bool
nand_reduce( const sc_int_subref_r& a );


bool
or_reduce( const sc_int_subref_r& a );


bool
nor_reduce( const sc_int_subref_r& a );


bool
xor_reduce( const sc_int_subref_r& a );


bool
xnor_reduce( const sc_int_subref_r& a );

::std::ostream&
operator << ( ::std::ostream& os, const sc_int_subref_r& a );

// ----------------------------------------------------------------------------
//  CLASS : sc_int_subref
//
//  Proxy class for sc_int part selection (r-value and l-value).
// ----------------------------------------------------------------------------

::std::istream&
operator >> ( ::std::istream& is, sc_int_subref& a );
// ----------------------------------------------------------------------------
//  CLASS : sc_int_base
//
//  Base class for sc_int.
// ----------------------------------------------------------------------------

// functional notation for the reduce methods

bool
and_reduce( const sc_int_base& a );

bool
nand_reduce( const sc_int_base& a );

bool
or_reduce( const sc_int_base& a );

bool
nor_reduce( const sc_int_base& a );

bool
xor_reduce( const sc_int_base& a );

bool
xnor_reduce( const sc_int_base& a );

inline
::std::ostream&
operator << ( ::std::ostream& os, const sc_int_base& a )
{
    a.print( os );
    return os;
}
::std::istream&
operator >> ( ::std::istream& is, sc_int_base& a );

} // namespace sc_dt


#endif

// Taf!
