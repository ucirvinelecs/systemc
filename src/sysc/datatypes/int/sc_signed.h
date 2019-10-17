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

  sc_signed.h -- Arbitrary precision signed arithmetic.

    This file includes the definitions of sc_signed_bitref,
    sc_signed_subref, and sc_signed classes. The first two classes are
    proxy classes to reference one bit and a range of bits of a
    sc_signed number, respectively.

    An sc_signed number has the sign-magnitude representation
    internally. However, its interface guarantees a 2's-complement
    representation. The sign-magnitude representation is chosen
    because of its efficiency: The sc_signed and sc_unsigned types are
    optimized for arithmetic rather than bitwise operations. For
    arithmetic operations, the sign-magnitude representation performs
    better.

    The implementations of sc_signed and sc_unsigned classes are
    almost identical: Most of the member and friend functions are
    defined in sc_nbcommon.cpp and sc_nbfriends.cpp so that they can
    be shared by both of these classes. These functions are chosed by
    defining a few macros before including them such as IF_SC_SIGNED
    and CLASS_TYPE. Our implementation choices are mostly dictated by
    performance considerations in that we tried to provide the most
    efficient sc_signed and sc_unsigned types without compromising
    their interface.

    For the behavior of operators, we have two semantics: the old and
    new. The most important difference between these two semantics is
    that the old semantics is closer to C/C++ semantics in that the
    result type of a binary operator on unsigned and signed arguments
    is unsigned; the new semantics, on the other hand, requires the
    result type be signed. The new semantics is required by the VSIA
    C/C++ data types standard. We have implemented the new semantics.

  Original Author: Ali Dasdan, Synopsys, Inc.

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date:
  Description of Modification:

 *****************************************************************************/

// $Log: sc_signed.h,v $
// Revision 1.3  2011/08/24 22:05:46  acg
//  Torsten Maehne: initialization changes to remove warnings.
//
// Revision 1.2  2011/02/18 20:19:15  acg
//  Andy Goodrich: updating Copyright notice.
//
// Revision 1.1.1.1  2006/12/15 20:20:05  acg
// SystemC 2.3
//
// Revision 1.5  2006/05/08 17:50:01  acg
//   Andy Goodrich: Added David Long's declarations for friend operators,
//   functions, and methods, to keep the Microsoft compiler happy.
//
// Revision 1.4  2006/03/13 20:25:27  acg
//  Andy Goodrich: Addition of function declarations, e.g., xor_signed_friend()
//  to keep gcc 4.x happy.
//
// Revision 1.3  2006/01/13 18:49:32  acg
// Added $Log command so that CVS check in comments are reproduced in the
// source.
//

#ifndef SC_SIGNED_H
#define SC_SIGNED_H


#include "sysc/kernel/sc_object.h"
#include "sysc/datatypes/misc/sc_value_base.h"
#include "sysc/utils/sc_iostream.h"
#include "sysc/utils/sc_temporary.h"
#include "sysc/datatypes/int/sc_length_param.h"
#include "sysc/datatypes/int/sc_nbdefs.h"
#include "sysc/datatypes/int/sc_nbutils.h"
#include "sysc/datatypes/int/sc_nbexterns.h"
#include "sysc/datatypes/int/sc_unsigned.h"


namespace sc_dt
{

// classes defined in this module
class sc_signed_bitref_r;
class sc_signed_bitref;
class sc_signed_subref_r;
class sc_signed_subref;
class sc_concatref;
class sc_signed;

// forward class declarations
class sc_bv_base;
class sc_lv_base;
class sc_int_base;
class sc_uint_base;
class sc_int_subref_r;
class sc_uint_subref_r;
class sc_signed;
class sc_unsigned;
class sc_unsigned_subref_r;
class sc_fxval;
class sc_fxval_fast;
class sc_fxnum;
class sc_fxnum_fast;


// Helper function declarations
sc_signed add_signed_friend(small_type us,
                                     int unb,
                                     int und,
                                     const sc_digit *ud,
                                     small_type vs,
                                     int vnb,
                                     int vnd,
                                     const sc_digit *vd);

sc_signed sub_signed_friend(small_type us,
                                     int unb,
                                     int und,
                                     const sc_digit *ud,
                                     small_type vs,
                                     int vnb,
                                     int vnd,
                                     const sc_digit *vd);

sc_signed mul_signed_friend(small_type s,
                                     int unb,
                                     int und,
                                     const sc_digit *ud,
                                     int vnb,
                                     int vnd,
                                     const sc_digit *vd);

sc_signed div_signed_friend(small_type s,
                                     int unb,
                                     int und,
                                     const sc_digit *ud,
                                     int vnb,
                                     int vnd,
                                     const sc_digit *vd);

sc_signed mod_signed_friend(small_type us,
                                     int unb,
                                     int und,
                                     const sc_digit *ud,
                                     int vnb,
                                     int vnd,
                                     const sc_digit *vd);

sc_signed and_signed_friend(small_type us,
                                     int unb,
                                     int und,
                                     const sc_digit *ud,
                                     small_type vs,
                                     int vnb,
                                     int vnd,
                                     const sc_digit *vd);

sc_signed or_signed_friend(small_type us,
                                    int unb,
                                    int und,
                                    const sc_digit *ud,
                                    small_type vs,
                                    int vnb,
                                    int vnd,
                                    const sc_digit *vd);

sc_signed xor_signed_friend(small_type us,
                                     int unb,
                                     int und,
                                     const sc_digit *ud,
                                     small_type vs,
                                     int vnb,
                                     int vnd,
                                     const sc_digit *vd);


// friend operator declarations
  // ARITHMETIC OPERATORS:

  // ADDition operators:

  sc_signed operator + (const sc_unsigned&  u, const sc_signed&    v);
  sc_signed operator + (const sc_signed&    u, const sc_unsigned&  v);

  sc_signed operator + (const sc_unsigned&  u, int64               v);
  sc_signed operator + (const sc_unsigned&  u, long                v);
  sc_signed operator + (const sc_unsigned&  u, int                 v);

  sc_signed operator + (int64               u, const sc_unsigned&  v);
  sc_signed operator + (long                u, const sc_unsigned&  v);
  sc_signed operator + (int                 u, const sc_unsigned&  v);

  sc_signed operator + (const sc_signed&    u, const sc_signed&    v);
  sc_signed operator + (const sc_signed&    u, int64               v);
  sc_signed operator + (const sc_signed&    u, uint64              v);
  sc_signed operator + (const sc_signed&    u, long                v);
  sc_signed operator + (const sc_signed&    u, unsigned long       v);
  sc_signed operator + (const sc_signed&    u, int                 v);
  sc_signed operator + (const sc_signed&    u, unsigned int        v);

  sc_signed operator + (int64               u, const sc_signed&    v);
  sc_signed operator + (uint64              u, const sc_signed&    v);
  sc_signed operator + (long                u, const sc_signed&    v);
  sc_signed operator + (unsigned long       u, const sc_signed&    v);
  sc_signed operator + (int                 u, const sc_signed&    v);
  sc_signed operator + (unsigned int        u, const sc_signed&    v);

  sc_signed operator + (const sc_unsigned&  u, const sc_int_base&  v);
  sc_signed operator + (const sc_int_base&  u, const sc_unsigned&  v);
  sc_signed operator + (const sc_signed&    u, const sc_int_base&  v);
  sc_signed operator + (const sc_signed&    u, const sc_uint_base& v);
  sc_signed operator + (const sc_int_base&  u, const sc_signed&    v);
  sc_signed operator + (const sc_uint_base& u, const sc_signed&    v);



  // SUBtraction operators:

  sc_signed operator - (const sc_unsigned&  u, const sc_signed&    v);
  sc_signed operator - (const sc_signed&    u, const sc_unsigned&  v);

  sc_signed operator - (const sc_unsigned&  u, const sc_unsigned&  v);
  sc_signed operator - (const sc_unsigned&  u, int64               v);
  sc_signed operator - (const sc_unsigned&  u, uint64              v);
  sc_signed operator - (const sc_unsigned&  u, long                v);
  sc_signed operator - (const sc_unsigned&  u, unsigned long       v);
  sc_signed operator - (const sc_unsigned&  u, int                v);
  sc_signed operator - (const sc_unsigned&  u, unsigned int       v);

  sc_signed operator - (int64               u, const sc_unsigned&  v);
  sc_signed operator - (uint64              u, const sc_unsigned&  v);
  sc_signed operator - (long                u, const sc_unsigned&  v);
  sc_signed operator - (unsigned long       u, const sc_unsigned&  v);
  sc_signed operator - (int                 u, const sc_unsigned&  v);
  sc_signed operator - (unsigned int        u, const sc_unsigned& v);

  sc_signed operator - (const sc_signed&    u, const sc_signed&    v);
  sc_signed operator - (const sc_signed&    u, int64               v);
  sc_signed operator - (const sc_signed&    u, uint64              v);
  sc_signed operator - (const sc_signed&    u, long                v);
  sc_signed operator - (const sc_signed&    u, unsigned long       v);
  sc_signed operator - (const sc_signed&    u, int                 v);
  sc_signed operator - (const sc_signed&    u, unsigned int        v);

  sc_signed operator - (int64               u, const sc_signed&    v);
  sc_signed operator - (uint64              u, const sc_signed&    v);
  sc_signed operator - (long                u, const sc_signed&    v);
  sc_signed operator - (unsigned long       u, const sc_signed&    v);
  sc_signed operator - (int                 u, const sc_signed&    v);
  sc_signed operator - (unsigned int        u, const sc_signed&    v);


  sc_signed operator - (const sc_unsigned&  u, const sc_int_base&  v);
  sc_signed operator - (const sc_unsigned&  u, const sc_uint_base& v);
  sc_signed operator - (const sc_int_base&  u, const sc_unsigned&  v);
  sc_signed operator - (const sc_uint_base& u, const sc_unsigned&  v);
  sc_signed operator - (const sc_signed&    u, const sc_int_base&  v);
  sc_signed operator - (const sc_signed&    u, const sc_uint_base& v);
  sc_signed operator - (const sc_int_base&  u, const sc_signed&    v);
  sc_signed operator - (const sc_uint_base& u, const sc_signed&    v);



  // MULtiplication operators:

  sc_signed operator * (const sc_unsigned&  u, const sc_signed&    v);
  sc_signed operator * (const sc_signed&    u, const sc_unsigned&  v);

  sc_signed operator * (const sc_unsigned&  u, int64               v);
  sc_signed operator * (const sc_unsigned&  u, long                v);
  sc_signed operator * (const sc_unsigned&  u, int                 v);

  sc_signed operator * (int64               u, const sc_unsigned&  v);
  sc_signed operator * (long                u, const sc_unsigned&  v);
  sc_signed operator * (int                 u, const sc_unsigned&  v);

  sc_signed operator * (const sc_signed&  u, const sc_signed&  v);
  sc_signed operator * (const sc_signed&  u, int64             v);
  sc_signed operator * (const sc_signed&  u, uint64            v);
  sc_signed operator * (const sc_signed&  u, long              v);
  sc_signed operator * (const sc_signed&  u, unsigned long     v);
  sc_signed operator * (const sc_signed&  u, int               v);
  sc_signed operator * (const sc_signed&  u, unsigned int      v);

  sc_signed operator * (int64             u, const sc_signed&  v);
  sc_signed operator * (uint64            u, const sc_signed&  v);
  sc_signed operator * (long              u, const sc_signed&  v);
  sc_signed operator * (unsigned long     u, const sc_signed&  v);
  sc_signed operator * (int               u, const sc_signed&  v);
  sc_signed operator * (unsigned int      u, const sc_signed&  v);

  sc_signed operator * (const sc_unsigned&  u, const sc_int_base&  v);
  sc_signed operator * (const sc_int_base&  u, const sc_unsigned&  v);
  sc_signed operator * (const sc_signed&    u, const sc_int_base&  v);
  sc_signed operator * (const sc_signed&    u, const sc_uint_base& v);
  sc_signed operator * (const sc_int_base&  u, const sc_signed&    v);
  sc_signed operator * (const sc_uint_base& u, const sc_signed&    v);



  // DIVision operators:

  sc_signed operator / (const sc_unsigned&  u, const sc_signed&    v);
  sc_signed operator / (const sc_signed&    u, const sc_unsigned&  v);

  sc_signed operator / (const sc_unsigned&  u, int64               v);
  sc_signed operator / (const sc_unsigned&  u, long                v);
  sc_signed operator / (const sc_unsigned&  u, int                 v);

  sc_signed operator / (int64               u, const sc_unsigned&  v);
  sc_signed operator / (long                u, const sc_unsigned&  v);
  sc_signed operator / (int                 u, const sc_unsigned&  v);

  sc_signed operator / (const sc_signed&    u, const sc_signed&    v);
  sc_signed operator / (const sc_signed&    u, int64               v);
  sc_signed operator / (const sc_signed&    u, uint64              v);
  sc_signed operator / (const sc_signed&    u, long                v);
  sc_signed operator / (const sc_signed&    u, unsigned long       v);
  sc_signed operator / (const sc_signed&    u, int                 v);
  sc_signed operator / (const sc_signed&    u, unsigned int        v);

  sc_signed operator / (int64               u, const sc_signed&    v);
  sc_signed operator / (uint64              u, const sc_signed&    v);
  sc_signed operator / (long                u, const sc_signed&    v);
  sc_signed operator / (unsigned long       u, const sc_signed&    v);
  sc_signed operator / (int                 u, const sc_signed&    v);
  sc_signed operator / (unsigned int        u, const sc_signed&    v);

  sc_signed operator / (const sc_unsigned&  u, const sc_int_base&  v);
  sc_signed operator / (const sc_int_base&  u, const sc_unsigned&  v);
  sc_signed operator / (const sc_signed&    u, const sc_int_base&  v);
  sc_signed operator / (const sc_signed&    u, const sc_uint_base& v);
  sc_signed operator / (const sc_int_base&  u, const sc_signed&    v);
  sc_signed operator / (const sc_uint_base& u, const sc_signed&    v);



  // MODulo operators:

  sc_signed operator % (const sc_unsigned&  u, const sc_signed&    v);
  sc_signed operator % (const sc_signed&    u, const sc_unsigned&  v);

  sc_signed operator % (const sc_unsigned&  u, int64               v);
  sc_signed operator % (const sc_unsigned&  u, long                v);
  sc_signed operator % (const sc_unsigned&  u, int                 v);

  sc_signed operator % (int64               u, const sc_unsigned&  v);
  sc_signed operator % (long                u, const sc_unsigned&  v);
  sc_signed operator % (int                 u, const sc_unsigned&  v);

  sc_signed operator % (const sc_signed&    u, const sc_signed&    v);
  sc_signed operator % (const sc_signed&    u, int64               v);
  sc_signed operator % (const sc_signed&    u, uint64              v);
  sc_signed operator % (const sc_signed&    u, long                v);
  sc_signed operator % (const sc_signed&    u, unsigned long       v);
  sc_signed operator % (const sc_signed&    u, int                 v);
  sc_signed operator % (const sc_signed&    u, unsigned int        v);

  sc_signed operator % (int64               u, const sc_signed&    v);
  sc_signed operator % (uint64              u, const sc_signed&    v);
  sc_signed operator % (long                u, const sc_signed&    v);
  sc_signed operator % (unsigned long       u, const sc_signed&    v);
  sc_signed operator % (int                 u, const sc_signed&    v);
  sc_signed operator % (unsigned int        u, const sc_signed&    v);

  sc_signed operator % (const sc_unsigned&  u, const sc_int_base&  v);
  sc_signed operator % (const sc_int_base&  u, const sc_unsigned&  v);
  sc_signed operator % (const sc_signed&    u, const sc_int_base&  v);
  sc_signed operator % (const sc_signed&    u, const sc_uint_base& v);
  sc_signed operator % (const sc_int_base&  u, const sc_signed&    v);
  sc_signed operator % (const sc_uint_base& u, const sc_signed&    v);



  // BITWISE OPERATORS:

  // Bitwise AND operators:

  sc_signed operator & (const sc_unsigned&  u, const sc_signed&    v);
  sc_signed operator & (const sc_signed&    u, const sc_unsigned&  v);

  sc_signed operator & (const sc_unsigned&  u, int64               v);
  sc_signed operator & (const sc_unsigned&  u, long                v);
  sc_signed operator & (const sc_unsigned&  u, int                 v);

  sc_signed operator & (int64               u, const sc_unsigned&  v);
  sc_signed operator & (long                u, const sc_unsigned&  v);
  sc_signed operator & (int                 u, const sc_unsigned&  v);

  sc_signed operator & (const sc_signed&    u, const sc_signed&    v);
  sc_signed operator & (const sc_signed&    u, int64               v);
  sc_signed operator & (const sc_signed&    u, uint64              v);
  sc_signed operator & (const sc_signed&    u, long                v);
  sc_signed operator & (const sc_signed&    u, unsigned long       v);
  sc_signed operator & (const sc_signed&    u, int                 v);
  sc_signed operator & (const sc_signed&    u, unsigned int        v);

  sc_signed operator & (int64             u, const sc_signed&  v);
  sc_signed operator & (uint64            u, const sc_signed&  v);
  sc_signed operator & (long              u, const sc_signed&  v);
  sc_signed operator & (unsigned long     u, const sc_signed&  v);
  sc_signed operator & (int               u, const sc_signed&  v);
  sc_signed operator & (unsigned int      u, const sc_signed&  v);

  sc_signed operator & (const sc_unsigned&  u, const sc_int_base&  v);
  sc_signed operator & (const sc_int_base&  u, const sc_unsigned&  v);
  sc_signed operator & (const sc_signed&    u, const sc_int_base&  v);
  sc_signed operator & (const sc_signed&    u, const sc_uint_base& v);
  sc_signed operator & (const sc_int_base&  u, const sc_signed&    v);
  sc_signed operator & (const sc_uint_base& u, const sc_signed&    v);



  // Bitwise OR operators:

  sc_signed operator | (const sc_unsigned&  u, const sc_signed&    v);
  sc_signed operator | (const sc_signed&    u, const sc_unsigned&  v);

  sc_signed operator | (const sc_unsigned&  u, int64               v);
  sc_signed operator | (const sc_unsigned&  u, long                v);
  sc_signed operator | (const sc_unsigned&  u, int                 v);

  sc_signed operator | (int64               u, const sc_unsigned&  v);
  sc_signed operator | (long                u, const sc_unsigned&  v);
  sc_signed operator | (int                 u, const sc_unsigned&  v);

  sc_signed operator | (const sc_signed&    u, const sc_signed&    v);
  sc_signed operator | (const sc_signed&    u, int64               v);
  sc_signed operator | (const sc_signed&    u, uint64              v);
  sc_signed operator | (const sc_signed&    u, long                v);
  sc_signed operator | (const sc_signed&    u, unsigned long       v);
  sc_signed operator | (const sc_signed&    u, int                 v);
  sc_signed operator | (const sc_signed&    u, unsigned int        v);

  sc_signed operator | (int64             u, const sc_signed&  v);
  sc_signed operator | (uint64            u, const sc_signed&  v);
  sc_signed operator | (long              u, const sc_signed&  v);
  sc_signed operator | (unsigned long     u, const sc_signed&  v);
  sc_signed operator | (int               u, const sc_signed&  v);
  sc_signed operator | (unsigned int      u, const sc_signed&  v);

  sc_signed operator | (const sc_unsigned&  u, const sc_int_base&  v);
  sc_signed operator | (const sc_int_base&  u, const sc_unsigned&  v);
  sc_signed operator | (const sc_signed&    u, const sc_int_base&  v);
  sc_signed operator | (const sc_signed&    u, const sc_uint_base& v);
  sc_signed operator | (const sc_int_base&  u, const sc_signed&    v);
  sc_signed operator | (const sc_uint_base& u, const sc_signed&    v);



  // Bitwise XOR operators:

  sc_signed operator ^ (const sc_unsigned&  u, const sc_signed&    v);
  sc_signed operator ^ (const sc_signed&    u, const sc_unsigned&  v);

  sc_signed operator ^ (const sc_unsigned&  u, int64               v);
  sc_signed operator ^ (const sc_unsigned&  u, long                v);
  sc_signed operator ^ (const sc_unsigned&  u, int                 v);

  sc_signed operator ^ (int64               u, const sc_unsigned&  v);
  sc_signed operator ^ (long                u, const sc_unsigned&  v);
  sc_signed operator ^ (int                 u, const sc_unsigned&  v);

  sc_signed operator ^ (const sc_signed&    u, const sc_signed&    v);
  sc_signed operator ^ (const sc_signed&    u, int64               v);
  sc_signed operator ^ (const sc_signed&    u, uint64              v);
  sc_signed operator ^ (const sc_signed&    u, long                v);
  sc_signed operator ^ (const sc_signed&    u, unsigned long       v);
  sc_signed operator ^ (const sc_signed&    u, int                 v);
  sc_signed operator ^ (const sc_signed&    u, unsigned int        v);

  sc_signed operator ^ (int64             u, const sc_signed&  v);
  sc_signed operator ^ (uint64            u, const sc_signed&  v);
  sc_signed operator ^ (long              u, const sc_signed&  v);
  sc_signed operator ^ (unsigned long     u, const sc_signed&  v);
  sc_signed operator ^ (int               u, const sc_signed&  v);
  sc_signed operator ^ (unsigned int      u, const sc_signed&  v);

  sc_signed operator ^ (const sc_unsigned&  u, const sc_int_base&  v);
  sc_signed operator ^ (const sc_int_base&  u, const sc_unsigned&  v);
  sc_signed operator ^ (const sc_signed&    u, const sc_int_base&  v);
  sc_signed operator ^ (const sc_signed&    u, const sc_uint_base& v);
  sc_signed operator ^ (const sc_int_base&  u, const sc_signed&    v);
  sc_signed operator ^ (const sc_uint_base& u, const sc_signed&    v);



  // SHIFT OPERATORS:

  // LEFT SHIFT operators:

  sc_unsigned operator << (const sc_unsigned&  u, const sc_signed&    v);
    sc_signed operator << (const sc_signed&    u, const sc_unsigned&  v);

    sc_signed operator << (const sc_signed&    u, const sc_signed&    v);
    sc_signed operator << (const sc_signed&    u, int64               v);
    sc_signed operator << (const sc_signed&    u, uint64              v);
    sc_signed operator << (const sc_signed&    u, long                v);
    sc_signed operator << (const sc_signed&    u, unsigned long       v);
  sc_signed operator << (const sc_signed&    u, int                 v);
  sc_signed operator << (const sc_signed&    u, unsigned int        v);

    sc_signed operator << (const sc_signed&    u, const sc_int_base&  v);
    sc_signed operator << (const sc_signed&    u, const sc_uint_base& v);



  // RIGHT SHIFT operators:

  sc_unsigned operator >> (const sc_unsigned&  u, const sc_signed&    v);
    sc_signed operator >> (const sc_signed&    u, const sc_unsigned&  v);

    sc_signed operator >> (const sc_signed&    u, const sc_signed&    v);
    sc_signed operator >> (const sc_signed&    u, int64               v);
    sc_signed operator >> (const sc_signed&    u, uint64              v);
    sc_signed operator >> (const sc_signed&    u, long                v);
    sc_signed operator >> (const sc_signed&    u, unsigned long       v);
  sc_signed operator >> (const sc_signed&    u, int                 v);
  sc_signed operator >> (const sc_signed&    u, unsigned int        v);

  sc_signed operator >> (const sc_signed&    u, const sc_int_base&  v);
  sc_signed operator >> (const sc_signed&    u, const sc_uint_base& v);



  // Unary arithmetic operators
  sc_signed operator + (const sc_signed&   u);
  sc_signed operator - (const sc_signed&   u);
  sc_signed operator - (const sc_unsigned& u);

  // LOGICAL OPERATORS:

  // Logical EQUAL operators:

  bool operator == (const sc_unsigned&  u, const sc_signed&    v);
  bool operator == (const sc_signed&    u, const sc_unsigned&  v);

  bool operator == (const sc_signed&    u, const sc_signed&    v);
  bool operator == (const sc_signed&    u, int64               v);
  bool operator == (const sc_signed&    u, uint64              v);
  bool operator == (const sc_signed&    u, long                v);
  bool operator == (const sc_signed&    u, unsigned long       v);
  bool operator == (const sc_signed&    u, int                 v);
  bool operator == (const sc_signed&    u, unsigned int        v);

  bool operator == (int64               u, const sc_signed&    v);
  bool operator == (uint64              u, const sc_signed&    v);
  bool operator == (long                u, const sc_signed&    v);
  bool operator == (unsigned long       u, const sc_signed&    v);
  bool operator == (int                 u, const sc_signed&    v);
  bool operator == (unsigned int        u, const sc_signed&    v);

  bool operator == (const sc_signed&    u, const sc_int_base&  v);
  bool operator == (const sc_signed&    u, const sc_uint_base& v);
  bool operator == (const sc_int_base&  u, const sc_signed&    v);
  bool operator == (const sc_uint_base& u, const sc_signed&    v);

  // Logical NOT_EQUAL operators:

  bool operator != (const sc_unsigned&  u, const sc_signed&    v);
  bool operator != (const sc_signed&    u, const sc_unsigned&  v);

  bool operator != (const sc_signed&    u, const sc_signed&    v);
  bool operator != (const sc_signed&    u, int64               v);
  bool operator != (const sc_signed&    u, uint64              v);
  bool operator != (const sc_signed&    u, long                v);
  bool operator != (const sc_signed&    u, unsigned long       v);
  bool operator != (const sc_signed&    u, int                 v);
  bool operator != (const sc_signed&    u, unsigned int        v);

  bool operator != (int64               u, const sc_signed&    v);
  bool operator != (uint64              u, const sc_signed&    v);
  bool operator != (long                u, const sc_signed&    v);
  bool operator != (unsigned long       u, const sc_signed&    v);
  bool operator != (int                 u, const sc_signed&    v);
  bool operator != (unsigned int        u, const sc_signed&    v);

  bool operator != (const sc_signed&    u, const sc_int_base&  v);
  bool operator != (const sc_signed&    u, const sc_uint_base& v);
  bool operator != (const sc_int_base&  u, const sc_signed&    v);
  bool operator != (const sc_uint_base& u, const sc_signed&    v);

  // Logical LESS_THAN operators:

  bool operator < (const sc_unsigned&  u, const sc_signed&    v);
  bool operator < (const sc_signed&    u, const sc_unsigned&  v);

  bool operator < (const sc_signed&    u, const sc_signed&    v);
  bool operator < (const sc_signed&    u, int64               v);
  bool operator < (const sc_signed&    u, uint64              v);
  bool operator < (const sc_signed&    u, long                v);
  bool operator < (const sc_signed&    u, unsigned long       v);
  bool operator < (const sc_signed&    u, int                 v);
  bool operator < (const sc_signed&    u, unsigned int        v);

  bool operator < (int64               u, const sc_signed&    v);
  bool operator < (uint64              u, const sc_signed&    v);
  bool operator < (long                u, const sc_signed&    v);
  bool operator < (unsigned long       u, const sc_signed&    v);
  bool operator < (int                 u, const sc_signed&    v);
  bool operator < (unsigned int        u, const sc_signed&    v);

  bool operator < (const sc_signed&    u, const sc_int_base&  v);
  bool operator < (const sc_signed&    u, const sc_uint_base& v);
  bool operator < (const sc_int_base&  u, const sc_signed&    v);
  bool operator < (const sc_uint_base& u, const sc_signed&    v);

  // Logical LESS_THAN_AND_EQUAL operators:

  bool operator <= (const sc_unsigned&  u, const sc_signed&    v);
  bool operator <= (const sc_signed&    u, const sc_unsigned&  v);

  bool operator <= (const sc_signed&    u, const sc_signed&    v);
  bool operator <= (const sc_signed&    u, int64               v);
  bool operator <= (const sc_signed&    u, uint64              v);
  bool operator <= (const sc_signed&    u, long                v);
  bool operator <= (const sc_signed&    u, unsigned long       v);
  bool operator <= (const sc_signed&    u, int                 v);
  bool operator <= (const sc_signed&    u, unsigned int        v);

  bool operator <= (int64               u, const sc_signed&    v);
  bool operator <= (uint64              u, const sc_signed&    v);
  bool operator <= (long                u, const sc_signed&    v);
  bool operator <= (unsigned long       u, const sc_signed&    v);
  bool operator <= (int                 u, const sc_signed&    v);
  bool operator <= (unsigned int        u, const sc_signed&    v);

  bool operator <= (const sc_signed&    u, const sc_int_base&  v);
  bool operator <= (const sc_signed&    u, const sc_uint_base& v);
  bool operator <= (const sc_int_base&  u, const sc_signed&    v);
  bool operator <= (const sc_uint_base& u, const sc_signed&    v);

  // Logical GREATER_THAN operators:

  bool operator > (const sc_unsigned&  u, const sc_signed&    v);
  bool operator > (const sc_signed&    u, const sc_unsigned&  v);

  bool operator > (const sc_signed&    u, const sc_signed&    v);
  bool operator > (const sc_signed&    u, int64               v);
  bool operator > (const sc_signed&    u, uint64              v);
  bool operator > (const sc_signed&    u, long                v);
  bool operator > (const sc_signed&    u, unsigned long       v);
  bool operator > (const sc_signed&    u, int                 v);
  bool operator > (const sc_signed&    u, unsigned int        v);

  bool operator > (int64               u, const sc_signed&    v);
  bool operator > (uint64              u, const sc_signed&    v);
  bool operator > (long                u, const sc_signed&    v);
  bool operator > (unsigned long       u, const sc_signed&    v);
  bool operator > (int                 u, const sc_signed&    v);
  bool operator > (unsigned int        u, const sc_signed&    v);

  bool operator > (const sc_signed&    u, const sc_int_base&  v);
  bool operator > (const sc_signed&    u, const sc_uint_base& v);
  bool operator > (const sc_int_base&  u, const sc_signed&    v);
  bool operator > (const sc_uint_base& u, const sc_signed&    v);

  // Logical GREATER_THAN_AND_EQUAL operators:

  bool operator >= (const sc_unsigned&  u, const sc_signed&    v);
  bool operator >= (const sc_signed&    u, const sc_unsigned&  v);

  bool operator >= (const sc_signed&    u, const sc_signed&    v);
  bool operator >= (const sc_signed&    u, int64               v);
  bool operator >= (const sc_signed&    u, uint64              v);
  bool operator >= (const sc_signed&    u, long                v);
  bool operator >= (const sc_signed&    u, unsigned long       v);
  bool operator >= (const sc_signed&    u, int                 v);
  bool operator >= (const sc_signed&    u, unsigned int        v);

  bool operator >= (int64               u, const sc_signed&    v);
  bool operator >= (uint64              u, const sc_signed&    v);
  bool operator >= (long                u, const sc_signed&    v);
  bool operator >= (unsigned long       u, const sc_signed&    v);
  bool operator >= (int                 u, const sc_signed&    v);
  bool operator >= (unsigned int        u, const sc_signed&    v);

  bool operator >= (const sc_signed&    u, const sc_int_base&  v);
  bool operator >= (const sc_signed&    u, const sc_uint_base& v);
  bool operator >= (const sc_int_base&  u, const sc_signed&    v);
  bool operator >= (const sc_uint_base& u, const sc_signed&    v);

  // Bitwise NOT operator (unary).
  sc_signed operator ~ (const sc_signed& u);

// ----------------------------------------------------------------------------
//  CLASS : sc_signed_bitref_r
//
//  Proxy class for sc_signed bit selection (r-value only).
// ----------------------------------------------------------------------------

class sc_signed_bitref_r : public sc_value_base
{
    friend class sc_signed;

protected:

    // constructor

    sc_signed_bitref_r();

    void initialize( const sc_signed* obj_p, int index_ );

public:

    // destructor

    virtual ~sc_signed_bitref_r();

    // copy constructor

    sc_signed_bitref_r( const sc_signed_bitref_r& a );

    // capacity

    int length() const;

    // implicit conversion to bool

    operator uint64 () const;
    bool operator ! () const;
    bool operator ~ () const;


    // explicit conversions

    bool value() const;

    bool to_bool() const;

    // concatenation support

    virtual int concat_length(bool* xz_present_p) const;
    virtual uint64 concat_get_uint64() const;
    virtual bool concat_get_ctrl( sc_digit* dst_p, int low_i ) const;
    virtual bool concat_get_data( sc_digit* dst_p, int low_i ) const;


    // other methods

    void print( ::std::ostream& os = ::std::cout ) const;

protected:

    int        m_index;  // Bit to be selected.
    sc_signed* m_obj_p;  // Target of this bit selection.

private:

    // disabled
    const sc_signed_bitref_r& operator = ( const sc_signed_bitref_r& );
};



inline
::std::ostream&
operator << ( ::std::ostream&, const sc_signed_bitref_r& );


// ----------------------------------------------------------------------------
//  CLASS : sc_signed_bitref
//
//  Proxy class for sc_signed bit selection (r-value and l-value).
// ----------------------------------------------------------------------------

class sc_signed_bitref
    : public sc_signed_bitref_r
{
    friend class sc_signed;
    friend class sc_core::sc_vpool<sc_signed_bitref>;


    // constructor

protected:

    sc_signed_bitref();

public:

    // copy constructor

    sc_signed_bitref( const sc_signed_bitref& a );

    // assignment operators

    const sc_signed_bitref& operator = ( const sc_signed_bitref_r& );
    const sc_signed_bitref& operator = ( const sc_signed_bitref& );
    const sc_signed_bitref& operator = ( bool );

    const sc_signed_bitref& operator &= ( bool );
    const sc_signed_bitref& operator |= ( bool );
    const sc_signed_bitref& operator ^= ( bool );

    // concatenation methods

    virtual void concat_set(int64 src, int low_i);
    virtual void concat_set(const sc_signed& src, int low_i);
    virtual void concat_set(const sc_unsigned& src, int low_i);
    virtual void concat_set(uint64 src, int low_i);


    // other methods

    void scan( ::std::istream& is = ::std::cin );

protected:
    static sc_core::sc_vpool<sc_signed_bitref> m_pool;
};



inline
::std::istream&
operator >> ( ::std::istream&, sc_signed_bitref& );


// ----------------------------------------------------------------------------
//  CLASS : sc_signed_subref_r
//
//  Proxy class for sc_signed part selection (r-value only).
// ----------------------------------------------------------------------------

class sc_signed_subref_r : public sc_value_base
{
    friend class sc_signed;
    friend class sc_signed_signal;
    friend class sc_unsigned;

protected:

    // constructor

    sc_signed_subref_r();

    void initialize( const sc_signed* obj_p, int left_, int right_ );


public:

    // destructor

    virtual ~sc_signed_subref_r();

    // copy constructor

    sc_signed_subref_r( const sc_signed_subref_r& a );

    // capacity

    int length() const;

    // implicit conversion to sc_unsigned

    operator sc_unsigned () const;


    // explicit conversions

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

    // concatenation support

    virtual int concat_length(bool* xz_present_p) const;
    virtual uint64 concat_get_uint64() const;
    virtual bool concat_get_ctrl( sc_digit* dst_p, int low_i ) const;
    virtual bool concat_get_data( sc_digit* dst_p, int low_i ) const;

    // reduce methods

    bool and_reduce() const;
    bool nand_reduce() const;
    bool or_reduce() const;
    bool nor_reduce() const;
    bool xor_reduce() const ;
    bool xnor_reduce() const;


    // other methods

    void print( ::std::ostream& os = ::std::cout ) const;

protected:

    int        m_left;   // Left-most bit in this part selection.
    sc_signed* m_obj_p;  // Target of this part selection.
    int        m_right;  // Right-most bit in this part selection.

private:
    const sc_signed_subref_r& operator = ( const sc_signed_subref_r& );

};



inline
::std::ostream&
operator << ( ::std::ostream&, const sc_signed_subref_r& );


// ----------------------------------------------------------------------------
//  CLASS : sc_signed_subref
//
//  Proxy class for sc_signed part selection (r-value and l-value).
// ----------------------------------------------------------------------------

class sc_signed_subref
    : public sc_signed_subref_r
{
    friend class sc_signed;
    friend class sc_core::sc_vpool<sc_signed_subref>;


    // constructor

    sc_signed_subref();

public:

    // copy constructor

    sc_signed_subref( const sc_signed_subref& a );

    // assignment operators

    const sc_signed_subref& operator = ( const sc_signed_subref_r& a );
    const sc_signed_subref& operator = ( const sc_signed_subref& a );
    const sc_signed_subref& operator = ( const sc_signed& a );

    const sc_signed_subref& operator = ( const sc_unsigned_subref_r& a );
    const sc_signed_subref& operator = ( const sc_unsigned& a );

    template< class T >
    const sc_signed_subref& operator = ( const sc_generic_base<T>& a )
    {
        sc_unsigned temp( length() );
	a->to_sc_unsigned(temp);
	return operator = (temp);
    }

    const sc_signed_subref& operator = ( const char* a );
    const sc_signed_subref& operator = ( unsigned long a );
    const sc_signed_subref& operator = ( long a );
    const sc_signed_subref& operator = ( unsigned int a );
    const sc_signed_subref& operator = ( int a );

    const sc_signed_subref& operator = ( uint64 a );
    const sc_signed_subref& operator = ( int64 a );
    const sc_signed_subref& operator = ( double a );
    const sc_signed_subref& operator = ( const sc_int_base& a );
    const sc_signed_subref& operator = ( const sc_uint_base& a );

    // concatenation methods

    virtual void concat_set(int64 src, int low_i);
    virtual void concat_set(const sc_signed& src, int low_i);
    virtual void concat_set(const sc_unsigned& src, int low_i);
    virtual void concat_set(uint64 src, int low_i);

    // other methods

    void scan( ::std::istream& is = ::std::cin );

protected:
    static sc_core::sc_vpool<sc_signed_subref> m_pool;
};



inline
::std::istream&
operator >> ( ::std::istream&, sc_signed_subref& );


// ----------------------------------------------------------------------------
//  CLASS : sc_signed
//
//  Arbitrary precision signed number.
// ----------------------------------------------------------------------------

class sc_signed : public sc_value_base
{
    friend class sc_concatref;
    friend class sc_signed_bitref_r;
    friend class sc_signed_bitref;
    friend class sc_signed_subref_r;
    friend class sc_signed_subref;
    friend class sc_unsigned;
    friend class sc_unsigned_subref;

  // Needed for types using sc_signed.
  typedef bool elemtype;

public:

    // constructors

    explicit sc_signed( int nb = sc_length_param().len() );
    sc_signed( const sc_signed&   v );
    sc_signed( const sc_unsigned& v );
    template<class T>
    explicit sc_signed( const sc_generic_base<T>& v );
    explicit sc_signed( const sc_bv_base& v );
    explicit sc_signed( const sc_lv_base& v );
    explicit sc_signed( const sc_int_subref_r& v );
    explicit sc_signed( const sc_uint_subref_r& v );
    explicit sc_signed( const sc_signed_subref_r& v );
    explicit sc_signed( const sc_unsigned_subref_r& v );

    // assignment operators

    const sc_signed& operator = (const sc_signed&          v);
    const sc_signed& operator = (const sc_signed_subref_r& a );

    template< class T >
    const sc_signed& operator = ( const sc_generic_base<T>& a )
        { a->to_sc_signed(*this); return *this; }

    const sc_signed& operator = (const sc_unsigned&        v);
    const sc_signed& operator = (const sc_unsigned_subref_r& a );

    const sc_signed& operator = (const char*               v);
    const sc_signed& operator = (int64                     v);
    const sc_signed& operator = (uint64                    v);
    const sc_signed& operator = (long                      v);
    const sc_signed& operator = (unsigned long             v);
    const sc_signed& operator = (int                       v);
    const sc_signed& operator = (unsigned int              v);

    const sc_signed& operator = (double                    v);
    const sc_signed& operator = (const sc_int_base&        v);
    const sc_signed& operator = (const sc_uint_base&       v);

    const sc_signed& operator = ( const sc_bv_base& );
    const sc_signed& operator = ( const sc_lv_base& );

#ifdef SC_INCLUDE_FX
    const sc_signed& operator = ( const sc_fxval& );
    const sc_signed& operator = ( const sc_fxval_fast& );
    const sc_signed& operator = ( const sc_fxnum& );
    const sc_signed& operator = ( const sc_fxnum_fast& );
#endif


    // destructor

    virtual ~sc_signed()
	{
#ifndef SC_MAX_NBITS
	    delete [] digit;
#endif
	}

    // Concatenation support:

    sc_digit* get_raw() const;
    virtual int concat_length(bool* xz_present_p) const;
    virtual bool concat_get_ctrl( sc_digit* dst_p, int low_i ) const;
    virtual bool concat_get_data( sc_digit* dst_p, int low_i ) const;
    virtual uint64 concat_get_uint64() const;
    virtual void concat_set(int64 src, int low_i);
    virtual void concat_set(const sc_signed& src, int low_i);
    virtual void concat_set(const sc_unsigned& src, int low_i);
    virtual void concat_set(uint64 src, int low_i);



    // Increment operators.
    sc_signed& operator ++ ();
    const sc_signed operator ++ (int);

    // Decrement operators.
    sc_signed& operator -- ();
    const sc_signed operator -- (int);


    // bit selection

    void check_index( int i ) const;

    void invalid_index( int i ) const;

    sc_signed_bitref& operator [] ( int i );

    const sc_signed_bitref_r& operator [] ( int i ) const;

    sc_signed_bitref& bit( int i );

    const sc_signed_bitref_r& bit( int i ) const;

    // part selection

    // Subref operators. Help access the range of bits from the ith to
    // jth. These indices have arbitrary precedence with respect to each
    // other, i.e., we can have i <= j or i > j. Note the equivalence
    // between range(i, j) and operator(i, j). Also note that
    // operator(i, i) returns a signed number that corresponds to the
    // bit operator[i], so these two forms are not the same.

    void check_range( int l, int r ) const;

    void invalid_range( int l, int r ) const;

    sc_signed_subref& range( int i, int j );

    const sc_signed_subref_r& range( int i, int j ) const;

    sc_signed_subref& operator () ( int i, int j );

    const sc_signed_subref_r& operator () ( int i, int j ) const;

    // explicit conversions

    int           to_int() const;
    unsigned int  to_uint() const;
    long          to_long() const;
    unsigned long to_ulong() const;
    int64         to_int64() const;
    uint64        to_uint64() const;
    double        to_double() const;

#ifdef SC_DT_DEPRECATED
    int to_signed() const
	{ return to_int(); }

    unsigned int to_unsigned() const
	{ return to_uint(); }
#endif

    // explicit conversion to character string

    const std::string to_string( sc_numrep numrep = SC_DEC ) const;
    const std::string to_string( sc_numrep numrep, bool w_prefix ) const;


    // Print functions. dump prints the internals of the class.

    void print( ::std::ostream& os = ::std::cout ) const;

    void scan( ::std::istream& is = ::std::cin );

    void dump( ::std::ostream& os = ::std::cout ) const;


  // Functions to find various properties.
  int  length() const { return nbits; }  // Bit width.
  bool iszero() const;                   // Is the number zero?
  bool sign() const;                     // Sign.

   // reduce methods

    bool and_reduce() const;

    bool nand_reduce() const;

    bool or_reduce() const;

    bool nor_reduce() const;

    bool xor_reduce() const;

    bool xnor_reduce() const;

  // Functions to access individual bits.
  bool test(int i) const;      // Is the ith bit 0 or 1?
  void set(int i);             // Set the ith bit to 1.
  void clear(int i);           // Set the ith bit to 0.
  void set(int i, bool v);      // Set the ith bit to v.
  void invert(int i);           // Negate the ith bit.

  // Make the number equal to its mirror image.
  void reverse();

  // Get/set a packed bit representation of the number.
  void get_packed_rep(sc_digit *buf) const;
  void set_packed_rep(sc_digit *buf);

  /*
    The comparison of the old and new semantics are as follows:

    Let s = sc_signed,
        u = sc_unsigned,
        un = { uint64, unsigned long, unsigned int },
        sn = { int64, long, int, char* }, and
        OP = { +, -, *, /, % }.

    Old semantics:                     New semantics:
      u OP u -> u                        u OP u -> u
      s OP u -> u                        s OP u -> s
      u OP s -> u                        u OP s -> s
      s OP s -> s                        s OP s -> s

      u OP un = un OP u -> u             u OP un = un OP u -> u
      u OP sn = sn OP u -> u             u OP sn = sn OP u -> s

      s OP un = un OP s -> s             s OP un = un OP s -> s
      s OP sn = sn OP s -> s             s OP sn = sn OP s -> s

    In the new semantics, the result is u if both operands are u; the
    result is s otherwise. The only exception is subtraction. The result
    of a subtraction is always s.

    The old semantics is like C/C++ semantics on integer types; the
    new semantics is due to the VSIA C/C++ data types standard.
   */

  // ARITHMETIC OPERATORS:

  // ADDition operators:

  friend sc_signed operator + (const sc_unsigned&  u, const sc_signed&    v);
  friend sc_signed operator + (const sc_signed&    u, const sc_unsigned&  v);

  friend sc_signed operator + (const sc_unsigned&  u, int64               v);
  friend sc_signed operator + (const sc_unsigned&  u, long                v);
  friend sc_signed operator + (const sc_unsigned&  u, int                 v);

  friend sc_signed operator + (int64               u, const sc_unsigned&  v);
  friend sc_signed operator + (long                u, const sc_unsigned&  v);
  friend sc_signed operator + (int                 u, const sc_unsigned&  v);

  friend sc_signed operator + (const sc_signed&    u, const sc_signed&    v);
  friend sc_signed operator + (const sc_signed&    u, int64               v);
  friend sc_signed operator + (const sc_signed&    u, uint64              v);
  friend sc_signed operator + (const sc_signed&    u, long                v);
  friend sc_signed operator + (const sc_signed&    u, unsigned long       v);
  friend sc_signed operator + (const sc_signed&    u, int                 v);
  friend sc_signed operator + (const sc_signed&    u, unsigned int        v);

  friend sc_signed operator + (int64               u, const sc_signed&    v);
  friend sc_signed operator + (uint64              u, const sc_signed&    v);
  friend sc_signed operator + (long                u, const sc_signed&    v);
  friend sc_signed operator + (unsigned long       u, const sc_signed&    v);
  friend sc_signed operator + (int                 u, const sc_signed&    v);
  friend sc_signed operator + (unsigned int        u, const sc_signed&    v);

  const sc_signed& operator += (const sc_signed&    v);
  const sc_signed& operator += (const sc_unsigned&  v);
  const sc_signed& operator += (int64               v);
  const sc_signed& operator += (uint64              v);
  const sc_signed& operator += (long                v);
  const sc_signed& operator += (unsigned long       v);
  const sc_signed& operator += (int                 v);
  const sc_signed& operator += (unsigned int        v);

  friend sc_signed operator + (const sc_unsigned&  u, const sc_int_base&  v);
  friend sc_signed operator + (const sc_int_base&  u, const sc_unsigned&  v);
  friend sc_signed operator + (const sc_signed&    u, const sc_int_base&  v);
  friend sc_signed operator + (const sc_signed&    u, const sc_uint_base& v);
  friend sc_signed operator + (const sc_int_base&  u, const sc_signed&    v);
  friend sc_signed operator + (const sc_uint_base& u, const sc_signed&    v);
  const sc_signed& operator += (const sc_int_base&  v);
  const sc_signed& operator += (const sc_uint_base& v);

  // SUBtraction operators:

  friend sc_signed operator - (const sc_unsigned&  u, const sc_signed&    v);
  friend sc_signed operator - (const sc_signed&    u, const sc_unsigned&  v);

  friend sc_signed operator - (const sc_unsigned&  u, const sc_unsigned&  v);
  friend sc_signed operator - (const sc_unsigned&  u, int64               v);
  friend sc_signed operator - (const sc_unsigned&  u, uint64              v);
  friend sc_signed operator - (const sc_unsigned&  u, long                v);
  friend sc_signed operator - (const sc_unsigned&  u, unsigned long       v);
  friend sc_signed operator - (const sc_unsigned&  u, int                v);
  friend sc_signed operator - (const sc_unsigned&  u, unsigned int       v);

  friend sc_signed operator - (int64               u, const sc_unsigned&  v);
  friend sc_signed operator - (uint64              u, const sc_unsigned&  v);
  friend sc_signed operator - (long                u, const sc_unsigned&  v);
  friend sc_signed operator - (unsigned long       u, const sc_unsigned&  v);
  friend sc_signed operator - (int                 u, const sc_unsigned&  v);
  friend sc_signed operator - (unsigned int        u, const sc_unsigned& v);

  friend sc_signed operator - (const sc_signed&    u, const sc_signed&    v);
  friend sc_signed operator - (const sc_signed&    u, int64               v);
  friend sc_signed operator - (const sc_signed&    u, uint64              v);
  friend sc_signed operator - (const sc_signed&    u, long                v);
  friend sc_signed operator - (const sc_signed&    u, unsigned long       v);
  friend sc_signed operator - (const sc_signed&    u, int                 v);
  friend sc_signed operator - (const sc_signed&    u, unsigned int        v);

  friend sc_signed operator - (int64               u, const sc_signed&    v);
  friend sc_signed operator - (uint64              u, const sc_signed&    v);
  friend sc_signed operator - (long                u, const sc_signed&    v);
  friend sc_signed operator - (unsigned long       u, const sc_signed&    v);
  friend sc_signed operator - (int                 u, const sc_signed&    v);
  friend sc_signed operator - (unsigned int        u, const sc_signed&    v);

  const sc_signed& operator -= (const sc_signed&    v);
  const sc_signed& operator -= (const sc_unsigned&  v);
  const sc_signed& operator -= (int64               v);
  const sc_signed& operator -= (uint64              v);
  const sc_signed& operator -= (long                v);
  const sc_signed& operator -= (unsigned long       v);
  const sc_signed& operator -= (int                 v);
  const sc_signed& operator -= (unsigned int        v);

  friend sc_signed operator - (const sc_unsigned&  u, const sc_int_base&  v);
  friend sc_signed operator - (const sc_unsigned&  u, const sc_uint_base& v);
  friend sc_signed operator - (const sc_int_base&  u, const sc_unsigned&  v);
  friend sc_signed operator - (const sc_uint_base& u, const sc_unsigned&  v);
  friend sc_signed operator - (const sc_signed&    u, const sc_int_base&  v);
  friend sc_signed operator - (const sc_signed&    u, const sc_uint_base& v);
  friend sc_signed operator - (const sc_int_base&  u, const sc_signed&    v);
  friend sc_signed operator - (const sc_uint_base& u, const sc_signed&    v);
  const sc_signed& operator -= (const sc_int_base&  v);
  const sc_signed& operator -= (const sc_uint_base& v);

  // MULtiplication operators:

  friend sc_signed operator * (const sc_unsigned&  u, const sc_signed&    v);
  friend sc_signed operator * (const sc_signed&    u, const sc_unsigned&  v);

  friend sc_signed operator * (const sc_unsigned&  u, int64               v);
  friend sc_signed operator * (const sc_unsigned&  u, long                v);
  friend sc_signed operator * (const sc_unsigned&  u, int                 v);

  friend sc_signed operator * (int64               u, const sc_unsigned&  v);
  friend sc_signed operator * (long                u, const sc_unsigned&  v);
  friend sc_signed operator * (int                 u, const sc_unsigned&  v);

  friend sc_signed operator * (const sc_signed&  u, const sc_signed&  v);
  friend sc_signed operator * (const sc_signed&  u, int64             v);
  friend sc_signed operator * (const sc_signed&  u, uint64            v);
  friend sc_signed operator * (const sc_signed&  u, long              v);
  friend sc_signed operator * (const sc_signed&  u, unsigned long     v);
  friend sc_signed operator * (const sc_signed&  u, int               v);
  friend sc_signed operator * (const sc_signed&  u, unsigned int      v);

  friend sc_signed operator * (int64             u, const sc_signed&  v);
  friend sc_signed operator * (uint64            u, const sc_signed&  v);
  friend sc_signed operator * (long              u, const sc_signed&  v);
  friend sc_signed operator * (unsigned long     u, const sc_signed&  v);
  friend sc_signed operator * (int               u, const sc_signed&  v);
  friend sc_signed operator * (unsigned int      u, const sc_signed&  v);

  const sc_signed& operator *= (const sc_signed&    v);
  const sc_signed& operator *= (const sc_unsigned&  v);
  const sc_signed& operator *= (int64               v);
  const sc_signed& operator *= (uint64              v);
  const sc_signed& operator *= (long                v);
  const sc_signed& operator *= (unsigned long       v);
  const sc_signed& operator *= (int                 v);
  const sc_signed& operator *= (unsigned int        v);

  friend sc_signed operator * (const sc_unsigned&  u, const sc_int_base&  v);
  friend sc_signed operator * (const sc_int_base&  u, const sc_unsigned&  v);
  friend sc_signed operator * (const sc_signed&    u, const sc_int_base&  v);
  friend sc_signed operator * (const sc_signed&    u, const sc_uint_base& v);
  friend sc_signed operator * (const sc_int_base&  u, const sc_signed&    v);
  friend sc_signed operator * (const sc_uint_base& u, const sc_signed&    v);
  const sc_signed& operator *= (const sc_int_base&  v);
  const sc_signed& operator *= (const sc_uint_base& v);

  // DIVision operators:

  friend sc_signed operator / (const sc_unsigned&  u, const sc_signed&    v);
  friend sc_signed operator / (const sc_signed&    u, const sc_unsigned&  v);

  friend sc_signed operator / (const sc_unsigned&  u, int64               v);
  friend sc_signed operator / (const sc_unsigned&  u, long                v);
  friend sc_signed operator / (const sc_unsigned&  u, int                 v);

  friend sc_signed operator / (int64               u, const sc_unsigned&  v);
  friend sc_signed operator / (long                u, const sc_unsigned&  v);
  friend sc_signed operator / (int                 u, const sc_unsigned&  v);

  friend sc_signed operator / (const sc_signed&    u, const sc_signed&    v);
  friend sc_signed operator / (const sc_signed&    u, int64               v);
  friend sc_signed operator / (const sc_signed&    u, uint64              v);
  friend sc_signed operator / (const sc_signed&    u, long                v);
  friend sc_signed operator / (const sc_signed&    u, unsigned long       v);
  friend sc_signed operator / (const sc_signed&    u, int                 v);
  friend sc_signed operator / (const sc_signed&    u, unsigned int        v);

  friend sc_signed operator / (int64               u, const sc_signed&    v);
  friend sc_signed operator / (uint64              u, const sc_signed&    v);
  friend sc_signed operator / (long                u, const sc_signed&    v);
  friend sc_signed operator / (unsigned long       u, const sc_signed&    v);
  friend sc_signed operator / (int                 u, const sc_signed&    v);
  friend sc_signed operator / (unsigned int        u, const sc_signed&    v);

  const sc_signed& operator /= (const sc_signed&    v);
  const sc_signed& operator /= (const sc_unsigned&  v);
  const sc_signed& operator /= (int64               v);
  const sc_signed& operator /= (uint64              v);
  const sc_signed& operator /= (long                v);
  const sc_signed& operator /= (unsigned long       v);
  const sc_signed& operator /= (int                 v);
  const sc_signed& operator /= (unsigned int        v);

  friend sc_signed operator / (const sc_unsigned&  u, const sc_int_base&  v);
  friend sc_signed operator / (const sc_int_base&  u, const sc_unsigned&  v);
  friend sc_signed operator / (const sc_signed&    u, const sc_int_base&  v);
  friend sc_signed operator / (const sc_signed&    u, const sc_uint_base& v);
  friend sc_signed operator / (const sc_int_base&  u, const sc_signed&    v);
  friend sc_signed operator / (const sc_uint_base& u, const sc_signed&    v);
  const sc_signed& operator /= (const sc_int_base&  v);
  const sc_signed& operator /= (const sc_uint_base& v);

  // MODulo operators:

  friend sc_signed operator % (const sc_unsigned&  u, const sc_signed&    v);
  friend sc_signed operator % (const sc_signed&    u, const sc_unsigned&  v);

  friend sc_signed operator % (const sc_unsigned&  u, int64               v);
  friend sc_signed operator % (const sc_unsigned&  u, long                v);
  friend sc_signed operator % (const sc_unsigned&  u, int                 v);

  friend sc_signed operator % (int64               u, const sc_unsigned&  v);
  friend sc_signed operator % (long                u, const sc_unsigned&  v);
  friend sc_signed operator % (int                 u, const sc_unsigned&  v);

  friend sc_signed operator % (const sc_signed&    u, const sc_signed&    v);
  friend sc_signed operator % (const sc_signed&    u, int64               v);
  friend sc_signed operator % (const sc_signed&    u, uint64              v);
  friend sc_signed operator % (const sc_signed&    u, long                v);
  friend sc_signed operator % (const sc_signed&    u, unsigned long       v);
  friend sc_signed operator % (const sc_signed&    u, int                 v);
  friend sc_signed operator % (const sc_signed&    u, unsigned int        v);

  friend sc_signed operator % (int64               u, const sc_signed&    v);
  friend sc_signed operator % (uint64              u, const sc_signed&    v);
  friend sc_signed operator % (long                u, const sc_signed&    v);
  friend sc_signed operator % (unsigned long       u, const sc_signed&    v);
  friend sc_signed operator % (int                 u, const sc_signed&    v);
  friend sc_signed operator % (unsigned int        u, const sc_signed&    v);

  const sc_signed& operator %= (const sc_signed&    v);
  const sc_signed& operator %= (const sc_unsigned&  v);
  const sc_signed& operator %= (int64               v);
  const sc_signed& operator %= (uint64              v);
  const sc_signed& operator %= (long                v);
  const sc_signed& operator %= (unsigned long       v);
  const sc_signed& operator %= (int                 v);
  const sc_signed& operator %= (unsigned int        v);

  friend sc_signed operator % (const sc_unsigned&  u, const sc_int_base&  v);
  friend sc_signed operator % (const sc_int_base&  u, const sc_unsigned&  v);
  friend sc_signed operator % (const sc_signed&    u, const sc_int_base&  v);
  friend sc_signed operator % (const sc_signed&    u, const sc_uint_base& v);
  friend sc_signed operator % (const sc_int_base&  u, const sc_signed&    v);
  friend sc_signed operator % (const sc_uint_base& u, const sc_signed&    v);
  const sc_signed& operator %= (const sc_int_base&  v);
  const sc_signed& operator %= (const sc_uint_base& v);

  // BITWISE OPERATORS:

  // Bitwise AND operators:

  friend sc_signed operator & (const sc_unsigned&  u, const sc_signed&    v);
  friend sc_signed operator & (const sc_signed&    u, const sc_unsigned&  v);

  friend sc_signed operator & (const sc_unsigned&  u, int64               v);
  friend sc_signed operator & (const sc_unsigned&  u, long                v);
  friend sc_signed operator & (const sc_unsigned&  u, int                 v);

  friend sc_signed operator & (int64               u, const sc_unsigned&  v);
  friend sc_signed operator & (long                u, const sc_unsigned&  v);
  friend sc_signed operator & (int                 u, const sc_unsigned&  v);

  friend sc_signed operator & (const sc_signed&    u, const sc_signed&    v);
  friend sc_signed operator & (const sc_signed&    u, int64               v);
  friend sc_signed operator & (const sc_signed&    u, uint64              v);
  friend sc_signed operator & (const sc_signed&    u, long                v);
  friend sc_signed operator & (const sc_signed&    u, unsigned long       v);
  friend sc_signed operator & (const sc_signed&    u, int                 v);
  friend sc_signed operator & (const sc_signed&    u, unsigned int        v);

  friend sc_signed operator & (int64             u, const sc_signed&  v);
  friend sc_signed operator & (uint64            u, const sc_signed&  v);
  friend sc_signed operator & (long              u, const sc_signed&  v);
  friend sc_signed operator & (unsigned long     u, const sc_signed&  v);
  friend sc_signed operator & (int               u, const sc_signed&  v);
  friend sc_signed operator & (unsigned int      u, const sc_signed&  v);

  const sc_signed& operator &= (const sc_signed&    v);
  const sc_signed& operator &= (const sc_unsigned&  v);
  const sc_signed& operator &= (int64               v);
  const sc_signed& operator &= (uint64              v);
  const sc_signed& operator &= (long                v);
  const sc_signed& operator &= (unsigned long       v);
  const sc_signed& operator &= (int                 v);
  const sc_signed& operator &= (unsigned int        v);

  friend sc_signed operator & (const sc_unsigned&  u, const sc_int_base&  v);
  friend sc_signed operator & (const sc_int_base&  u, const sc_unsigned&  v);
  friend sc_signed operator & (const sc_signed&    u, const sc_int_base&  v);
  friend sc_signed operator & (const sc_signed&    u, const sc_uint_base& v);
  friend sc_signed operator & (const sc_int_base&  u, const sc_signed&    v);
  friend sc_signed operator & (const sc_uint_base& u, const sc_signed&    v);
  const sc_signed& operator &= (const sc_int_base&  v);
  const sc_signed& operator &= (const sc_uint_base& v);

  // Bitwise OR operators:

  friend sc_signed operator | (const sc_unsigned&  u, const sc_signed&    v);
  friend sc_signed operator | (const sc_signed&    u, const sc_unsigned&  v);

  friend sc_signed operator | (const sc_unsigned&  u, int64               v);
  friend sc_signed operator | (const sc_unsigned&  u, long                v);
  friend sc_signed operator | (const sc_unsigned&  u, int                 v);

  friend sc_signed operator | (int64               u, const sc_unsigned&  v);
  friend sc_signed operator | (long                u, const sc_unsigned&  v);
  friend sc_signed operator | (int                 u, const sc_unsigned&  v);

  friend sc_signed operator | (const sc_signed&    u, const sc_signed&    v);
  friend sc_signed operator | (const sc_signed&    u, int64               v);
  friend sc_signed operator | (const sc_signed&    u, uint64              v);
  friend sc_signed operator | (const sc_signed&    u, long                v);
  friend sc_signed operator | (const sc_signed&    u, unsigned long       v);
  friend sc_signed operator | (const sc_signed&    u, int                 v);
  friend sc_signed operator | (const sc_signed&    u, unsigned int        v);

  friend sc_signed operator | (int64             u, const sc_signed&  v);
  friend sc_signed operator | (uint64            u, const sc_signed&  v);
  friend sc_signed operator | (long              u, const sc_signed&  v);
  friend sc_signed operator | (unsigned long     u, const sc_signed&  v);
  friend sc_signed operator | (int               u, const sc_signed&  v);
  friend sc_signed operator | (unsigned int      u, const sc_signed&  v);

  const sc_signed& operator |= (const sc_signed&    v);
  const sc_signed& operator |= (const sc_unsigned&  v);
  const sc_signed& operator |= (int64               v);
  const sc_signed& operator |= (uint64              v);
  const sc_signed& operator |= (long                v);
  const sc_signed& operator |= (unsigned long       v);
  const sc_signed& operator |= (int                 v);
  const sc_signed& operator |= (unsigned int        v);

  friend sc_signed operator | (const sc_unsigned&  u, const sc_int_base&  v);
  friend sc_signed operator | (const sc_int_base&  u, const sc_unsigned&  v);
  friend sc_signed operator | (const sc_signed&    u, const sc_int_base&  v);
  friend sc_signed operator | (const sc_signed&    u, const sc_uint_base& v);
  friend sc_signed operator | (const sc_int_base&  u, const sc_signed&    v);
  friend sc_signed operator | (const sc_uint_base& u, const sc_signed&    v);
  const sc_signed& operator |= (const sc_int_base&  v);
  const sc_signed& operator |= (const sc_uint_base& v);

  // Bitwise XOR operators:

  friend sc_signed operator ^ (const sc_unsigned&  u, const sc_signed&    v);
  friend sc_signed operator ^ (const sc_signed&    u, const sc_unsigned&  v);

  friend sc_signed operator ^ (const sc_unsigned&  u, int64               v);
  friend sc_signed operator ^ (const sc_unsigned&  u, long                v);
  friend sc_signed operator ^ (const sc_unsigned&  u, int                 v);

  friend sc_signed operator ^ (int64               u, const sc_unsigned&  v);
  friend sc_signed operator ^ (long                u, const sc_unsigned&  v);
  friend sc_signed operator ^ (int                 u, const sc_unsigned&  v);

  friend sc_signed operator ^ (const sc_signed&    u, const sc_signed&    v);
  friend sc_signed operator ^ (const sc_signed&    u, int64               v);
  friend sc_signed operator ^ (const sc_signed&    u, uint64              v);
  friend sc_signed operator ^ (const sc_signed&    u, long                v);
  friend sc_signed operator ^ (const sc_signed&    u, unsigned long       v);
  friend sc_signed operator ^ (const sc_signed&    u, int                 v);
  friend sc_signed operator ^ (const sc_signed&    u, unsigned int        v);

  friend sc_signed operator ^ (int64             u, const sc_signed&  v);
  friend sc_signed operator ^ (uint64            u, const sc_signed&  v);
  friend sc_signed operator ^ (long              u, const sc_signed&  v);
  friend sc_signed operator ^ (unsigned long     u, const sc_signed&  v);
  friend sc_signed operator ^ (int               u, const sc_signed&  v);
  friend sc_signed operator ^ (unsigned int      u, const sc_signed&  v);

  const sc_signed& operator ^= (const sc_signed&    v);
  const sc_signed& operator ^= (const sc_unsigned&  v);
  const sc_signed& operator ^= (int64               v);
  const sc_signed& operator ^= (uint64              v);
  const sc_signed& operator ^= (long                v);
  const sc_signed& operator ^= (unsigned long       v);
  const sc_signed& operator ^= (int                 v);
  const sc_signed& operator ^= (unsigned int        v);

  friend sc_signed operator ^ (const sc_unsigned&  u, const sc_int_base&  v);
  friend sc_signed operator ^ (const sc_int_base&  u, const sc_unsigned&  v);
  friend sc_signed operator ^ (const sc_signed&    u, const sc_int_base&  v);
  friend sc_signed operator ^ (const sc_signed&    u, const sc_uint_base& v);
  friend sc_signed operator ^ (const sc_int_base&  u, const sc_signed&    v);
  friend sc_signed operator ^ (const sc_uint_base& u, const sc_signed&    v);
  const sc_signed& operator ^= (const sc_int_base&  v);
  const sc_signed& operator ^= (const sc_uint_base& v);

  // SHIFT OPERATORS:

  // LEFT SHIFT operators:

  friend sc_unsigned operator << (const sc_unsigned&  u, const sc_signed&    v);
  friend   sc_signed operator << (const sc_signed&    u, const sc_unsigned&  v);

  friend   sc_signed operator << (const sc_signed&    u, const sc_signed&    v);
  friend   sc_signed operator << (const sc_signed&    u, int64               v);
  friend   sc_signed operator << (const sc_signed&    u, uint64              v);
  friend   sc_signed operator << (const sc_signed&    u, long                v);
  friend   sc_signed operator << (const sc_signed&    u, unsigned long       v);
  friend   sc_signed operator << (const sc_signed&    u, int                 v);
  friend   sc_signed operator << (const sc_signed&    u, unsigned int        v);

  const sc_signed& operator <<= (const sc_signed&    v);
  const sc_signed& operator <<= (const sc_unsigned&  v);
  const sc_signed& operator <<= (int64               v);
  const sc_signed& operator <<= (uint64              v);
  const sc_signed& operator <<= (long                v);
  const sc_signed& operator <<= (unsigned long       v);
  const sc_signed& operator <<= (int                 v);
  const sc_signed& operator <<= (unsigned int        v);

  friend   sc_signed operator << (const sc_signed&    u, const sc_int_base&  v);
  friend   sc_signed operator << (const sc_signed&    u, const sc_uint_base& v);
  const sc_signed& operator <<= (const sc_int_base&  v);
  const sc_signed& operator <<= (const sc_uint_base& v);

  // RIGHT SHIFT operators:

  friend sc_unsigned operator >> (const sc_unsigned&  u, const sc_signed&    v);
  friend   sc_signed operator >> (const sc_signed&    u, const sc_unsigned&  v);

  friend   sc_signed operator >> (const sc_signed&    u, const sc_signed&    v);
  friend   sc_signed operator >> (const sc_signed&    u, int64               v);
  friend   sc_signed operator >> (const sc_signed&    u, uint64              v);
  friend   sc_signed operator >> (const sc_signed&    u, long                v);
  friend   sc_signed operator >> (const sc_signed&    u, unsigned long       v);
  friend   sc_signed operator >> (const sc_signed&    u, int                 v);
  friend   sc_signed operator >> (const sc_signed&    u, unsigned int        v);

  const sc_signed& operator >>= (const sc_signed&    v);
  const sc_signed& operator >>= (const sc_unsigned&  v);
  const sc_signed& operator >>= (int64               v);
  const sc_signed& operator >>= (uint64              v);
  const sc_signed& operator >>= (long                v);
  const sc_signed& operator >>= (unsigned long       v);
  const sc_signed& operator >>= (int                 v);
  const sc_signed& operator >>= (unsigned int        v);

  friend sc_signed operator >> (const sc_signed&    u, const sc_int_base&  v);
  friend sc_signed operator >> (const sc_signed&    u, const sc_uint_base& v);
  const sc_signed& operator >>= (const sc_int_base&  v);
  const sc_signed& operator >>= (const sc_uint_base& v);

  // Unary arithmetic operators
  friend sc_signed operator + (const sc_signed&   u);
  friend sc_signed operator - (const sc_signed&   u);
  friend sc_signed operator - (const sc_unsigned& u);

  // LOGICAL OPERATORS:

  // Logical EQUAL operators:

  friend bool operator == (const sc_unsigned&  u, const sc_signed&    v);
  friend bool operator == (const sc_signed&    u, const sc_unsigned&  v);

  friend bool operator == (const sc_signed&    u, const sc_signed&    v);
  friend bool operator == (const sc_signed&    u, int64               v);
  friend bool operator == (const sc_signed&    u, uint64              v);
  friend bool operator == (const sc_signed&    u, long                v);
  friend bool operator == (const sc_signed&    u, unsigned long       v);
  friend bool operator == (const sc_signed&    u, int                 v);
  friend bool operator == (const sc_signed&    u, unsigned int        v);

  friend bool operator == (int64               u, const sc_signed&    v);
  friend bool operator == (uint64              u, const sc_signed&    v);
  friend bool operator == (long                u, const sc_signed&    v);
  friend bool operator == (unsigned long       u, const sc_signed&    v);
  friend bool operator == (int                 u, const sc_signed&    v);
  friend bool operator == (unsigned int        u, const sc_signed&    v);

  friend bool operator == (const sc_signed&    u, const sc_int_base&  v);
  friend bool operator == (const sc_signed&    u, const sc_uint_base& v);
  friend bool operator == (const sc_int_base&  u, const sc_signed&    v);
  friend bool operator == (const sc_uint_base& u, const sc_signed&    v);

  // Logical NOT_EQUAL operators:

  friend bool operator != (const sc_unsigned&  u, const sc_signed&    v);
  friend bool operator != (const sc_signed&    u, const sc_unsigned&  v);

  friend bool operator != (const sc_signed&    u, const sc_signed&    v);
  friend bool operator != (const sc_signed&    u, int64               v);
  friend bool operator != (const sc_signed&    u, uint64              v);
  friend bool operator != (const sc_signed&    u, long                v);
  friend bool operator != (const sc_signed&    u, unsigned long       v);
  friend bool operator != (const sc_signed&    u, int                 v);
  friend bool operator != (const sc_signed&    u, unsigned int        v);

  friend bool operator != (int64               u, const sc_signed&    v);
  friend bool operator != (uint64              u, const sc_signed&    v);
  friend bool operator != (long                u, const sc_signed&    v);
  friend bool operator != (unsigned long       u, const sc_signed&    v);
  friend bool operator != (int                 u, const sc_signed&    v);
  friend bool operator != (unsigned int        u, const sc_signed&    v);

  friend bool operator != (const sc_signed&    u, const sc_int_base&  v);
  friend bool operator != (const sc_signed&    u, const sc_uint_base& v);
  friend bool operator != (const sc_int_base&  u, const sc_signed&    v);
  friend bool operator != (const sc_uint_base& u, const sc_signed&    v);

  // Logical LESS_THAN operators:

  friend bool operator < (const sc_unsigned&  u, const sc_signed&    v);
  friend bool operator < (const sc_signed&    u, const sc_unsigned&  v);

  friend bool operator < (const sc_signed&    u, const sc_signed&    v);
  friend bool operator < (const sc_signed&    u, int64               v);
  friend bool operator < (const sc_signed&    u, uint64              v);
  friend bool operator < (const sc_signed&    u, long                v);
  friend bool operator < (const sc_signed&    u, unsigned long       v);
  friend bool operator < (const sc_signed&    u, int                 v);
  friend bool operator < (const sc_signed&    u, unsigned int        v);

  friend bool operator < (int64               u, const sc_signed&    v);
  friend bool operator < (uint64              u, const sc_signed&    v);
  friend bool operator < (long                u, const sc_signed&    v);
  friend bool operator < (unsigned long       u, const sc_signed&    v);
  friend bool operator < (int                 u, const sc_signed&    v);
  friend bool operator < (unsigned int        u, const sc_signed&    v);

  friend bool operator < (const sc_signed&    u, const sc_int_base&  v);
  friend bool operator < (const sc_signed&    u, const sc_uint_base& v);
  friend bool operator < (const sc_int_base&  u, const sc_signed&    v);
  friend bool operator < (const sc_uint_base& u, const sc_signed&    v);

  // Logical LESS_THAN_AND_EQUAL operators:

  friend bool operator <= (const sc_unsigned&  u, const sc_signed&    v);
  friend bool operator <= (const sc_signed&    u, const sc_unsigned&  v);

  friend bool operator <= (const sc_signed&    u, const sc_signed&    v);
  friend bool operator <= (const sc_signed&    u, int64               v);
  friend bool operator <= (const sc_signed&    u, uint64              v);
  friend bool operator <= (const sc_signed&    u, long                v);
  friend bool operator <= (const sc_signed&    u, unsigned long       v);
  friend bool operator <= (const sc_signed&    u, int                 v);
  friend bool operator <= (const sc_signed&    u, unsigned int        v);

  friend bool operator <= (int64               u, const sc_signed&    v);
  friend bool operator <= (uint64              u, const sc_signed&    v);
  friend bool operator <= (long                u, const sc_signed&    v);
  friend bool operator <= (unsigned long       u, const sc_signed&    v);
  friend bool operator <= (int                 u, const sc_signed&    v);
  friend bool operator <= (unsigned int        u, const sc_signed&    v);

  friend bool operator <= (const sc_signed&    u, const sc_int_base&  v);
  friend bool operator <= (const sc_signed&    u, const sc_uint_base& v);
  friend bool operator <= (const sc_int_base&  u, const sc_signed&    v);
  friend bool operator <= (const sc_uint_base& u, const sc_signed&    v);

  // Logical GREATER_THAN operators:

  friend bool operator > (const sc_unsigned&  u, const sc_signed&    v);
  friend bool operator > (const sc_signed&    u, const sc_unsigned&  v);

  friend bool operator > (const sc_signed&    u, const sc_signed&    v);
  friend bool operator > (const sc_signed&    u, int64               v);
  friend bool operator > (const sc_signed&    u, uint64              v);
  friend bool operator > (const sc_signed&    u, long                v);
  friend bool operator > (const sc_signed&    u, unsigned long       v);
  friend bool operator > (const sc_signed&    u, int                 v);
  friend bool operator > (const sc_signed&    u, unsigned int        v);

  friend bool operator > (int64               u, const sc_signed&    v);
  friend bool operator > (uint64              u, const sc_signed&    v);
  friend bool operator > (long                u, const sc_signed&    v);
  friend bool operator > (unsigned long       u, const sc_signed&    v);
  friend bool operator > (int                 u, const sc_signed&    v);
  friend bool operator > (unsigned int        u, const sc_signed&    v);

  friend bool operator > (const sc_signed&    u, const sc_int_base&  v);
  friend bool operator > (const sc_signed&    u, const sc_uint_base& v);
  friend bool operator > (const sc_int_base&  u, const sc_signed&    v);
  friend bool operator > (const sc_uint_base& u, const sc_signed&    v);

  // Logical GREATER_THAN_AND_EQUAL operators:

  friend bool operator >= (const sc_unsigned&  u, const sc_signed&    v);
  friend bool operator >= (const sc_signed&    u, const sc_unsigned&  v);

  friend bool operator >= (const sc_signed&    u, const sc_signed&    v);
  friend bool operator >= (const sc_signed&    u, int64               v);
  friend bool operator >= (const sc_signed&    u, uint64              v);
  friend bool operator >= (const sc_signed&    u, long                v);
  friend bool operator >= (const sc_signed&    u, unsigned long       v);
  friend bool operator >= (const sc_signed&    u, int                 v);
  friend bool operator >= (const sc_signed&    u, unsigned int        v);

  friend bool operator >= (int64               u, const sc_signed&    v);
  friend bool operator >= (uint64              u, const sc_signed&    v);
  friend bool operator >= (long                u, const sc_signed&    v);
  friend bool operator >= (unsigned long       u, const sc_signed&    v);
  friend bool operator >= (int                 u, const sc_signed&    v);
  friend bool operator >= (unsigned int        u, const sc_signed&    v);

  friend bool operator >= (const sc_signed&    u, const sc_int_base&  v);
  friend bool operator >= (const sc_signed&    u, const sc_uint_base& v);
  friend bool operator >= (const sc_int_base&  u, const sc_signed&    v);
  friend bool operator >= (const sc_uint_base& u, const sc_signed&    v);

  // Bitwise NOT operator (unary).
  friend sc_signed operator ~ (const sc_signed& u);

  // Helper functions.
  friend sc_signed add_signed_friend(small_type us,
                                     int unb,
                                     int und,
                                     const sc_digit *ud,
                                     small_type vs,
                                     int vnb,
                                     int vnd,
                                     const sc_digit *vd);

  friend sc_signed sub_signed_friend(small_type us,
                                     int unb,
                                     int und,
                                     const sc_digit *ud,
                                     small_type vs,
                                     int vnb,
                                     int vnd,
                                     const sc_digit *vd);

  friend sc_signed mul_signed_friend(small_type s,
                                     int unb,
                                     int und,
                                     const sc_digit *ud,
                                     int vnb,
                                     int vnd,
                                     const sc_digit *vd);

  friend sc_signed div_signed_friend(small_type s,
                                     int unb,
                                     int und,
                                     const sc_digit *ud,
                                     int vnb,
                                     int vnd,
                                     const sc_digit *vd);

  friend sc_signed mod_signed_friend(small_type us,
                                     int unb,
                                     int und,
                                     const sc_digit *ud,
                                     int vnb,
                                     int vnd,
                                     const sc_digit *vd);

  friend sc_signed and_signed_friend(small_type us,
                                     int unb,
                                     int und,
                                     const sc_digit *ud,
                                     small_type vs,
                                     int vnb,
                                     int vnd,
                                     const sc_digit *vd);

  friend sc_signed or_signed_friend(small_type us,
                                    int unb,
                                    int und,
                                    const sc_digit *ud,
                                    small_type vs,
                                    int vnb,
                                    int vnd,
                                    const sc_digit *vd);

  friend sc_signed xor_signed_friend(small_type us,
                                     int unb,
                                     int und,
                                     const sc_digit *ud,
                                     small_type vs,
                                     int vnb,
                                     int vnd,
                                     const sc_digit *vd);

private:

  small_type  sgn;         // Shortened as s.
  int nbits;       // Shortened as nb.
  int ndigits;     // Shortened as nd.

#ifdef SC_MAX_NBITS
  sc_digit digit[DIV_CEIL(SC_MAX_NBITS)];   // Shortened as d.
#else
  sc_digit *digit;                       // Shortened as d.
#endif

  // Private constructors:

  // Create a copy of v with sign s.
  sc_signed(const sc_signed&   v, small_type s);
  sc_signed(const sc_unsigned& v, small_type s);

  // Create a signed number with the given attributes.
  sc_signed(small_type s, int nb, int nd,
            sc_digit *d, bool alloc = true);

  // Create an unsigned number using the bits u[l..r].
  sc_signed(const sc_signed* u, int l, int r);
  sc_signed(const sc_unsigned* u, int l, int r);

  // Private member functions. The called functions are inline functions.

  small_type default_sign() const;

  int num_bits(int nb) const;

  bool check_if_outside(int bit_num) const;

  void copy_digits(int nb, int nd, const sc_digit *d);

  void makezero();

  // Conversion functions between 2's complement (2C) and
  // sign-magnitude (SM):
  void convert_2C_to_SM();

  void convert_SM_to_2C_to_SM();

  void convert_SM_to_2C();

};



inline
::std::ostream&
operator << ( ::std::ostream&, const sc_signed& );

inline
::std::istream&
operator >> ( ::std::istream&, sc_signed& );


::std::ostream&
operator << ( ::std::ostream& os, const sc_signed_bitref_r& a );


::std::istream&
operator >> ( ::std::istream& is, sc_signed_bitref& a );


// ----------------------------------------------------------------------------
//  CLASS : sc_signed_subref_r
//
//  Proxy class for sc_signed part selection (r-value only).
// ----------------------------------------------------------------------------

::std::ostream&
operator << ( ::std::ostream& os, const sc_signed_subref_r& a );

// ----------------------------------------------------------------------------
//  CLASS : sc_signed_subref
//
//  Proxy class for sc_signed part selection (r-value and l-value).
// ----------------------------------------------------------------------------

::std::istream&
operator >> ( ::std::istream& is, sc_signed_subref& a );

// ----------------------------------------------------------------------------
//  CLASS : sc_signed
//
//  Arbitrary precision signed number.
// ----------------------------------------------------------------------------

template<class T>
sc_signed::sc_signed( const sc_generic_base<T>& v )
{
    int nb = v->length();
    sgn = default_sign();
    if( nb > 0 ) {
        nbits = num_bits( nb );
    } else {
        char msg[BUFSIZ];
        std::sprintf( msg,
		    "sc_unsigned( sc_generic_base<T> ) : nb = %d is not valid", nb);
        SC_REPORT_ERROR( sc_core::SC_ID_INIT_FAILED_, msg );
    }
    ndigits = DIV_CEIL(nbits);
#   ifdef SC_MAX_NBITS
        test_bound(nb);
#    else
        digit = new sc_digit[ndigits];
#    endif
    makezero();
    v->to_sc_signed(*this);
}


::std::ostream&
operator << ( ::std::ostream& os, const sc_signed& a )
{
    a.print( os );
    return os;
}


::std::istream&
operator >> ( ::std::istream& is, sc_signed& a );


} // namespace sc_dt


#endif
