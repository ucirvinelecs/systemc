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

  sc_method_process.h -- Thread process declarations

  Original Author: Andy Goodrich, Forte Design Systems, 4 August 2005
               

  CHANGE LOG AT THE END OF THE FILE
 *****************************************************************************/


#if !defined(sc_method_process_h_INCLUDED)
#define sc_method_process_h_INCLUDED

#include "sysc/kernel/sc_spawn_options.h"
#include "sysc/kernel/sc_process.h"
#include "sysc/kernel/sc_cor.h"
#include "sysc/kernel/sc_event.h"
#include "sysc/kernel/sc_except.h"
#include "sysc/kernel/sc_reset.h"

// DEBUGGING MACROS:
//
// DEBUG_MSG(NAME,P,MSG)
//     MSG  = message to print
//     NAME = name that must match the process for the message to print, or
//            null if the message should be printed unconditionally.
//     P    = pointer to process message is for, or NULL in which case the
//            message will not print.
#if 0
#   define DEBUG_NAME ""
#   define DEBUG_MSG(NAME,P,MSG) \
    { \
        if ( P && ( (strlen(NAME)==0) || !strcmp(NAME,P->name())) ) \
          std::cout << "**** " << sc_time_stamp() << " ("  \
	            << sc_get_current_process_name() << "): " << MSG \
		    << " - " << P->name() << std::endl; \
    }
#else
#   define DEBUG_MSG(NAME,P,MSG) 
#endif

// 02/22/2016 ZC: to enable verbose display or not
#ifndef _SYSC_PRINT_VERBOSE_MESSAGE_ENV_VAR
#define _SYSC_PRINT_VERBOSE_MESSAGE_ENV_VAR "SYSC_PRINT_VERBOSE_MESSAGE"
#endif
namespace sc_core {

// forward references:
class sc_event_and_list;
class sc_event_or_list;
class sc_reset;
void sc_method_cor_fn( void* );
void sc_set_stack_size( sc_method_handle, std::size_t );
class sc_event;
class sc_join;
class sc_module;
class sc_process_handle;
class sc_process_table;
class sc_simcontext;
class sc_runnable;

class Invoker; //DM 05/16/2019

sc_cor* get_cor_pointer( sc_process_b* process_p );
void sc_set_stack_size( sc_method_handle thread_h, std::size_t size );

//DM 05/24/2019
void next_trigger( sc_simcontext* );
void next_trigger( const sc_event&, sc_simcontext* );
void next_trigger( const sc_event_or_list&, sc_simcontext* );
void next_trigger( const sc_event_and_list&, sc_simcontext* );
void next_trigger( const sc_time&, sc_simcontext* );
void next_trigger( const sc_time&, const sc_event&, sc_simcontext* );
void next_trigger( const sc_time&, const sc_event_or_list&, sc_simcontext* );
void next_trigger( const sc_time&, const sc_event_and_list&, sc_simcontext* );



/**************************************************************************//**
 *  \class sc_method_process
 *
 *  \brief A thread process.
 *****************************************************************************/
class sc_method_process : public sc_process_b {
    friend void sc_method_cor_fn( void* );
    friend void sc_set_stack_size( sc_method_handle, std::size_t );
    friend class sc_event;
    friend class sc_join;
    friend class sc_module;

    // 04/07/2015 GL: a new sc_channel class is derived from sc_module
    friend class sc_channel;

    friend class sc_process_b;
    friend class sc_process_handle;
    friend class sc_process_table;
    friend class sc_simcontext;
    friend class sc_runnable;
    friend sc_cor* get_cor_pointer( sc_process_b* process_p );

    friend class Invoker; //DM 05/16/2019
//DM 05/28/2019 removed seg id argument in next_trigger()
    friend void next_trigger( sc_simcontext* );
    friend void next_trigger( const sc_event&,
                  sc_simcontext* );
    friend void next_trigger( const sc_event_or_list&,
                  sc_simcontext* );
    friend void next_trigger( const sc_event_and_list&,
                  sc_simcontext* );
    friend void next_trigger( const sc_time&,
                  sc_simcontext* );
    friend void next_trigger( const sc_time&, const sc_event&,
                  sc_simcontext* );
    friend void next_trigger( const sc_time&, const sc_event_or_list&,
                  sc_simcontext* );
    friend void next_trigger( const sc_time&, const sc_event_and_list&,
                  sc_simcontext* );



  public:
  
	sc_event* waiting_event;
	sc_timestamp first_triggerable_time;
  sc_method_process( const char* name_p, bool free_host, 
      SC_ENTRY_FUNC method_p, sc_process_host* host_p, 
      const sc_spawn_options* opt_p );

  virtual const char* kind() const
      { return "sc_method_process"; }
	
	void aux_boundary();
  protected: 
    // may not be deleted manually (called from sc_process_b)
    virtual ~sc_method_process();

    virtual void disable_process( 
        sc_descendant_inclusion_info descendants = SC_NO_DESCENDANTS );
    virtual void enable_process( 
        sc_descendant_inclusion_info descendants = SC_NO_DESCENDANTS );
    virtual void kill_process(
        sc_descendant_inclusion_info descendants = SC_NO_DESCENDANTS );
    sc_method_handle next_exist(); 
    sc_method_handle next_runnable();
    virtual void prepare_for_simulation();
    /**
     *  \brief A new parameter segment ID is added for the out-of-order 
     *         simulation.
     */
    // 08/14/2015 GL: modified for the OoO simulation
    void clear_trigger( );

    /**
     *  \brief A new parameter segment ID is added for the out-of-order 
     *         simulation.
     */
    // 08/14/2015 GL: modified for the OoO simulation
    void next_trigger( const sc_event& );

    /**
     *  \brief A new parameter segment ID is added for the out-of-order 
     *         simulation.
     */
    // 08/14/2015 GL: modified for the OoO simulation
    void next_trigger( const sc_event_or_list& );

    /**
     *  \brief A new parameter segment ID is added for the out-of-order 
     *         simulation.
     */
    // 08/14/2015 GL: modified for the OoO simulation
    void next_trigger( const sc_event_and_list& );

    /**
     *  \brief A new parameter segment ID is added for the out-of-order 
     *         simulation.
     */
    // 08/14/2015 GL: modified for the OoO simulation
    void next_trigger( const sc_time& );

    /**
     *  \brief A new parameter segment ID is added for the out-of-order 
     *         simulation.
     */
    // 08/14/2015 GL: modified for the OoO simulation
    void next_trigger( const sc_time&, const sc_event& );

    /**
     *  \brief A new parameter segment ID is added for the out-of-order 
     *         simulation.
     */
    // 08/14/2015 GL: modified for the OoO simulation
    void next_trigger( const sc_time&, const sc_event_or_list& );

    /**
     *  \brief A new parameter segment ID is added for the out-of-order 
     *         simulation.
     */
    // 08/14/2015 GL: modified for the OoO simulation
    void next_trigger( const sc_time&, const sc_event_and_list& );

 


    virtual void resume_process( 
        sc_descendant_inclusion_info descendants = SC_NO_DESCENDANTS );
    void set_next_exist( sc_method_handle next_p );
    void set_next_runnable( sc_method_handle next_p );

    void set_stack_size( std::size_t size );

    virtual void suspend_process( 
        sc_descendant_inclusion_info descendants = SC_NO_DESCENDANTS );
    virtual void throw_reset( bool async );
    virtual void throw_user( const sc_throw_it_helper& helper,
        sc_descendant_inclusion_info descendants = SC_NO_DESCENDANTS );
 
    bool trigger_dynamic( sc_event*, bool& );
    void deliver_event_at_time( sc_event* e, sc_timestamp e_delivery_time );
	
    /**
     *  \brief A new parameter is added to update the local time stamp in the
     *         thread process.
     */
    // 08/14/2015 GL: add a new parameter to update the local time stamp
    //inline void trigger_static();
    inline void trigger_static( sc_event* );
	
  protected:
    void add_monitor( sc_process_monitor* monitor_p );
    void remove_monitor( sc_process_monitor* monitor_p);
    void signal_monitors( int type = 0 );

  protected:
    sc_cor*                          m_cor_p;        // Thread's coroutine.
    std::vector<sc_process_monitor*> m_monitor_q;    // Thread monitors.
    std::size_t                      m_stack_size;   // Thread stack size.
    int                              m_wait_cycle_n; // # of waits to be done.

  private: // disabled
    sc_method_process( const sc_method_process& );
    const sc_method_process& operator = ( const sc_method_process& );

};

//------------------------------------------------------------------------------
//"sc_method_process::set_stack_size"
//
//------------------------------------------------------------------------------
inline void sc_method_process::set_stack_size( std::size_t size )
{
    assert( size );
    m_stack_size = size;
}

//------------------------------------------------------------------------------
//"sc_method_process::next_trigger"
//
// Notes:
//   (1) The correct order to lock and unlock channel locks (to avoid deadlocks
//       and races) for SystemC functions without context switch:
//
//       outer_channel.lock_and_push
//           [outer channel work]
//           inner_channel.lock_and_push
//               [inner channel work]
//   +------------------------------NEXT_TRIGGER------------------------------+
//   |   +------------------------Simulation Kernel------------------------+  |
//   |   |       acquire kernel lock                                       |  |
//   |   |           [kernel work]                                         |  |
//   |   |       release kernel lock                                       |  |
//   |   +-----------------------------------------------------------------+  |
//   +------------------------------------------------------------------------+
//               [inner channel work]
//           inner_channel.pop_and_unlock
//           [outer channel work]
//       outer_channel.pop_and_unlock
//
//   (2) For more information, please refer to sc_thread_process.h: 272
//
// (02/20/2015 GL)
//------------------------------------------------------------------------------
inline
void
sc_method_process::next_trigger( const sc_event& e )
{
    // 05/25/2015 GL: sc_kernel_lock constructor acquires the kernel lock
    sc_kernel_lock lock;

#ifdef SC_LOCK_CHECK
    assert( sc_get_curr_simcontext()->is_locked_and_owner() );
#endif /* SC_LOCK_CHECK */
    clear_trigger( );
    e.add_dynamic( this );
    m_event_p = &e;
    m_trigger_type = EVENT;
    // 05/25/2015 GL: sc_kernel_lock destructor releases the kernel lock
}

inline
void
sc_method_process::next_trigger( const sc_event_or_list& el )
{
    // 05/25/2015 GL: sc_kernel_lock constructor acquires the kernel lock
    sc_kernel_lock lock;

#ifdef SC_LOCK_CHECK
    assert( sc_get_curr_simcontext()->is_locked_and_owner() );
#endif /* SC_LOCK_CHECK */
    clear_trigger( );
    el.add_dynamic( this );
    m_event_list_p = &el;
    m_trigger_type = OR_LIST;
    // 05/25/2015 GL: sc_kernel_lock destructor releases the kernel lock
}

inline
void
sc_method_process::next_trigger( const sc_event_and_list& el )
{
    // 05/25/2015 GL: sc_kernel_lock constructor acquires the kernel lock
    sc_kernel_lock lock;

#ifdef SC_LOCK_CHECK
    assert( sc_get_curr_simcontext()->is_locked_and_owner() );
#endif /* SC_LOCK_CHECK */
    clear_trigger( );
    el.add_dynamic( this );
    m_event_list_p = &el;
    m_event_count = el.size();
    m_trigger_type = AND_LIST;
    // 05/25/2015 GL: sc_kernel_lock destructor releases the kernel lock
}

inline
void
sc_method_process::next_trigger( const sc_time& t )
{
    // 05/25/2015 GL: sc_kernel_lock constructor acquires the kernel lock
    sc_kernel_lock lock;

#ifdef SC_LOCK_CHECK
    assert( sc_get_curr_simcontext()->is_locked_and_owner() );
#endif /* SC_LOCK_CHECK */
    clear_trigger( );
    m_timeout_event_p->notify_internal( t );
    m_timeout_event_p->add_dynamic( this );
    m_trigger_type = TIMEOUT;
    // 05/25/2015 GL: sc_kernel_lock destructor releases the kernel lock
}

inline
void
sc_method_process::next_trigger( const sc_time& t, const sc_event& e )
{
    // 05/25/2015 GL: sc_kernel_lock constructor acquires the kernel lock
    sc_kernel_lock lock;

#ifdef SC_LOCK_CHECK
    assert( sc_get_curr_simcontext()->is_locked_and_owner() );
#endif /* SC_LOCK_CHECK */
    clear_trigger( );
    m_timeout_event_p->notify_internal( t );
    m_timeout_event_p->add_dynamic( this );
    e.add_dynamic( this );
    m_event_p = &e;
    m_trigger_type = EVENT_TIMEOUT;
    // 05/25/2015 GL: sc_kernel_lock destructor releases the kernel lock
}

inline
void
sc_method_process::next_trigger( const sc_time& t, const sc_event_or_list& el )
{
    // 05/25/2015 GL: sc_kernel_lock constructor acquires the kernel lock
    sc_kernel_lock lock;

#ifdef SC_LOCK_CHECK
    assert( sc_get_curr_simcontext()->is_locked_and_owner() );
#endif /* SC_LOCK_CHECK */
    clear_trigger( );
    m_timeout_event_p->notify_internal( t );
    m_timeout_event_p->add_dynamic( this );
    el.add_dynamic( this );
    m_event_list_p = &el;
    m_trigger_type = OR_LIST_TIMEOUT;
    // 05/25/2015 GL: sc_kernel_lock destructor releases the kernel lock
}

inline
void
sc_method_process::next_trigger( const sc_time& t, const sc_event_and_list& el )
{
    // 05/25/2015 GL: sc_kernel_lock constructor acquires the kernel lock
    sc_kernel_lock lock;

#ifdef SC_LOCK_CHECK
    assert( sc_get_curr_simcontext()->is_locked_and_owner() );
#endif /* SC_LOCK_CHECK */
    clear_trigger( );
    m_timeout_event_p->notify_internal( t );
    m_timeout_event_p->add_dynamic( this );
    el.add_dynamic( this );
    m_event_list_p = &el;
    m_event_count = el.size();
    m_trigger_type = AND_LIST_TIMEOUT;
    // 05/25/2015 GL: sc_kernel_lock destructor releases the kernel lock
}

inline
void
sc_method_process::aux_boundary()
{
	
	
	
	
    if( m_unwinding )
        SC_REPORT_ERROR( SC_ID_WAIT_DURING_UNWINDING_, name() );

    {
        // 05/25/2015 GL: sc_kernel_lock constructor acquires the kernel lock
        sc_kernel_lock lock;

#ifdef SC_LOCK_CHECK
        assert( sc_get_curr_simcontext()->is_locked_and_owner() );
#endif /* SC_LOCK_CHECK */

        // 08/14/2015 GL: set the new segment ID of this thread
        set_segment_id( -2 ); 

        unlock_all_channels(); // 02/16/2015 GL: release all the channel locks
       
		
		//ZC 9:06 2017/3/14
		// if(getenv(_SYSC_PRINT_VERBOSE_MESSAGE_ENV_VAR))
		// 	printf("\n%s is calling wait for nothing\n",this->name());
		
		
        sc_get_curr_simcontext()->oooschedule( m_cor_p );
        // 05/25/2015 GL: sc_kernel_lock destructor releases the kernel lock
    }
#ifdef SC_LOCK_CHECK
    assert( sc_get_curr_simcontext()->is_not_owner() );
#endif /* SC_LOCK_CHECK */
    lock_all_channels(); // 02/16/2015 GL: acquire all the channel locks
	
	
	
}

//------------------------------------------------------------------------------
//"sc_method_process::miscellaneous support"
//
//------------------------------------------------------------------------------
inline
void sc_method_process::add_monitor(sc_process_monitor* monitor_p)
{
    m_monitor_q.push_back(monitor_p);
}


inline
void sc_method_process::remove_monitor(sc_process_monitor* monitor_p)
{
    int mon_n = m_monitor_q.size();

    for ( int mon_i = 0; mon_i < mon_n; mon_i++ )
    {
    if  ( m_monitor_q[mon_i] == monitor_p )
        {
            m_monitor_q[mon_i] = m_monitor_q[mon_n-1];
            m_monitor_q.resize(mon_n-1);
        }
    }
}

inline
void sc_method_process::set_next_exist(sc_method_handle next_p)
{
    m_exist_p = next_p;
}

inline
sc_method_handle sc_method_process::next_exist()
{
    return (sc_method_handle)m_exist_p;
}

inline
void sc_method_process::set_next_runnable(sc_method_handle next_p)
{
    m_runnable_p = next_p;
}

inline
sc_method_handle sc_method_process::next_runnable()
{
    return (sc_method_handle)m_runnable_p;
}

//------------------------------------------------------------------------------
//"sc_method_process::trigger_static"
//
// This inline method adds the current thread to the queue of runnable
// processes, if required.  This is the case if the following criteria
// are met:
//   (1) The process is in a runnable state.
//   (2) The process is not already on the run queue.
//   (3) The process is expecting a static trigger,
//       dynamic event waits take priority.
//   (4) The process' static wait count is zero.
//
// If the triggering process is the same process, the trigger is
// ignored as well, unless SC_ENABLE_IMMEDIATE_SELF_NOTIFICATIONS
// is defined.
//------------------------------------------------------------------------------
inline
void
// 08/14/2015 GL: add a new parameter to update the local time stamp
//sc_method_process::trigger_static()
sc_method_process::trigger_static( sc_event* e )
{
    // 05/05/2015 GL: we may or may not have acquired the kernel lock upon here
    // 1) this function is invoked in sc_simcontext::prepare_to_simulate(), 
    //    where the kernel lock is not acquired as it is in the initialization 
    //    phase
    // 2) this function is also invoked in sc_event::notify(), where the kernel 
    //    lock is acquired

    // No need to try queueing this thread if one of the following is true:
    //    (a) its disabled
    //    (b) its already queued for execution
    //    (c) its waiting on a dynamic event
    //    (d) its wait count is not satisfied

    if ( (m_state & ps_bit_disabled) || is_runnable() ||
          m_trigger_type != STATIC )
        return;

#if ! defined( SC_ENABLE_IMMEDIATE_SELF_NOTIFICATIONS )
    if( SC_UNLIKELY_( sc_get_current_process_b() == this ) )
    {
        report_immediate_self_notification();
        return;
    }
#endif // SC_ENABLE_IMMEDIATE_SELF_NOTIFICATIONS

    if ( m_wait_cycle_n > 0 )
    {
        --m_wait_cycle_n;
        return;
    }

    // If we get here then the thread is has satisfied its wait criteria, if 
    // its suspended mark its state as ready to run. If its not suspended then 
    // push it onto the runnable queue.

    if ( m_state & ps_bit_suspended )
    {
        m_state = m_state | ps_bit_ready_to_run;
    }
    else
    {
        // 12/22/2016 GL: store the current time before updating
        sc_time curr_time = m_timestamp.get_time_count();

        // 08/14/2015 GL: update the local time stamp of this thread process
        sc_timestamp ts = e->get_notify_timestamp();
        switch( e->m_notify_type )
        {
            case sc_event::DELTA: // delta notification
                if ( ts > m_timestamp ) {
                    set_timestamp( sc_timestamp( ts.get_time_count(),
                                                 ts.get_delta_count() + 1 ) );
                } else {
                    set_timestamp( sc_timestamp( m_timestamp.get_time_count(),
                                                 m_timestamp.get_delta_count() 
                                                     + 1 ) );
                }
                break;
            case sc_event::TIMED: // timed notification
                set_timestamp( ts );
                break;
            case sc_event::NONE:
                assert( 0 ); // wrong type
        }

	simcontext()->push_runnable_method(this);

        // 12/22/2016 GL: update m_oldest_time in sc_simcontext if necessary
        simcontext()->update_oldest_time( curr_time );
    }
}

#undef DEBUG_MSG
#undef DEBUG_NAME

} // namespace sc_core 

// $Log: sc_method_process.h,v $
// Revision 1.30  2011/08/26 20:46:11  acg
//  Andy Goodrich: moved the modification log to the end of the file to
//  eliminate source line number skew when check-ins are done.
//
// Revision 1.29  2011/08/24 23:36:12  acg
//  Andy Goodrich: removed break statements that can never be reached and
//  which causes warnings in the Greenhills C++ compiler.
//
// Revision 1.28  2011/04/14 22:34:27  acg
//  Andy Goodrich: removed dead code.
//
// Revision 1.27  2011/04/13 05:02:18  acg
//  Andy Goodrich: added missing check to the wake up code in suspend_me()
//  so that we just return if the call to suspend_me() was issued from a
//  stack unwinding.
//
// Revision 1.26  2011/04/13 02:44:26  acg
//  Andy Goodrich: added m_unwinding flag in place of THROW_NOW because the
//  throw status will be set back to THROW_*_RESET if reset is active and
//  the check for an unwind being complete was expecting THROW_NONE as the
//  clearing of THROW_NOW.
//
// Revision 1.25  2011/04/11 22:05:14  acg
//  Andy Goodrich: use the DEBUG_NAME macro in DEBUG_MSG invocations.
//
// Revision 1.24  2011/04/10 22:12:32  acg
//  Andy Goodrich: adding debugging macros.
//
// Revision 1.23  2011/04/08 22:41:28  acg
//  Andy Goodrich: added comment pointing to the description of the reset
//  mechanism in sc_reset.cpp.
//
// Revision 1.22  2011/04/08 18:27:33  acg
//  Andy Goodrich: added check to make sure we don't schedule a running process
//  because of it issues a notify() it is sensitive to.
//
// Revision 1.21  2011/04/05 06:22:38  acg
//  Andy Goodrich: expanded comment for trigger_static() initial vetting.
//
// Revision 1.20  2011/04/01 21:24:57  acg
//  Andy Goodrich: removed unused code.
//
// Revision 1.19  2011/02/19 08:30:53  acg
//  Andy Goodrich: Moved process queueing into trigger_static from
//  sc_event::notify.
//
// Revision 1.18  2011/02/18 20:27:14  acg
//  Andy Goodrich: Updated Copyrights.
//
// Revision 1.17  2011/02/17 19:55:58  acg
//  Andy Goodrich:
//    (1) Changed signature of trigger_dynamic() back to a bool.
//    (2) Simplified process control usage.
//    (3) Changed trigger_static() to recognize process controls and to
//        do the down-count on wait(N), allowing the elimination of
//        ready_to_run().
//
// Revision 1.16  2011/02/16 22:37:31  acg
//  Andy Goodrich: clean up to remove need for ps_disable_pending.
//
// Revision 1.15  2011/02/13 21:47:38  acg
//  Andy Goodrich: update copyright notice.
//
// Revision 1.14  2011/02/13 21:35:54  acg
//  Andy Goodrich: added error for performing a wait() during unwinding.
//
// Revision 1.13  2011/02/11 13:25:24  acg
//  Andy Goodrich: Philipp A. Hartmann's changes:
//    (1) Removal of SC_CTHREAD method overloads.
//    (2) New exception processing code.
//
// Revision 1.12  2011/02/01 23:01:53  acg
//  Andy Goodrich: removed dead code.
//
// Revision 1.11  2011/02/01 21:18:01  acg
//  Andy Goodrich:
//  (1) Changes in throw processing for new process control rules.
//  (2) Support of new process_state enum values.
//
// Revision 1.10  2011/01/25 20:50:37  acg
//  Andy Goodrich: changes for IEEE 1666 2011.
//
// Revision 1.9  2011/01/19 23:21:50  acg
//  Andy Goodrich: changes for IEEE 1666 2011
//
// Revision 1.8  2011/01/18 20:10:45  acg
//  Andy Goodrich: changes for IEEE1666_2011 semantics.
//
// Revision 1.7  2011/01/06 17:59:58  acg
//  Andy Goodrich: removed debugging output.
//
// Revision 1.6  2010/07/22 20:02:33  acg
//  Andy Goodrich: bug fixes.
//
// Revision 1.5  2009/07/28 01:10:53  acg
//  Andy Goodrich: updates for 2.3 release candidate.
//
// Revision 1.4  2009/05/22 16:06:29  acg
//  Andy Goodrich: process control updates.
//
// Revision 1.3  2009/03/12 22:59:58  acg
//  Andy Goodrich: updates for 2.4 stuff.
//
// Revision 1.2  2008/05/22 17:06:06  acg
//  Andy Goodrich: formatting and comments.
//
// Revision 1.1.1.1  2006/12/15 20:20:05  acg
// SystemC 2.3
//
// Revision 1.7  2006/05/08 17:57:13  acg
//  Andy Goodrich: Added David Long's forward declarations for friend functions
//  to keep the Microsoft C++ compiler happy.
//
// Revision 1.6  2006/04/20 17:08:17  acg
//  Andy Goodrich: 3.0 style process changes.
//
// Revision 1.5  2006/04/11 23:13:21  acg
//   Andy Goodrich: Changes for reduced reset support that only includes
//   sc_cthread, but has preliminary hooks for expanding to method and thread
//   processes also.
//
// Revision 1.4  2006/01/24 20:49:05  acg
// Andy Goodrich: changes to remove the use of deprecated features within the
// simulator, and to issue warning messages when deprecated features are used.
//
// Revision 1.3  2006/01/13 18:44:30  acg
// Added $Log to record CVS changes into the source.

#endif // !defined(sc_method_process_h_INCLUDED)
