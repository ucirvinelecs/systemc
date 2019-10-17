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

  sc_vector.h - A vector of named (SystemC) objects (modules, ports, channels)

  Original Author: Philipp A. Hartmann, OFFIS

  CHANGE LOG AT END OF FILE
 *****************************************************************************/

#ifndef SC_VECTOR_H_INCLUDED_
#define SC_VECTOR_H_INCLUDED_

#include <vector>
#include <iterator>
#include <string>
#include <algorithm> // std::swap

#include "sysc/kernel/sc_object.h"
#include "sysc/packages/boost/config.hpp"
#include "sysc/packages/boost/utility/enable_if.hpp"

//#define SC_VECTOR_HEADER_ONLY_

namespace sc_core {
namespace sc_meta {

  using ::sc_boost::enable_if;

  // simplistic version to avoid Boost et.al.
  template< typename T > struct remove_const          { typedef T type; };
  template< typename T > struct remove_const<const T> { typedef T type; };

  template< typename T, typename U >
  struct is_same      { SC_BOOST_STATIC_CONSTANT( bool, value = false ); };
  template< typename T >
  struct is_same<T,T> { SC_BOOST_STATIC_CONSTANT( bool, value = true );  };

  template< typename T >
  struct is_const           { SC_BOOST_STATIC_CONSTANT( bool, value = false ); };
  template< typename T >
  struct is_const< const T> { SC_BOOST_STATIC_CONSTANT( bool, value = true );  };

  template< typename CT, typename T >
  struct is_more_const {
    SC_BOOST_STATIC_CONSTANT( bool, value
       = ( is_same< typename remove_const<CT>::type
                 , typename remove_const<T>::type
                 >::value
          && ( is_const<CT>::value >= is_const<T>::value ) ) );
  };

  struct special_result {};
  template< typename T > struct remove_special_fptr {};
  template< typename T > 
  struct remove_special_fptr< special_result& (*)( T ) >
    { typedef T type; };

#define SC_RPTYPE_(Type)                                   \
  typename ::sc_core::sc_meta::remove_special_fptr         \
    < ::sc_core::sc_meta::special_result& (*) Type >::type

#define SC_ENABLE_IF_( Cond )                              \
  typename ::sc_core::sc_meta::enable_if                   \
    < SC_RPTYPE_(Cond) >::type * = NULL

} // namespace sc_meta

// forward declarations
template< typename T >              class sc_vector;
template< typename T, typename MT > class sc_vector_assembly;
template< typename T, typename MT > class sc_vector_iter;

// implementation-defined
template< typename Container, typename ArgumentIterator >
typename Container::iterator
sc_vector_do_bind( Container & cont
                 , ArgumentIterator  first
                 , ArgumentIterator  last
                 , typename Container::iterator from );

// implementation-defined
template< typename Container, typename ArgumentIterator >
typename Container::iterator
sc_vector_do_operator_paren( Container & cont
                           , ArgumentIterator  first
                           , ArgumentIterator  last
                           , typename Container::iterator from );

class sc_vector_base
  : public sc_object
{

  template<typename,typename> friend class sc_vector_assembly;
  template<typename,typename> friend class sc_vector_iter;

public:

  typedef std::vector< void* >          storage_type;
  typedef storage_type::size_type       size_type;
  typedef storage_type::difference_type difference_type;

  const char * kind() const;

  std::vector<sc_object*> const & get_elements() const;

  size_type size() const;

protected:

  // begin implementation defined

  typedef storage_type::iterator        iterator;
  typedef storage_type::const_iterator  const_iterator;

  sc_vector_base();

  sc_vector_base( const char* prefix );

  ~sc_vector_base();

  void * & at( size_type i );

  void const * at( size_type i ) const;

  void reserve( size_type n );

  void clear();

  void push_back( void* item );

  void check_index( size_type i ) const;
  bool check_init( size_type n )  const;

  static std::string make_name( const char* prefix, size_type index );

  iterator begin();
  iterator end();

  const_iterator begin() const;
  const_iterator end()   const;

  virtual sc_object* object_cast( void* ) const = 0;

  sc_object* implicit_cast( sc_object* p ) const;
  sc_object* implicit_cast( ... /* incompatible */ )  const;

public: 

  void report_empty_bind( const char* kind_, bool dst_range_ ) const;

private:
  storage_type vec_;
  mutable std::vector< sc_object* >* objs_vec_;

  // disabled
  sc_vector_base( const sc_vector_base& );
  sc_vector_base& operator=( const sc_vector_base& );

}; // sc_vector_base

// iterator access adapters
template< typename ElementType >
struct sc_direct_access
{
  typedef ElementType  element_type;
  typedef element_type type;
  typedef typename sc_meta::remove_const<type>::type plain_type;

  typedef sc_direct_access< type >             policy;
  typedef sc_direct_access< plain_type >       non_const_policy;
  typedef sc_direct_access< const plain_type > const_policy;

  sc_direct_access();
  sc_direct_access( const non_const_policy& );
  // convert from any policy to (const) direct policy
  template<typename U>
  sc_direct_access( const U&
    , SC_ENABLE_IF_((
        sc_meta::is_more_const<type,typename U::policy::element_type>
    )) );

  type* get( type* this_ ) const;
};

// iterator access adapters
template< typename ElementType
        , typename AccessType   >
class sc_member_access
{
  public:
    template< typename, typename > friend class sc_member_access;

    typedef ElementType element_type;
    typedef AccessType  access_type;
    typedef access_type (ElementType::*member_type);
    typedef access_type type;
    typedef typename sc_meta::remove_const<type>::type plain_type;
    typedef typename sc_meta::remove_const<ElementType>::type plain_elem_type;

    typedef sc_member_access< element_type, access_type > policy;
    typedef sc_member_access< plain_elem_type, plain_type >
      non_const_policy;
    typedef sc_member_access< const plain_elem_type, const plain_type >
      const_policy;

    sc_member_access( member_type ptr );

    sc_member_access( const non_const_policy& other );

    access_type * get( element_type* this_ ) const;

  private:
    member_type ptr_;
}; // sc_member_access


template< typename ElementType
        , typename AccessPolicy = sc_direct_access<ElementType> >
class sc_vector_iter
  : public std::iterator< std::random_access_iterator_tag
                        , typename AccessPolicy::type >
  , private AccessPolicy
{
  typedef ElementType  element_type;
  typedef typename AccessPolicy::policy            access_policy;
  typedef typename AccessPolicy::non_const_policy  non_const_policy;
  typedef typename AccessPolicy::const_policy      const_policy;
  typedef typename access_policy::type access_type;

  typedef typename sc_meta::remove_const<ElementType>::type plain_type;
  typedef const plain_type                                  const_plain_type;

  friend class sc_vector< plain_type >;
  template< typename, typename > friend class sc_vector_assembly;
  template< typename, typename > friend class sc_vector_iter;

  typedef std::iterator< std::random_access_iterator_tag, access_type > base_type;
  typedef sc_vector_iter               this_type;
  typedef sc_vector<plain_type>        vector_type;
  typedef sc_vector_base::storage_type storage_type;

  // select correct base-type iterator
  template< typename U > struct select_iter
    { typedef typename storage_type::iterator       type; };
  template< typename U > struct select_iter< const U >
    { typedef typename storage_type::const_iterator type; };

  typedef typename select_iter<ElementType>::type          raw_iterator;
  typedef sc_vector_iter< const_plain_type, const_policy > const_iterator;

  // underlying vector iterator

  raw_iterator it_;

  sc_vector_iter( raw_iterator it, access_policy acc = access_policy() )
    : access_policy(acc), it_(it) {}

  access_policy const & get_policy() const { return *this; }

public:
  // interface for Random Access Iterator category,
  // see ISO/IEC 14882:2003(E), 24.1 [lib.iterator.requirements]

  typedef typename base_type::difference_type difference_type;
  typedef typename base_type::reference       reference;
  typedef typename base_type::pointer         pointer;

  sc_vector_iter() : access_policy(), it_() {}

  // iterator conversions to more const, and/or direct iterators
  template< typename OtherElement, typename OtherPolicy >
  sc_vector_iter( const sc_vector_iter<OtherElement, OtherPolicy>& it
      , SC_ENABLE_IF_((
          sc_meta::is_more_const< element_type
                                , typename OtherPolicy::element_type >
        ))
      )
    : access_policy( it.get_policy() ), it_( it.it_ )
  {}

  // step
  this_type& operator++(){ ++it_; return *this; }
  this_type& operator--(){ --it_; return *this; }
  this_type  operator++(int){ this_type old(*this); ++it_; return old; }
  this_type  operator--(int){ this_type old(*this); --it_; return old; }

  // advance
  this_type  operator+( difference_type n ) const
    { return this_type( it_ + n, get_policy()); }
  this_type  operator-( difference_type n ) const
    { return this_type( it_ - n, get_policy()); }

  this_type& operator+=( difference_type n ) { it_+=n; return *this; }
  this_type& operator-=( difference_type n ) { it_-=n; return *this; }

  // relations
  bool operator== ( const this_type& that ) const { return it_ == that.it_; }
  bool operator!= ( const this_type& that ) const { return it_ != that.it_; }
  bool operator<= ( const this_type& that ) const { return it_ <= that.it_; }
  bool operator>= ( const this_type& that ) const { return it_ >= that.it_; }
  bool operator<  ( const this_type& that ) const { return it_ < that.it_; }
  bool operator>  ( const this_type& that ) const { return it_ > that.it_; }

  // dereference
  reference operator*() const
    { return *access_policy::get( static_cast<element_type*>(*it_) ); }
  pointer   operator->() const
    { return access_policy::get( static_cast<element_type*>(*it_) ); }
  reference operator[]( difference_type n ) const
    { return *access_policy::get( static_cast<element_type*>(it_[n]) ); }

  // distance
  difference_type operator-( this_type const& that ) const
    { return it_-that.it_; }

}; // sc_vector_iter

template< typename T >
class sc_vector
  : public sc_vector_base
{
  typedef sc_vector_base base_type;
  typedef sc_vector<T>   this_type;

public:
  typedef T element_type;
  typedef sc_vector_iter< element_type >       iterator;
  typedef sc_vector_iter< const element_type > const_iterator;

  sc_vector();

  explicit sc_vector( const char* prefix );

  explicit sc_vector( const char* prefix, size_type n );

  template< typename Creator >
  sc_vector( const char* prefix, size_type n, Creator creator );

  virtual ~sc_vector();

  element_type& operator[]( size_type i );

  element_type& at( size_type i );

  const element_type& operator[]( size_type i ) const;

  const element_type& at( size_type i ) const;

  void init( size_type n );

  template< typename Creator >
  void init( size_type n, Creator c );

  static element_type * create_element( const char* prefix, size_type index );

  iterator begin();
  iterator end();

  const_iterator begin()  const;
  const_iterator end()    const;

  const_iterator cbegin() const;
  const_iterator cend()   const;

  template< typename ContainerType, typename ArgumentType >
  iterator bind( sc_vector_assembly<ContainerType,ArgumentType> c );

  template< typename BindableContainer >
  iterator bind( BindableContainer & c );

  template< typename BindableIterator >
  iterator bind( BindableIterator first, BindableIterator last );

  template< typename BindableIterator >
  iterator bind( BindableIterator first, BindableIterator last
               , iterator from );

  template< typename ContainerType, typename ArgumentType >
  iterator operator()( sc_vector_assembly<ContainerType,ArgumentType> c );

  template< typename ArgumentContainer >
  iterator operator()( ArgumentContainer & c );

  template< typename ArgumentIterator >
  iterator operator()( ArgumentIterator first, ArgumentIterator last );

  template< typename ArgumentIterator >
  iterator operator()( ArgumentIterator first, ArgumentIterator last
                     , iterator from );

  // member-wise access

  template< typename MT >
  sc_vector_assembly<T,MT> assemble( MT (T::*member_ptr) )
    { return sc_vector_assembly<T,MT>( *this, member_ptr ); }

protected:

  void clear();

  virtual sc_object* object_cast( void* p ) const;

};

template< typename T, typename MT >
class sc_vector_assembly
{
  template< typename U > friend class sc_vector;

public:

  typedef sc_vector<T> base_type;

  typedef sc_vector_iter< T, sc_member_access<T, MT> >          iterator;
  typedef sc_vector_iter< const T
                        , sc_member_access<const T, const MT> > const_iterator;

  typedef T  element_type;
  typedef MT access_type;

  typedef typename base_type::size_type       size_type;
  typedef typename base_type::difference_type difference_type;
  typedef typename iterator::reference        reference;
  typedef typename iterator::pointer          pointer;
  typedef typename const_iterator::reference  const_reference;
  typedef typename const_iterator::pointer    const_pointer;


  typedef access_type (T::*member_type);

  const char* name() const;
  const char* kind() const;

  iterator begin();
  iterator end();

  const_iterator cbegin() const;
  const_iterator cend() const;

  const_iterator begin() const;
  const_iterator end()   const;

  size_type size() const;
  const std::vector< sc_object* > & get_elements() const;

  reference operator[]( size_type idx );
  reference at( size_type idx );
  const_reference operator[]( size_type idx ) const;
  const_reference at( size_type idx ) const;

  template< typename ContainerType, typename ArgumentType >
  iterator bind( sc_vector_assembly<ContainerType,ArgumentType> c );

  template< typename BindableContainer >
  iterator bind( BindableContainer & c );

  template< typename BindableIterator >
  iterator bind( BindableIterator first, BindableIterator last );

  template< typename BindableIterator >
  iterator bind( BindableIterator first, BindableIterator last
               , iterator from );

  template< typename BindableIterator >
  iterator bind( BindableIterator first, BindableIterator last
               , typename base_type::iterator from );

  template< typename ContainerType, typename ArgumentType >
  iterator operator()( sc_vector_assembly<ContainerType,ArgumentType> c );

  template< typename ArgumentContainer >
  iterator operator()( ArgumentContainer & c );

  template< typename ArgumentIterator >
  iterator operator()( ArgumentIterator first, ArgumentIterator last );

  template< typename ArgumentIterator >
  iterator operator()( ArgumentIterator first, ArgumentIterator last
                     , iterator from );

  template< typename ArgumentIterator >
  iterator operator()( ArgumentIterator first, ArgumentIterator last
                     , typename base_type::iterator from );

  sc_vector_assembly( const sc_vector_assembly & other );

  sc_vector_assembly& operator=( sc_vector_assembly other_copy );

  void swap( sc_vector_assembly & that );

  void report_empty_bind( const char* kind_, bool dst_empty_ ) const;

  ~sc_vector_assembly();

private:

  sc_vector_assembly( base_type & v, member_type ptr );

  sc_object* object_cast( pointer p ) const;

  base_type * vec_;
  member_type ptr_;

  mutable std::vector< sc_object* >* child_vec_;
};

template< typename T, typename MT >
sc_vector_assembly<T,MT>
sc_assemble_vector( sc_vector<T> & vec, MT (T::*ptr) )
{
  return vec.assemble( ptr );
}

template< typename T >
typename sc_vector<T>::element_type *
sc_vector<T>::create_element( const char* name, size_type /* idx */ )
{
  return new T( name );
}

template< typename T >
template< typename Creator >
void
sc_vector<T>::init( size_type n, Creator c )
{
  if ( base_type::check_init(n) )
  {
    base_type::reserve( n );
    try
    {
      for ( size_type i = 0; i<n; ++i )
      {
        // this workaround is needed for SystemC 2.2/2.3 sc_bind
        std::string  name  = make_name( basename(), i );
        const char*  cname = name.c_str();

        void* p = c( cname, i ) ; // call Creator
        base_type::push_back(p);
      }
    }
    catch ( ... )
    {
      clear();
      throw;
    }
  }
}

template< typename T >
void
sc_vector<T>::clear()
{
  size_type i = size();
  while ( i --> 0 )
  {
    delete &( (*this)[i] );
    base_type::at(i) = 0;
  }
  base_type::clear();
}

template< typename Container, typename ArgumentIterator >
typename Container::iterator
sc_vector_do_bind( Container & cont
                 , ArgumentIterator  first
                 , ArgumentIterator  last
                 , typename Container::iterator from )
{
  typename Container::iterator end = cont.end();

  if( !cont.size() || from == end || first == last )
      cont.report_empty_bind( cont.kind(), from == end );

  while( from!=end && first != last )
    (*from++).bind( *first++ );
  return from;
}

template< typename Container, typename ArgumentIterator >
typename Container::iterator
sc_vector_do_operator_paren( Container& cont
                           , ArgumentIterator  first
                           , ArgumentIterator  last
                           , typename Container::iterator from )
{
  typename Container::iterator end = cont.end();

  if( !cont.size() || from == end || first == last )
      cont.report_empty_bind( cont.kind(), from == end );

  while( from!=end && first != last )
    (*from++)( *first++ );
  return from;
}

template< typename T >
sc_vector<T>::~sc_vector()
{
  clear();
}

template< typename T, typename MT >
std::vector< sc_object* > const &
sc_vector_assembly<T,MT>::get_elements() const
{
  if( !child_vec_ )
    child_vec_ = new std::vector< sc_object* >;

  if( child_vec_->size() || !size() )
    return *child_vec_;

  child_vec_->reserve( size() );
  for( const_iterator it=begin(); it != end(); ++it )
    if( sc_object * obj = object_cast( const_cast<MT*>(&*it) ) )
      child_vec_->push_back( obj );

  return *child_vec_;
}

} // namespace sc_core
#undef SC_RPTYPE_
#undef SC_ENABLE_IF_

// $Log: sc_vector.h,v $
// Revision 1.17  2011/08/26 20:46:20  acg
//  Andy Goodrich: moved the modification log to the end of the file to
//  eliminate source line number skew when check-ins are done.
//
// Revision 1.16  2011/07/25 10:21:17  acg
//  Andy Goodrich: check in aftermath of call to automake.
//
// Revision 1.15  2011/04/02 00:04:32  acg
//  Philipp A. Hartmann: fix distance from member iterators, and
//  add iterator conversions.
//
// Revision 1.14  2011/04/01 22:35:19  acg
//  Andy Goodrich: spelling fix.
//
// Revision 1.13  2011/03/28 13:03:09  acg
//  Andy Goodrich: Philipp's latest update.
//
// Revision 1.12  2011/03/23 16:16:28  acg
//  Philipp A. Hartmann: rebase implementation on void*
//      - supports virtual inheritance from sc_object again
//      - build up get_elements result on demand
//      - still requires element type to be derived from sc_object
//
// Revision 1.11  2011/02/19 16:46:36  acg
//  Andy Goodrich: finally get the update from Philipp correct!
//

#endif // SC_VECTOR_H_INCLUDED_
// Taf!
