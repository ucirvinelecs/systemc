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

  sc_concatref.h -- Concatenation support.
 
  Original Author: Andy Goodrich, Forte Design, Inc.
  
 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date:
  Description of Modification:

  Andy Goodrich, Forte Design Systems, 17 Nov 2002
  Creation of sc_concatref class by merging the capabilities of 
  sc_int_concref, sc_int_concref, sc_uint_concref, sc_uint_concref, 
  and implementing the capabilities of sc_signed_concref, sc_signed_concref, 
  sc_unsigned_concref, and sc_unsigned_concref. The resultant class allows 
  mixed mode concatenations on the left and right sides of an assignment.

 *****************************************************************************/

// $Log: sc_concatref.h,v $
// Revision 1.6  2011/08/24 22:05:48  acg
//  Torsten Maehne: initialization changes to remove warnings.
//
// Revision 1.5  2009/11/17 19:58:15  acg
//  Andy Goodrich: fix of shift rhs possibilities to include "int".
//
// Revision 1.4  2009/02/28 00:26:29  acg
//  Andy Goodrich: bug fixes.
//
// Revision 1.3  2008/04/29 20:23:55  acg
//  Andy Goodrich: fixed the code that assigns the value of a string to
//  an sc_concatref instance.
//
// Revision 1.2  2008/02/14 20:57:26  acg
//  Andy Goodrich: added casts to ~0 instances to keep MSVC compiler happy.
//
// Revision 1.1.1.1  2006/12/15 20:20:05  acg
// SystemC 2.3
//
// Revision 1.4  2006/10/23 19:36:59  acg
//  Andy Goodrich: changed casts for operations on concatenation values to
//  mirror those of sc_unsigned. For instance, an sc_unsigned minus a value
//  returns an sc_signed result, whereas an sc_concatref minus a value was
//  returning an sc_unsigned result. Now both sc_unsigned and sc_concatref
//  minus a value return an sc_signed result.
//
// Revision 1.3  2006/01/13 18:54:01  acg
// Andy Goodrich: added $Log command so that CVS comments are reproduced in
// the source.
//

#ifndef SC_CONCATREF_H
#define SC_CONCATREF_H

#include "sysc/kernel/sc_object.h"
#include "sysc/datatypes/misc/sc_value_base.h"
#include "sysc/utils/sc_temporary.h"
#include "sysc/datatypes/bit/sc_bv.h"
#include "sysc/datatypes/bit/sc_lv.h"
#include "sysc/datatypes/int/sc_int_base.h"
#include "sysc/datatypes/int/sc_uint_base.h"
#include "sysc/datatypes/int/sc_signed.h"
#include "sysc/datatypes/int/sc_unsigned.h"

namespace sc_core {
    extern sc_byte_heap sc_temp_heap; // Temporary storage.
} // namespace sc_core

namespace sc_dt
{

// ----------------------------------------------------------------------------
//  CLASS TEMPLATE : sc_concatref
//
//  Proxy class for sized bit concatenation.
// ----------------------------------------------------------------------------

class sc_concatref : public sc_generic_base<sc_concatref>, public sc_value_base
{
public:
    friend class sc_core::sc_vpool<sc_concatref>;

    void initialize( 
        sc_value_base& left, sc_value_base& right );

    void initialize( 
        const sc_value_base& left, const sc_value_base& right );

    // destructor

    virtual ~sc_concatref();

    // capacity

    unsigned int length() const;

#ifdef SC_DT_DEPRECATED
    int bitwidth() const
        { return length(); }
#endif

    // concatenation

    virtual int concat_length( bool* xz_present_p ) const;

    virtual void concat_clear_data( bool to_ones );

    virtual bool concat_get_ctrl( sc_digit* dst_p, int low_i ) const;

    virtual bool concat_get_data( sc_digit* dst_p, int low_i ) const;

    virtual uint64 concat_get_uint64() const;

    virtual void concat_set( int64 src, int low_i );

    virtual void concat_set( const sc_signed& src, int low_i );

    virtual void concat_set( const sc_unsigned& src, int low_i );

    virtual void concat_set( uint64 src, int low_i );

    // explicit conversions

    uint64 to_uint64() const;

    const sc_unsigned& value() const
        {
            bool           left_non_zero;
            sc_unsigned*   result_p = sc_unsigned::m_pool.allocate();
            bool           right_non_zero;

            result_p->nbits = result_p->num_bits(m_len);
	    result_p->ndigits = DIV_CEIL(result_p->nbits);
            result_p->digit = (sc_digit*)sc_core::sc_temp_heap.allocate( 
                sizeof(sc_digit)*result_p->ndigits );
#if defined(_MSC_VER)
            // workaround spurious initialisation issue on MS Visual C++
            memset( result_p->digit, 0, sizeof(sc_digit)*result_p->ndigits );
#else
            result_p->digit[result_p->ndigits-1] = 0;
#endif
            right_non_zero = m_right_p->concat_get_data( result_p->digit, 0 );
            left_non_zero = m_left_p->concat_get_data(result_p->digit, m_len_r); 
            if ( left_non_zero || right_non_zero ) 
                result_p->sgn = SC_POS;
            else
                result_p->sgn = SC_ZERO;
            return *result_p;
        }

    int64 to_int64() const;

    int to_int() const;

    unsigned int  to_uint() const;

    long to_long() const;

    unsigned long to_ulong() const;

    double to_double() const;

    void to_sc_signed( sc_signed& target ) const;

    void to_sc_unsigned( sc_unsigned& target ) const;

    // implicit conversions:

    operator  uint64 () const ;

    operator const sc_unsigned& () const;

    // unary operators:

    sc_unsigned operator + () const;

    sc_signed operator - () const;

    sc_unsigned operator ~ () const;

    // explicit conversion to character string

    const std::string to_string( sc_numrep numrep = SC_DEC ) const;

    const std::string to_string( sc_numrep numrep, bool w_prefix ) const;

    // assignments

    const sc_concatref& operator = ( int v );

    const sc_concatref& operator = ( long v );

    const sc_concatref& operator = ( int64 v );

    const sc_concatref& operator = ( unsigned int v );

    const sc_concatref& operator = ( unsigned long v );

    const sc_concatref& operator = ( uint64 v );

    const sc_concatref& operator = ( const sc_concatref& v );

    const sc_concatref& operator = ( const sc_signed& v );

    const sc_concatref& operator = ( const sc_unsigned& v );

    const sc_concatref& operator = ( const char* v_p );

    const sc_concatref& operator = ( const sc_bv_base& v );

    const sc_concatref& operator = ( const sc_lv_base& v );

    // reduce methods

    bool and_reduce() const;

    bool nand_reduce() const;

    bool or_reduce() const;

    bool nor_reduce() const;

    bool xor_reduce() const;

    bool xnor_reduce() const;

    // other methods

    void print( ::std::ostream& os = ::std::cout ) const;

    void scan( ::std::istream& is ) ;

public:
    static sc_core::sc_vpool<sc_concatref> m_pool; // Pool of temporary objects.

public:
    enum concat_flags {
        cf_none = 0,        // Normal value. 
        cf_xz_present = 1   // X and/or Z values present.
    };

protected:
    sc_value_base*  m_left_p;    // Left hand operand of concatenation.
    sc_value_base*  m_right_p;   // Right hand operand of concatenation.
    int             m_len;       // Length of concatenation.
    int             m_len_r;     // Length of m_rightt_p.
    concat_flags    m_flags;     // Value is read only.

private:
    sc_concatref(const sc_concatref&);
    sc_concatref();
};


// functional notation for the reduce methods

bool
and_reduce( const sc_concatref& a );

bool
nand_reduce( const sc_concatref& a );

bool
or_reduce( const sc_concatref& a );

bool
nor_reduce( const sc_concatref& a );

bool
xor_reduce( const sc_concatref& a );

bool
xnor_reduce( const sc_concatref& a );


// SHIFT OPERATORS FOR sc_concatref OBJECT INSTANCES:
//
// Because sc_concatref has implicit casts to both uint64 and sc_unsigned
// it is necessary to disambiguate the use of the shift operators. We do
// this in favor of sc_unsigned so that precision is not lost. To get an
// integer-based result use a cast to uint64 before performing the shift.

const sc_unsigned operator << (const sc_concatref& target, uint64 shift);

const sc_unsigned operator << (const sc_concatref& target, int64 shift);

const sc_unsigned operator << ( 
    const sc_concatref& target, unsigned long shift );

const sc_unsigned operator << ( 
    const sc_concatref& target, int shift );

const sc_unsigned operator << ( 
    const sc_concatref& target, unsigned int shift );

const sc_unsigned operator << ( const sc_concatref& target, long shift );

const sc_unsigned operator >> (const sc_concatref& target, uint64 shift);

const sc_unsigned operator >> (const sc_concatref& target, int64 shift);

const sc_unsigned operator >> ( 
    const sc_concatref& target, unsigned long shift );

const sc_unsigned operator >> ( 
    const sc_concatref& target, int shift );

const sc_unsigned operator >> ( 
    const sc_concatref& target, unsigned int shift );

const sc_unsigned operator >> ( const sc_concatref& target, long shift );

// STREAM OPERATORS FOR sc_concatref OBJECT INSTANCES:

::std::ostream&
operator << ( ::std::ostream& os, const sc_concatref& v );

::std::istream&
operator >> ( ::std::istream& is, sc_concatref& a );


// ----------------------------------------------------------------------------
//  CLASS TEMPLATE : sc_concat_bool
//
//  Proxy class for read-only boolean values in concatenations.
// ----------------------------------------------------------------------------

class sc_concat_bool : public sc_value_base
{
  protected:
    static sc_core::sc_vpool<sc_concat_bool> m_pool;  // Temporaries pool.
    bool                                     m_value; // Value for this obj.

  public:

    // constructor:
    
    sc_concat_bool();

    // destructor:

    virtual ~sc_concat_bool();

    // allocation of temporary object:

    static sc_concat_bool* allocate( bool v );

    // concatenation:

    virtual int concat_length( bool* xz_present_p ) const;

    virtual bool concat_get_ctrl( sc_digit* dst_p, int low_i ) const;

    virtual bool concat_get_data( sc_digit* dst_p, int low_i ) const;

    virtual uint64 concat_get_uint64() const;
};


// ----------------------------------------------------------------------------
// ARITHMETIC AND LOGIC OPERATORS FOR sc_concatref
// ----------------------------------------------------------------------------

#define SC_CONCAT_OP_TYPE(RESULT,OP,OTHER_TYPE) \
    inline RESULT operator OP ( const sc_concatref& a, OTHER_TYPE b ) \
    { \
        return a.value() OP b; \
    } \
    inline RESULT operator OP ( OTHER_TYPE a, const sc_concatref& b ) \
    { \
        return a OP b.value(); \
    } 


#define SC_CONCAT_OP(RESULT,OP) \
    inline RESULT operator OP ( const sc_concatref& a, const sc_concatref& b ) \
    { \
        return a.value() OP b.value(); \
    }  \
    SC_CONCAT_OP_TYPE(const sc_signed,OP,int) \
    SC_CONCAT_OP_TYPE(const sc_signed,OP,long) \
    SC_CONCAT_OP_TYPE(const sc_signed,OP,int64) \
    SC_CONCAT_OP_TYPE(RESULT,OP,unsigned int) \
    SC_CONCAT_OP_TYPE(RESULT,OP,unsigned long) \
    SC_CONCAT_OP_TYPE(RESULT,OP,uint64) \
    SC_CONCAT_OP_TYPE(const sc_signed,OP,const sc_int_base&) \
    SC_CONCAT_OP_TYPE(RESULT,OP,const sc_uint_base&) \
    SC_CONCAT_OP_TYPE(const sc_signed,OP,const sc_signed&) \
    SC_CONCAT_OP_TYPE(RESULT,OP,const sc_unsigned&) \
    inline RESULT operator OP ( const sc_concatref& a, bool b ) \
    { \
        return a.value() OP (int)b; \
    } \
    inline RESULT operator OP ( bool a, const sc_concatref& b ) \
    { \
        return (int)a OP b.value(); \
    } 

#define SC_CONCAT_BOOL_OP(OP) \
    inline bool operator OP ( const sc_concatref& a, const sc_concatref& b ) \
    { \
        return a.value() OP b.value(); \
    }  \
    SC_CONCAT_OP_TYPE(bool,OP,int) \
    SC_CONCAT_OP_TYPE(bool,OP,long) \
    SC_CONCAT_OP_TYPE(bool,OP,int64) \
    SC_CONCAT_OP_TYPE(bool,OP,unsigned int) \
    SC_CONCAT_OP_TYPE(bool,OP,unsigned long) \
    SC_CONCAT_OP_TYPE(bool,OP,uint64) \
    SC_CONCAT_OP_TYPE(bool,OP,const sc_int_base&) \
    SC_CONCAT_OP_TYPE(bool,OP,const sc_uint_base&) \
    SC_CONCAT_OP_TYPE(bool,OP,const sc_signed&) \
    SC_CONCAT_OP_TYPE(bool,OP,const sc_unsigned&) \
    inline bool operator OP ( const sc_concatref& a, bool b ) \
    { \
        return a.value() OP (int)b; \
    } \
    inline bool operator OP ( bool a, const sc_concatref& b ) \
    { \
        return (int)a OP b.value(); \
    } 

SC_CONCAT_OP(const sc_unsigned,+)
SC_CONCAT_OP(const sc_signed,-)
SC_CONCAT_OP(const sc_unsigned,*)
SC_CONCAT_OP(const sc_unsigned,/)
SC_CONCAT_OP(const sc_unsigned,%)
SC_CONCAT_OP(const sc_unsigned,&)
SC_CONCAT_OP(const sc_unsigned,|)
SC_CONCAT_OP(const sc_unsigned,^)
SC_CONCAT_BOOL_OP(==)
SC_CONCAT_BOOL_OP(<=)
SC_CONCAT_BOOL_OP(>=)
SC_CONCAT_BOOL_OP(!=)
SC_CONCAT_BOOL_OP(>)
SC_CONCAT_BOOL_OP(<)

#undef SC_CONCAT_OP
#undef SC_CONCAT_OP_TYPE


// ----------------------------------------------------------------------------
// CONCATENATION FUNCTION AND OPERATOR FOR STANDARD SYSTEM C DATA TYPES:
// ----------------------------------------------------------------------------

sc_dt::sc_concatref& concat(
    sc_dt::sc_value_base& a, sc_dt::sc_value_base& b);

const
sc_dt::sc_concatref& concat(
    const sc_dt::sc_value_base& a, const sc_dt::sc_value_base& b);

const
sc_dt::sc_concatref& concat(const sc_dt::sc_value_base& a, bool b);

const
sc_dt::sc_concatref& concat(bool a, const sc_dt::sc_value_base& b);

sc_dt::sc_concatref& operator , (
    sc_dt::sc_value_base& a, sc_dt::sc_value_base& b);

const
sc_dt::sc_concatref& operator , (
    const sc_dt::sc_value_base& a, const sc_dt::sc_value_base& b);

const
sc_dt::sc_concatref& operator , (const sc_dt::sc_value_base& a, bool b);

const
sc_dt::sc_concatref& operator , (bool a, const sc_dt::sc_value_base& b);

} // namespace sc_dt

#endif //  SC_CONCATREF_H

