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

  sc_event.h --

  Original Author: Martin Janssen, Synopsys, Inc., 2001-05-21

  CHANGE LOG AT THE END OF THE FILE
 *****************************************************************************/


#ifndef SC_EVENT_H
#define SC_EVENT_H

#include "sysc/kernel/sc_cmnhdr.h"
#include "sysc/kernel/sc_kernel_ids.h"
#include "sysc/kernel/sc_simcontext.h"
#include "sysc/communication/sc_writer_policy.h"

namespace sc_core {

// forward declarations
class sc_event;
class sc_event_timed;
class sc_event_list;
class sc_event_or_list;
class sc_event_and_list;
class sc_object;

// friend function declarations
    int sc_notify_time_compare( const void*, const void* );

// ----------------------------------------------------------------------------
//  CLASS : sc_event_expr
//
//  The event expression class.
// ----------------------------------------------------------------------------

template< typename T >
class sc_event_expr
{
    friend class sc_event;
    friend class sc_event_and_list;
    friend class sc_event_or_list;

    typedef T type;

    sc_event_expr();

public:

    sc_event_expr( sc_event_expr const & e); // move semantics

    T const & release() const;

    void push_back( sc_event const & e) const;

    void push_back( type const & el) const;

    operator T const &() const;

    ~sc_event_expr();

private:
    mutable type * m_expr;

    // disabled
    void operator=( sc_event_expr const & );
};

// ----------------------------------------------------------------------------
//  CLASS : sc_event_list
//
//  Base class for lists of events.
// ----------------------------------------------------------------------------

class sc_event_list
{
    friend class sc_process_b;
    friend class sc_method_process;
    friend class sc_thread_process;
    friend void sc_thread_cor_fn( void* arg );

public:
    sc_event_list( const sc_event_list& );
    sc_event_list& operator = ( const sc_event_list& );

    int size() const;

protected:

    void push_back( const sc_event& );
    void push_back( const sc_event_list& );

    explicit
    sc_event_list( bool and_list_, bool auto_delete_ = false );

    sc_event_list( const sc_event&,
                   bool and_list_,
                   bool auto_delete_ = false );

    ~sc_event_list();

    void swap( sc_event_list& );
    void move_from( const sc_event_list& );

    bool and_list() const;

    void add_dynamic( sc_method_handle ) const;
    void add_dynamic( sc_thread_handle ) const;
    void remove_dynamic( sc_method_handle, const sc_event* ) const;
    void remove_dynamic( sc_thread_handle, const sc_event* ) const;

    bool busy()        const;
    bool temporary()   const;
    void auto_delete() const;

    void report_premature_destruction() const;
    void report_invalid_modification()  const;

public:
    std::vector<const sc_event*> m_events;

    bool                         m_and_list;
    bool                         m_auto_delete;
    mutable unsigned             m_busy;
};


// ----------------------------------------------------------------------------
//  CLASS : sc_event_and_list
//
//  AND list of events.
// ----------------------------------------------------------------------------

class sc_event_and_list
: public sc_event_list
{
    friend class sc_event;
    friend class sc_event_expr<sc_event_and_list>;
    friend class sc_process_b;
    friend class sc_method_process;
    friend class sc_thread_process;

protected:

    explicit
    sc_event_and_list( bool auto_delete_ );

public:

    sc_event_and_list();
    sc_event_and_list( const sc_event& );

    void swap( sc_event_and_list& );
    sc_event_and_list& operator &= ( const sc_event& );
    sc_event_and_list& operator &= ( const sc_event_and_list & );

    sc_event_expr<sc_event_and_list>  operator & ( const sc_event& );
    sc_event_expr<sc_event_and_list>  operator & ( const sc_event_and_list& );
};

typedef sc_event_expr<sc_event_and_list> sc_event_and_expr;

// ----------------------------------------------------------------------------
//  CLASS : sc_event_or_list
//
//  OR list of events.
// ----------------------------------------------------------------------------

class sc_event_or_list
: public sc_event_list
{
    friend class sc_event;
    friend class sc_event_expr<sc_event_or_list>;
    friend class sc_process_b;
    friend class sc_method_process;
    friend class sc_thread_process;

protected:

    explicit
    sc_event_or_list( bool auto_delete_ );

public:
    sc_event_or_list();
    sc_event_or_list( const sc_event& );
    void swap( sc_event_or_list& );
    sc_event_or_list& operator |= ( const sc_event& );
    sc_event_or_list& operator |= ( const sc_event_or_list & );
    sc_event_expr<sc_event_or_list>  operator | ( const sc_event& ) const;
    sc_event_expr<sc_event_or_list>  operator | ( const sc_event_or_list& ) const;
};

typedef sc_event_expr<sc_event_or_list> sc_event_or_expr;

// ----------------------------------------------------------------------------
//  CLASS : sc_event
//
//  The event class.
// ----------------------------------------------------------------------------

class sc_event
{
    friend class sc_clock;
    friend class sc_event_list;
    friend class sc_event_timed;
    friend class sc_simcontext;
    friend class sc_object;
    friend class sc_process_b;
    friend class sc_method_process;
    friend class sc_thread_process;
    template<typename IF, sc_writer_policy POL> friend class sc_signal;
    friend void sc_thread_cor_fn( void* arg );

public:

    sc_event();
    sc_event( const char* name );
    ~sc_event();

    void cancel();

    const char* name() const;
    const char* basename() const;
    sc_object* get_parent_object() const;
 
    bool in_hierarchy() const;

    void notify();
    void notify( const sc_time& );
    void notify( double, sc_time_unit );

    void notify_delayed();
    void notify_delayed( const sc_time& );
    void notify_delayed( double, sc_time_unit );

    sc_event_or_expr  operator | ( const sc_event& ) const; 
    sc_event_or_expr  operator | ( const sc_event_or_list& ) const;
    sc_event_and_expr operator & ( const sc_event& ) const;
    sc_event_and_expr operator & ( const sc_event_and_list& ) const;


private:

    void add_static( sc_method_handle ) const;
    void add_static( sc_thread_handle ) const;
    void add_dynamic( sc_method_handle ) const;
    void add_dynamic( sc_thread_handle ) const;

    void notify_internal( const sc_time& );
    void notify_next_delta();

    bool remove_static( sc_method_handle ) const;
    bool remove_static( sc_thread_handle ) const;
    bool remove_dynamic( sc_method_handle ) const;
    bool remove_dynamic( sc_thread_handle ) const;

    void register_event( const char* name );
    void reset();

    void trigger();

private:

    enum notify_t { NONE, DELTA, TIMED };

    std::string     m_name;     // name of object.
    sc_object*      m_parent_p; // parent sc_object for this event.
    sc_simcontext*  m_simc;
    notify_t        m_notify_type;
    int             m_delta_event_index;
    sc_event_timed* m_timed;

    mutable std::vector<sc_method_handle> m_methods_static;
    mutable std::vector<sc_method_handle> m_methods_dynamic;
    mutable std::vector<sc_thread_handle> m_threads_static;
    mutable std::vector<sc_thread_handle> m_threads_dynamic;

private:

    // disabled
    sc_event( const sc_event& );
    sc_event& operator = ( const sc_event& );
};

#define SC_KERNEL_EVENT_PREFIX "$$$$kernel_event$$$$_"

extern sc_event sc_non_event; // Event that never happens.

// ----------------------------------------------------------------------------
//  CLASS : sc_event_timed
//
//  Class for storing the time to notify a timed event.
// ----------------------------------------------------------------------------

class sc_event_timed
{
    friend class sc_event;
    friend class sc_simcontext;

    friend int sc_notify_time_compare( const void*, const void* );

private:

    sc_event_timed( sc_event* e, const sc_time& t );

    ~sc_event_timed();

    sc_event* event() const;

    const sc_time& notify_time() const;

    static void* operator new( std::size_t );

    static void operator delete( void* p, std::size_t );

private:

    // dedicated memory management
    static void* allocate();
    static void  deallocate( void* );

private:

    sc_event* m_event;
    sc_time   m_notify_time;

private:

    // disabled
    sc_event_timed();
    sc_event_timed( const sc_event_timed& );
    sc_event_timed& operator = ( const sc_event_timed& );
};


// ----------------------------------------------------------------------------
//  Deprecated functional notation for notifying events.
// ----------------------------------------------------------------------------

extern void notify( sc_event& e );
extern void notify( const sc_time& t, sc_event& e );
extern void notify( double v, sc_time_unit tu, sc_event& e );

// IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII


// sc_event_expr

sc_event_or_expr
operator | ( sc_event_or_expr expr, sc_event const & e );

sc_event_or_expr
operator | ( sc_event_or_expr expr, sc_event_or_list const & el );


// IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII


// sc_event_expr

sc_event_and_expr
operator & ( sc_event_and_expr expr, sc_event const & e );

sc_event_and_expr
operator & ( sc_event_and_expr expr, sc_event_and_list const & el );

} // namespace sc_core

// $Log: sc_event.h,v $
// Revision 1.14  2011/08/29 18:04:32  acg
//  Philipp A. Hartmann: miscellaneous clean ups.
//
// Revision 1.13  2011/08/26 20:46:09  acg
//  Andy Goodrich: moved the modification log to the end of the file to
//  eliminate source line number skew when check-ins are done.
//
// Revision 1.12  2011/08/24 22:05:50  acg
//  Torsten Maehne: initialization changes to remove warnings.
//
// Revision 1.11  2011/03/12 21:07:51  acg
//  Andy Goodrich: changes to kernel generated event support.
//
// Revision 1.10  2011/03/06 15:55:11  acg
//  Andy Goodrich: Changes for named events.
//
// Revision 1.9  2011/03/05 01:39:21  acg
//  Andy Goodrich: changes for named events.
//
// Revision 1.8  2011/02/18 20:27:14  acg
//  Andy Goodrich: Updated Copyrights.
//
// Revision 1.7  2011/02/13 21:47:37  acg
//  Andy Goodrich: update copyright notice.
//
// Revision 1.6  2011/02/01 21:03:23  acg
//  Andy Goodrich: new return codes for trigger_dynamic calls.
//
// Revision 1.5  2011/01/18 20:10:44  acg
//  Andy Goodrich: changes for IEEE1666_2011 semantics.
//
// Revision 1.4  2010/12/07 20:09:11  acg
// Andy Goodrich: writer policy fix.
//
// Revision 1.3  2009/05/22 16:06:29  acg
//  Andy Goodrich: process control updates.
//
// Revision 1.2  2008/05/22 17:06:25  acg
//  Andy Goodrich: updated copyright notice to include 2008.
//
// Revision 1.1.1.1  2006/12/15 20:20:05  acg
// SystemC 2.3
//
// Revision 1.8  2006/05/26 20:33:16  acg
//   Andy Goodrich: changes required by additional platform compilers (i.e.,
//   Microsoft VC++, Sun Forte, HP aCC).
//
// Revision 1.7  2006/05/08 17:57:51  acg
// Andy Goodrich: added David Long's forward declarations for friend
//   functions, methods, and operators to keep the Microsoft compiler happy.
//
// Revision 1.6  2006/04/11 23:13:20  acg
//   Andy Goodrich: Changes for reduced reset support that only includes
//   sc_cthread, but has preliminary hooks for expanding to method and thread
//   processes also.
//
// Revision 1.5  2006/01/24 20:56:00  acg
//  Andy Goodrich: fixed up CVS comment.
//
// Revision 1.4  2006/01/24 20:48:14  acg
// Andy Goodrich: added deprecation warnings for notify_delayed(). Added two
// new implementation-dependent methods, notify_next_delta() & notify_internal()
// to replace calls to notify_delayed() from within the simulator. These two
// new methods are simpler than notify_delayed() and should speed up simulations
//
// Revision 1.3  2006/01/13 18:44:29  acg
// Added $Log to record CVS changes into the source.
//

#endif

// Taf!
