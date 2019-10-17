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

  sc_vector.cpp - A vector of named (SystemC) objects (modules, ports, channels)

  Original Author: Philipp A. Hartmann, OFFIS

  CHANGE LOG AT END OF FILE
 *****************************************************************************/


#include "sc_vector.h"

#include "sysc/utils/sc_hash.h"
#include "sysc/utils/sc_list.h"
#include "sysc/utils/sc_utils_ids.h"

#include "sysc/kernel/sc_simcontext.h"
#include "sysc/kernel/sc_object_manager.h"

#include <sstream>
//-------------------------------------------------------------Farah is working here 
const char * sc_core::sc_vector_base::kind() const { return "sc_vector"; }

  sc_core::sc_vector_base::size_type sc_core::sc_vector_base::size() const
    { return vec_.size(); }

  sc_core::sc_vector_base::sc_vector_base( const char* prefix )
    : sc_object( prefix )
    , vec_()
    , objs_vec_(0)
  {}

  sc_core::sc_vector_base::~sc_vector_base()
    { delete objs_vec_; }

  void * & sc_core::sc_vector_base::at( size_type i )
    { return vec_[i]; }

  void const * sc_core::sc_vector_base::at( size_type i ) const
    { return vec_[i]; }

  void sc_core::sc_vector_base::reserve( size_type n )
    { vec_.reserve(n); }

  void sc_core::sc_vector_base::clear()
    { vec_.clear(); }

  void sc_core::sc_vector_base::push_back( void* item )
    { vec_.push_back(item); }

  sc_core::sc_vector_base::iterator sc_core::sc_vector_base::begin() { return vec_.begin(); }
  sc_core::sc_vector_base::iterator sc_core::sc_vector_base::end()   { return vec_.end();   }

  sc_core::sc_vector_base::const_iterator sc_core::sc_vector_base::begin() const { return vec_.begin(); }
  sc_core::sc_vector_base::const_iterator sc_core::sc_vector_base::end()   const { return vec_.end();   }

  sc_core::sc_object* sc_core::sc_vector_base::implicit_cast( sc_object* p ) const { return p; }

  template< typename ElementType >
  sc_core::sc_direct_access<ElementType>::sc_direct_access(){}

  template< typename ElementType >
  sc_core::sc_direct_access<ElementType>::sc_direct_access( const non_const_policy& ) {}

  template< typename ElementType >
 typename sc_core::sc_direct_access<ElementType>::type* sc_core::sc_direct_access<ElementType>::get( type* this_ ) const
    { return this_; }

    template< typename ElementType , typename AccessType   >
    sc_core::sc_member_access< ElementType , AccessType   >::sc_member_access( member_type ptr )
      : ptr_(ptr) {}

    template< typename ElementType , typename AccessType   >
    sc_core::sc_member_access< ElementType , AccessType   >::sc_member_access( const non_const_policy& other )
      : ptr_(other.ptr_)
    {}

    template< typename ElementType , typename AccessType   >
    typename sc_core::sc_member_access<ElementType, AccessType>::access_type * sc_core::sc_member_access<ElementType , AccessType>::get( element_type* this_ ) const
      { return &(this_->*ptr_); }

//sc_vector functions

template< typename T >
 sc_core::sc_vector<T>::sc_vector(){}

template< typename T >
  sc_core::sc_vector<T>::sc_vector( const char* prefix )
    : base_type( prefix )
  {}

template< typename T >
  sc_core::sc_vector<T>::sc_vector( const char* prefix, size_type n )
    : base_type( prefix )
    { init(n); }

template< typename T >
  template< typename Creator >
  sc_core::sc_vector<T>::sc_vector( const char* prefix, size_type n, Creator creator )
    : base_type( prefix )
  {
    init( n, creator );
  }

template< typename T >
 typename  sc_core::sc_vector<T>::element_type& sc_core::sc_vector<T>::operator[]( size_type i )
    { return *static_cast<element_type*>( base_type::at(i) ); }

template< typename T >
 typename  sc_core::sc_vector<T>::element_type& sc_core::sc_vector<T>::at( size_type i )
    { check_index(i); return (*this)[i]; }

template< typename T >
  const typename sc_core::sc_vector<T>::element_type& sc_core::sc_vector<T>::operator[]( size_type i ) const
    { return *static_cast<element_type const *>( base_type::at(i) ); }

template< typename T >
 const  typename sc_core::sc_vector<T>::element_type& sc_core::sc_vector<T>::at( size_type i ) const
    { check_index(i); return (*this)[i]; }

template< typename T >
  void sc_core::sc_vector<T>::init( size_type n )
    { init( n, &this_type::create_element ); }

template< typename T >
 typename sc_core::sc_vector<T>::iterator sc_core::sc_vector<T>::begin() { return base_type::begin(); }

 template< typename T >
 typename sc_core::sc_vector<T>::iterator sc_core::sc_vector<T>::end()   { return base_type::end();   }

template< typename T >
 typename sc_core::sc_vector<T>::const_iterator sc_core::sc_vector<T>::begin()  const { return base_type::begin(); }

 template< typename T >
 typename sc_core::sc_vector<T>::const_iterator sc_core::sc_vector<T>::end()    const { return base_type::end(); }

template< typename T >
 typename sc_core::sc_vector<T>::const_iterator sc_core::sc_vector<T>::cbegin() const { return base_type::begin(); }

 template< typename T >
 typename sc_core::sc_vector<T>::const_iterator sc_core::sc_vector<T>::cend()   const { return base_type::end(); }

template< typename T >
 template< typename ContainerType, typename ArgumentType >
 typename sc_core::sc_vector<T>::iterator sc_core::sc_vector<T>::bind( sc_vector_assembly<ContainerType,ArgumentType> c )
    { return bind( c.begin(), c.end() ); }

template< typename T >
  template< typename BindableContainer >
 typename sc_core::sc_vector<T>::iterator sc_core::sc_vector<T>::bind( BindableContainer & c )
    { return bind( c.begin(), c.end() ); }

template< typename T >
  template< typename BindableIterator >
 typename sc_core::sc_vector<T>::iterator sc_core::sc_vector<T>::bind( BindableIterator first, BindableIterator last )
    { return bind( first, last, this->begin() ); }

template< typename T >
  template< typename BindableIterator >
 typename sc_core::sc_vector<T>::iterator sc_core::sc_vector<T>::bind( BindableIterator first, BindableIterator last
               , iterator from )
    { return sc_vector_do_bind( *this, first, last, from ); }

template< typename T >
  template< typename ContainerType, typename ArgumentType >
 typename sc_core::sc_vector<T>::iterator sc_core::sc_vector<T>::operator()( sc_vector_assembly<ContainerType,ArgumentType> c )
    { return operator()( c.begin(), c.end() ); }

template< typename T >
  template< typename ArgumentContainer >
 typename sc_core::sc_vector<T>::iterator sc_core::sc_vector<T>::operator()( ArgumentContainer & c )
    { return operator()( c.begin(), c.end() ); }

template< typename T >
  template< typename ArgumentIterator >
 typename sc_core::sc_vector<T>::iterator sc_core::sc_vector<T>::operator()( ArgumentIterator first, ArgumentIterator last )
    { return operator()( first, last, this->begin() ); }

template< typename T >
  template< typename ArgumentIterator >
 typename sc_core::sc_vector<T>::iterator sc_core::sc_vector<T>::operator()( ArgumentIterator first, ArgumentIterator last
                     , iterator from )
    { return sc_vector_do_operator_paren( *this, first, last, from ); }

  // member-wise access
/*
template< typename T >
  template< typename MT >
  sc_vector_assembly<T,MT> assemble( MT (T::*member_ptr) )
    { return sc_vector_assembly<T,MT>( *this, member_ptr ); }
i*/

template< typename T >
 typename  sc_core::sc_vector<T>::sc_object* sc_core::sc_vector<T>::object_cast( void* p ) const
    { return implicit_cast( static_cast<element_type*>(p) ); }





//sc_vector_assembly functions

template< typename T, typename MT >
  const char* sc_core::sc_vector_assembly<T, MT>::name() const { return vec_->name(); }

  template< typename T, typename MT >
  const char* sc_core::sc_vector_assembly<T, MT>::kind() const { return "sc_vector_assembly"; }

template< typename T, typename MT >
  typename sc_core::sc_vector_assembly<T, MT>::iterator sc_core::sc_vector_assembly<T, MT>::begin()
    { return iterator( (*vec_).begin().it_, ptr_ ); }

  template< typename T, typename MT >
  typename sc_core::sc_vector_assembly<T, MT>::iterator sc_core::sc_vector_assembly<T, MT>::end()
    { return iterator( (*vec_).end().it_, ptr_ ); }

template< typename T, typename MT >
  typename sc_core::sc_vector_assembly<T, MT>::const_iterator sc_core::sc_vector_assembly<T, MT>::cbegin() const
    { return const_iterator( (*vec_).cbegin().it_, ptr_ ); }

    template< typename T, typename MT >
  typename sc_core::sc_vector_assembly<T, MT>::const_iterator sc_core::sc_vector_assembly<T, MT>::cend() const
    { return const_iterator( (*vec_).cend().it_, ptr_ ); }

template< typename T, typename MT >
  typename sc_core::sc_vector_assembly<T, MT>::const_iterator sc_core::sc_vector_assembly<T, MT>::begin() const
    { return const_iterator( (*vec_).begin().it_, ptr_ ); }

    template< typename T, typename MT >
  typename sc_core::sc_vector_assembly<T, MT>::const_iterator sc_core::sc_vector_assembly<T, MT>::end()   const
    { return const_iterator( (*vec_).end().it_, ptr_ ); }

template< typename T, typename MT >
  typename sc_core::sc_vector_assembly<T, MT>::size_type sc_core::sc_vector_assembly<T, MT>::size() const { return vec_->size(); }

template< typename T, typename MT >
  typename sc_core::sc_vector_assembly<T, MT>::reference sc_core::sc_vector_assembly<T, MT>::operator[]( size_type idx )
    { return (*vec_)[idx].*ptr_; }

  template< typename T, typename MT >
  typename sc_core::sc_vector_assembly<T, MT>::reference sc_core::sc_vector_assembly<T, MT>::at( size_type idx )
    { return vec_->at(idx).*ptr_; }

    template< typename T, typename MT >
  typename sc_core::sc_vector_assembly<T, MT>::const_reference sc_core::sc_vector_assembly<T, MT>::operator[]( size_type idx ) const
    { return (*vec_)[idx].*ptr_; }

    template< typename T, typename MT >
 typename  sc_core::sc_vector_assembly<T, MT>::const_reference sc_core::sc_vector_assembly<T, MT>::at( size_type idx ) const
    { return vec_->at(idx).*ptr_; }

template< typename T, typename MT >
  template< typename ContainerType, typename ArgumentType >
 typename sc_core::sc_vector_assembly<T, MT>::iterator sc_core::sc_vector_assembly<T, MT>::bind( sc_vector_assembly<ContainerType,ArgumentType> c )
    { return bind( c.begin(), c.end() ); }

template< typename T, typename MT >
  template< typename BindableContainer >
 typename sc_core::sc_vector_assembly<T, MT>::iterator sc_core::sc_vector_assembly<T, MT>::bind( BindableContainer & c )
    { return bind( c.begin(), c.end() ); }

template< typename T, typename MT >
  template< typename BindableIterator >
 typename sc_core::sc_vector_assembly<T, MT>::iterator sc_core::sc_vector_assembly<T, MT>::bind( BindableIterator first, BindableIterator last )
    { return bind( first, last, this->begin() ); }

template< typename T, typename MT >
  template< typename BindableIterator >
 typename sc_core::sc_vector_assembly<T, MT>::iterator sc_core::sc_vector_assembly<T, MT>::bind( BindableIterator first, BindableIterator last
               , iterator from )
    { return sc_vector_do_bind( *this, first, last, from ); }

template< typename T, typename MT >
  template< typename BindableIterator >
 typename sc_core::sc_vector_assembly<T, MT>::iterator sc_core::sc_vector_assembly<T, MT>::bind( BindableIterator first, BindableIterator last
               , typename base_type::iterator from )
    { return bind( first, last, iterator(from.it_, ptr_) ); }

template< typename T, typename MT >
  template< typename ContainerType, typename ArgumentType >
 typename sc_core::sc_vector_assembly<T, MT>::iterator sc_core::sc_vector_assembly<T, MT>::operator()( sc_vector_assembly<ContainerType,ArgumentType> c )
    { return operator()( c.begin(), c.end() ); }

template< typename T, typename MT >
  template< typename ArgumentContainer >
 typename sc_core::sc_vector_assembly<T, MT>::iterator sc_core::sc_vector_assembly<T, MT>::operator()( ArgumentContainer & c )
    { return operator()( c.begin(), c.end() ); }

template< typename T, typename MT >
  template< typename ArgumentIterator >
 typename sc_core::sc_vector_assembly<T, MT>::iterator sc_core::sc_vector_assembly<T, MT>::operator()( ArgumentIterator first, ArgumentIterator last )
    { return operator()( first, last, this->begin() ); }

template< typename T, typename MT >
  template< typename ArgumentIterator >
 typename sc_core::sc_vector_assembly<T, MT>::iterator sc_core::sc_vector_assembly<T, MT>::operator()( ArgumentIterator first, ArgumentIterator last
                     , iterator from )
    { return sc_vector_do_operator_paren( *this, first, last, from ); }

template< typename T, typename MT >
  template< typename ArgumentIterator >
 typename sc_core::sc_vector_assembly<T, MT>::iterator sc_core::sc_vector_assembly<T, MT>::operator()( ArgumentIterator first, ArgumentIterator last
                     , typename base_type::iterator from )
    { return operator()( first, last, iterator(from.it_, ptr_) ); }

  template< typename T, typename MT >
  sc_core::sc_vector_assembly<T, MT>::sc_vector_assembly( const sc_vector_assembly & other )
    : vec_( other.vec_ )
    , ptr_( other.ptr_ )
    , child_vec_(0)
  {}

template< typename T, typename MT >
typename sc_core::sc_vector_assembly<T, MT>::sc_vector_assembly& sc_core::sc_vector_assembly<T, MT>::operator=( sc_vector_assembly other_copy )
  {
    swap( other_copy );
    return *this;
  }

template< typename T, typename MT >
  void sc_core::sc_vector_assembly<T, MT>::swap( sc_vector_assembly & that )
  {
    using std::swap;
    swap( vec_,       that.vec_ );
    swap( ptr_,       that.ptr_ );
    swap( child_vec_, that.child_vec_ );
  }

template< typename T, typename MT >
  void sc_core::sc_vector_assembly<T, MT>::report_empty_bind( const char* kind_, bool dst_empty_ ) const
    { vec_->report_empty_bind( kind_, dst_empty_ ); }

template< typename T, typename MT >
  sc_core::sc_vector_assembly<T, MT>::~sc_vector_assembly()
    { delete child_vec_; }


template< typename T, typename MT >
  sc_core::sc_vector_assembly<T, MT>::sc_vector_assembly( base_type & v, member_type ptr )
    : vec_(&v)
    , ptr_(ptr)
    , child_vec_(0)
  {}

template< typename T, typename MT >
typename sc_core:: sc_object* sc_core::sc_vector_assembly<T, MT>::object_cast( pointer p ) const
    { return vec_->implicit_cast( p ); }


//--------------------------------------------------------------Farah is done working here
namespace sc_core {

sc_vector_base::sc_vector_base()
  : sc_object( sc_gen_unique_name("vector") )
  , vec_()
  , objs_vec_()
{}

std::vector< sc_object* > const &
sc_vector_base::get_elements() const
{
  if( !objs_vec_ )
    objs_vec_ = new std::vector< sc_object* >;

  if( objs_vec_->size() || !size() )
    return *objs_vec_;

  objs_vec_->reserve( size() );
  for( const_iterator it=begin(); it != end(); ++it )
    if( sc_object* obj = object_cast(*it) )
      objs_vec_->push_back( obj );

  return *objs_vec_;
}

sc_object*
sc_vector_base::implicit_cast( ... ) const
{
  SC_REPORT_ERROR( SC_ID_VECTOR_NONOBJECT_ELEMENTS_, name() );
  return NULL;
}

void
sc_vector_base::check_index( size_type i ) const
{
  if( i>=size() )
  {
    std::stringstream str;
    str << name()
        << "[" << i << "] >= size() = " << size();
    SC_REPORT_ERROR( SC_ID_OUT_OF_BOUNDS_, str.str().c_str() );
  }
}

bool
sc_vector_base::check_init( size_type n ) const
{
  if ( !n )
    return false;

  if( size() ) // already filled
  {
    std::stringstream str;
    str << name()
        << ", size=" << size()
        << ", requested size=" << n;
    SC_REPORT_ERROR( SC_ID_VECTOR_INIT_CALLED_TWICE_
                   , str.str().c_str() );
    return false;
  }

  sc_simcontext* simc = simcontext();
  sc_assert( simc == sc_get_curr_simcontext() );

  sc_object* parent_p = simc->active_object();
  if( parent_p != get_parent_object() )
  {
    std::stringstream str;
    str << name() << ": expected "
        << ( get_parent_object()
              ? get_parent_object()->name() : "<top-level>" )
        << ", got "
        << ( parent_p ? parent_p->name() : "<top-level>" );

    SC_REPORT_ERROR( SC_ID_VECTOR_INIT_INVALID_CONTEXT_
                   , str.str().c_str() );
    return false;
  }

  return true;
}

void
sc_vector_base::report_empty_bind( const char* kind_, bool dst_empty_ ) const
{
  std::stringstream str;

  str << "target `" << name() << "' "
      << "(" << kind_ << ") ";

  if( !size() ) {
    str << "not initialised yet";
  } else if ( dst_empty_ ) {
    str << "empty range given";
  } else {
    str << "empty destination range given";
  }

  SC_REPORT_WARNING( SC_ID_VECTOR_BIND_EMPTY_, str.str().c_str() );
}

std::string
sc_vector_base::make_name( const char* prefix, size_type /* idx */ )
{
  // TODO: How to handle name clashes due to interleaved vector
  //       creation and init()?
  //       sc_vector< foo > v1, v2;
  //       v1.name() == "vector", v2.name() == "vector_0"
  //       v1.init( 1 ); -> v1[0].name() == "vector_0" -> clash
  return sc_gen_unique_name( prefix );
}

} // namespace sc_core

// $Log: sc_vector.cpp,v $
// Revision 1.6  2011/08/26 20:46:20  acg
//  Andy Goodrich: moved the modification log to the end of the file to
//  eliminate source line number skew when check-ins are done.
//
// Revision 1.5  2011/04/01 22:35:19  acg
//  Andy Goodrich: spelling fix.
//
// Revision 1.4  2011/03/28 13:03:09  acg
//  Andy Goodrich: Philipp's latest update.
//
// Revision 1.3  2011/03/23 16:16:28  acg
//  Philipp A. Hartmann: rebase implementation on void*
//      - supports virtual inheritance from sc_object again
//      - build up get_elements result on demand
//      - still requires element type to be derived from sc_object
//
// Revision 1.2  2011/02/14 17:54:25  acg
//  Andy Goodrich: Philipp's addition of early bind checks.
//
// Revision 1.1  2011/02/13 21:54:14  acg
//  Andy Goodrich: turn on embedding of cvs log records.

// Taf!
