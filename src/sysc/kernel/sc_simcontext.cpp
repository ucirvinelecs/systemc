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

  sc_simcontext.cpp -- Provides a simulation context for use with multiple
                       simulations.

  Original Author: Stan Y. Liao, Synopsys, Inc.
                   Martin Janssen, Synopsys, Inc.

  CHANGE LOG AT THE END OF THE FILE
 *****************************************************************************/

#include <algorithm>

#define SC_DISABLE_API_VERSION_CHECK // for in-library sc_ver.h inclusion

#include "sysc/kernel/sc_cor_fiber.h"
#include "sysc/kernel/sc_cor_pthread.h"
#include "sysc/kernel/sc_cor_qt.h"
#include "sysc/kernel/sc_event.h"
#include "sysc/kernel/sc_kernel_ids.h"
#include "sysc/kernel/sc_module.h"
#include "sysc/kernel/sc_module_registry.h"
#include "sysc/kernel/sc_name_gen.h"
#include "sysc/kernel/sc_object_manager.h"
#include "sysc/kernel/sc_cthread_process.h"
#include "sysc/kernel/sc_method_process.h"
#include "sysc/kernel/sc_thread_process.h"
#include "sysc/kernel/sc_process_handle.h"
#include "sysc/kernel/sc_simcontext.h"
#include "sysc/kernel/sc_simcontext_int.h"
#include "sysc/kernel/sc_reset.h"
#include "sysc/kernel/sc_ver.h"
#include "sysc/kernel/sc_boost.h"
#include "sysc/kernel/sc_spawn.h"
#include "sysc/kernel/sc_phase_callback_registry.h"
#include "sysc/communication/sc_port.h"
#include "sysc/communication/sc_export.h"
#include "sysc/communication/sc_prim_channel.h"
#include "sysc/tracing/sc_trace.h"
#include "sysc/utils/sc_mempool.h"
#include "sysc/utils/sc_list.h"
#include "sysc/utils/sc_utils_ids.h"

#include <string>
#include <set>
#include <map> // 09/02/2015 GL: to include std::map for index lookup
#include <unordered_map>
#include <math.h>
#include <fstream>
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

#ifndef SC_NO_THREADS
#define SC_NO_THREADS ((sc_thread_handle)0xdb)
#endif

#if SC_HAS_PHASE_CALLBACKS_
#  define SC_DO_PHASE_CALLBACK_( Kind ) \
    m_phase_cb_registry->Kind()
#else
#  define SC_DO_PHASE_CALLBACK_( Kind ) \
    ((void)0) /* do nothing */
#endif

#if defined( SC_ENABLE_SIMULATION_PHASE_CALLBACKS_TRACING )
// use callback based tracing
#  define SC_SIMCONTEXT_TRACING_  0
#else
// enable tracing via explicit trace_cycle calls from simulator loop
#  define SC_SIMCONTEXT_TRACING_  1
#endif

// 04/06/2015 GL: set the number of simulation cores
#ifndef _SYSC_DEFAULT_PAR_SIM_CPUS
#define _SYSC_DEFAULT_PAR_SIM_CPUS 64
#endif

#ifndef _SYSC_PAR_SIM_CPUS_ENV_VAR
#define _SYSC_PAR_SIM_CPUS_ENV_VAR "SYSC_PAR_SIM_CPUS"
#endif

// 06/16/2016 GL: to enable synchronized parallel simulation or not
#ifndef _SYSC_DEFAULT_SYNC_PAR_SIM
#define _SYSC_DEFAULT_SYNC_PAR_SIM false
#endif

#ifndef _SYSC_SYNC_PAR_SIM_ENV_VAR
#define _SYSC_SYNC_PAR_SIM_ENV_VAR "SYSC_SYNC_PAR_SIM"
#endif

// 02/22/2016 ZC: to enable prediction or not
#ifndef _SYSC_PREDICTION_SWITCH_FLAG_ENV_VAR
#define _SYSC_PREDICTION_SWITCH_FLAG_ENV_VAR "SYSC_DISABLE_PREDICTION"
#endif

// 02/22/2016 ZC: to enable verbose display or not
#ifndef _SYSC_PRINT_VERBOSE_MESSAGE_ENV_VAR
#define _SYSC_PRINT_VERBOSE_MESSAGE_ENV_VAR "SYSC_PRINT_VERBOSE_MESSAGE"
#endif

#ifndef _SYSC_PRINT_MODE_MESSAGE_ENV_VAR
#define _SYSC_PRINT_MODE_MESSAGE_ENV_VAR "SYSC_PRINT_MODE_MESSAGE"
#endif

// 03/22/2019 ZC : add environmental variables for debugging logs
#ifndef _SYSC_VERBOSITY_FLAG_1
#define _SYSC_VERBOSITY_FLAG_1 "SYSC_VERBOSITY_FLAG_1"
#endif

#ifndef _SYSC_VERBOSITY_FLAG_2
#define _SYSC_VERBOSITY_FLAG_2 "SYSC_VERBOSITY_FLAG_2"
#endif

#ifndef _SYSC_VERBOSITY_FLAG_3
#define _SYSC_VERBOSITY_FLAG_3 "SYSC_VERBOSITY_FLAG_3"
#endif

#ifndef _SYSC_VERBOSITY_FLAG_4
#define _SYSC_VERBOSITY_FLAG_4 "SYSC_VERBOSITY_FLAG_4"
#endif

#ifndef _SYSC_VERBOSITY_FLAG_5
#define _SYSC_VERBOSITY_FLAG_5 "SYSC_VERBOSITY_FLAG_5"
#endif

#ifndef _SYSC_VERBOSITY_FLAG_6
#define _SYSC_VERBOSITY_FLAG_6 "SYSC_VERBOSITY_FLAG_6"
#endif

#ifndef _SYSC_VERBOSITY_FLAG
#define _SYSC_VERBOSITY_FLAG "SYSC_VERBOSITY_FLAG"
#endif


// 12/2202/16 GL: set the maximum run-ahead time interval
//#ifndef _SYSC_DEFAULT_RUN_AHEAD_MAX_IN_MS
//#define _SYSC_DEFAULT_RUN_AHEAD_MAX_IN_MS 1000
//#endif

/*
#ifndef _SYSC_RUN_AHEAD_MAX_ENV_VAR
#define _SYSC_RUN_AHEAD_MAX_ENV_VAR "SYSC_RUN_AHEAD_MAX_IN_MS"
#endif
*/
typedef unsigned long long cycles_t; //measure cpu cycles

namespace sc_core{

//measure cpu cycles
static inline cycles_t currentcycles( void ) 
{
    cycles_t hi, lo;
    //assembly code, no header needed, __asm__ stands for assembly
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi)); 
    return (lo | (hi << 32));
}

class Invoker : public sc_module {

    friend class sc_simcontext;
    
    void method_invoker();
    void suspend_invoker();

    SC_HAS_PROCESS(Invoker);
    Invoker(sc_module_name name);

    sc_thread_handle proc_handle;
    std::list<sc_process_b*> method_queue;
    
};


Invoker::Invoker(sc_module_name name) {
     class sc_process_handle method_invoker_handle = sc_core::sc_get_curr_simcontext() ->  create_invoker_process ("invoker",false,(static_cast < sc_core::SC_ENTRY_FUNC  >  ((&Invoker::method_invoker))),(this),0,0 /*invoker_id*/);
      (this) -> sensitive << method_invoker_handle;
      (this) -> sensitive_pos << method_invoker_handle;
      (this) -> sensitive_neg << method_invoker_handle;

    proc_handle = method_invoker_handle;

}

void Invoker::method_invoker() {
    while(1) {
        for(std::list<sc_process_b*>::iterator method_iter = method_queue.begin();
            method_iter != method_queue.end(); method_iter++) {
            sc_process_b* current_method = (*method_iter);
            SC_ENTRY_FUNC func_ptr = current_method->m_semantics_method_p;
            sc_module* current_mod = DCAST<sc_module*> (current_method->m_semantics_host_p);
            ( (sc_process_b*) proc_handle)->cur_invoker_method_handle = current_method;
            DCAST<sc_method_process*>(current_method)->clear_trigger();
            current_mod->invoke_method(func_ptr);
        }
        {   
            sc_kernel_lock lock;

#ifdef SC_LOCK_CHECK
                assert( sc_get_curr_simcontext()->is_locked_and_owner() );
#endif

            while( !method_queue.empty() ) {
                sc_process_b* current_method = method_queue.front();
                switch(current_method->m_trigger_type) {
                    case sc_process_b::STATIC:
                        current_method->m_sensitivity_events->add_dynamic(RCAST<sc_method_handle>( current_method ));
                        current_method->m_event_list_p = current_method->m_sensitivity_events;
                        current_method->m_event_count = current_method->m_sensitivity_events->size();
                        current_method->m_trigger_type = sc_process_b::OR_LIST;
                        current_method->m_process_state=2;
                        sc_get_curr_simcontext()->add_to_wait_queue( current_method );
                        break;
                    case sc_process_b::EVENT:
                        current_method->m_process_state=2;
                        sc_get_curr_simcontext()->add_to_wait_queue( current_method );
                        break;
                    case sc_process_b::OR_LIST:
                        current_method->m_process_state=2;
                        sc_get_curr_simcontext()->add_to_wait_queue( current_method );
                        break;
                    case sc_process_b::AND_LIST:
                        current_method->m_process_state=2;
                        sc_get_curr_simcontext()->add_to_wait_queue( current_method );
                        break;
                    case sc_process_b::TIMEOUT:
                        current_method->m_process_state=3;
                        break;
                    case sc_process_b::EVENT_TIMEOUT:
                        break;
                    case sc_process_b::OR_LIST_TIMEOUT:
                        break;
                    case sc_process_b::AND_LIST_TIMEOUT:
                        break;
                    default:
                        assert(0);
                }
                sc_get_curr_simcontext()->remove_running_process( current_method );
                method_queue.pop_front();
            }
            suspend_invoker();
        }
    }
}

void Invoker::suspend_invoker() {
    sc_simcontext* simc_p = simcontext();
    simc_p->running_invokers.erase(this);
    simc_p->oooschedule( proc_handle->m_cor_p );
        
    // if I am not scheduled to execute again
    if ( simc_p->running_invokers.count( this ) == 0 ) {
        DEBUG_MSG( DEBUG_NAME , this, "suspending thread");
        simc_p->suspend_cor( proc_handle->m_cor_p );
        DEBUG_MSG( DEBUG_NAME , this, "resuming thread");
    }
}
    
/* the use of already_checked is for prediction of conflict.
    for example, seg 1 is to be issued and thus check for any potential 
    conflict, and seg 2 is concurrent. We have to check whether seg 2 will 
    wake up any other seg 3 that may conflict with seg 1. But when seg 3 
    also wakes up seg 2, then there will be a forever loop. So we use 
    AlREADY_CHECK to avoid this, such that when seg 2 is already checked,
    ALREADY_CHECKED[seg 2] is set to true
    ZC
    */
std::string _OoO_File_Name_;
std::string _OoO_Table_File_Name_;
bool verbosity_flag_1;
bool verbosity_flag_2;
bool verbosity_flag_3;
bool verbosity_flag_4;
bool verbosity_flag_5;
bool verbosity_flag_6;
bool verbosity_flag;
bool print_verbose_message;
bool print_mode_message;
bool sych_mode_defined;
// 02/22/2017 ZC: enable prediction or not
bool prediction_switch;
// 03/22/2019 ZC : add environmental variables for debugging logs


char* num_cpus; 
// 04/06/2015 GL: the number of simulation cores
unsigned int _SYSC_NUM_SIM_CPUs;

// 06/16/2016 GL: enable synchronized parallel simulation
bool _SYSC_SYNC_PAR_SIM = _SYSC_DEFAULT_SYNC_PAR_SIM;

// 12/22/2016 GL: the maximum run-ahead time interval
//sc_time _SYSC_RUN_AHEAD_MAX;

sc_stop_mode stop_mode = SC_STOP_FINISH_DELTA;

// 11/05/2014 GL: make empty_eval_phase a global varible so that mapper can 
//                communicate with crunch
//bool empty_eval_phase = true;

//------------------------------------------------------------------------------
// "get_cor_pointer"
//
// This method returns m_cor_p in sc_method_process or
// sc_(c)thread_process.
//
// 05/22/2015 GL: moved from sc_thread_process.h
//------------------------------------------------------------------------------
sc_cor* get_cor_pointer( sc_process_b* process_p )
{
    if ( !process_p ) // the root thread
        return sc_get_curr_simcontext()->m_cor;
    switch ( process_p->proc_kind() )
    {
        case SC_THREAD_PROC_:
        case SC_CTHREAD_PROC_:
        {
            sc_thread_handle thread_p = DCAST<sc_thread_handle>(process_p);
            return thread_p->m_cor_p;
        }
        case SC_METHOD_PROC_:
        {
            sc_method_handle method_p = DCAST<sc_method_handle>(process_p);
            return method_p->m_cor_p;
        }
        default:
        {
            SC_REPORT_ERROR(SC_ID_UNKNOWN_PROCESS_TYPE_, process_p->name());
            return NULL;
        }
    }
}

// 05/22/2015 GL: constructor & destructor for sc_kernel_lock
sc_kernel_lock::sc_kernel_lock()
{
    simc_p = sc_get_curr_simcontext();
    m_cor_p = get_cor_pointer( simc_p->get_curr_proc() );

    if ( !m_cor_p ) // m_cor_p is 0 in the elaboration phase
    {
        assert( !simc_p->m_elaboration_done );
        return; // only the root thread is running in the elaboration phase
    }

    if ( simc_p->is_locked_and_owner() ) 
        m_cor_p->increment_counter();
    else // acquire the kernel lock to protect the simulation kernel
        simc_p->acquire_sched_mutex();
}

sc_kernel_lock::~sc_kernel_lock()
{
    if ( !m_cor_p ) // m_cor_p is 0 in the elaboration phase
    {
        assert( !simc_p->m_elaboration_done );
        return; // only the root thread is running in the elaboration phase
    }

    if ( m_cor_p->get_counter() )
        m_cor_p->decrement_counter();
    else
        simc_p->release_sched_mutex(); // release the kernel lock
}


// ----------------------------------------------------------------------------
//  CLASS : sc_process_table
//
//  Container class that keeps track of all method processes,
//  (c)thread processes.
// ----------------------------------------------------------------------------

class sc_process_table
{
  public:

    sc_process_table();
    ~sc_process_table();
    void push_front( sc_method_handle );
    void push_front( sc_thread_handle );
    sc_method_handle method_q_head();
    sc_method_handle remove( sc_method_handle );
    sc_thread_handle thread_q_head();
    sc_thread_handle remove( sc_thread_handle );


  private:

    sc_method_handle  m_method_q;  // Queue of existing method processes.
    sc_thread_handle  m_thread_q;  // Queue of existing thread processes.
};

sc_process_table::sc_process_table() : m_method_q(0), m_thread_q(0)
{}

sc_process_table::~sc_process_table()
{

    sc_method_handle  method_next_p;    // Next method to delete.
    sc_method_handle  method_now_p; // Method now deleting.

    for( method_now_p = m_method_q; method_now_p; method_now_p = method_next_p )
    {
    method_next_p = method_now_p->next_exist();
    delete method_now_p;
    }

    if ( m_thread_q )
    {
        ::std::cout << ::std::endl 
             << "WATCH OUT!! In sc_process_table destructor. "
             << "Threads and cthreads are not actually getting deleted here. "
         << "Some memory may leak. Look at the comments here in "
         << "kernel/sc_simcontext.cpp for more details."
         << ::std::endl;
    }

    // don't delete threads and cthreads. If a (c)thread
    // has died, then it has already been deleted. Only (c)threads created
    // before simulation-start are in this table. Due to performance
    // reasons, we don't look up the dying thread in the process table
    // and remove it from there. simcontext::reset and ~simcontext invoke this
    // destructor. At present none of these routines are ever invoked. 
    // We can delete threads and cthreads here if a dying thread figured out
    // it was created before simulation-start and took itself off the 
    // process_table. 

#if 0
    sc_thread_handle  thread_next_p;    // Next thread to delete.
    sc_thread_handle  thread_now_p; // Thread now deleting.

    for( thread_now_p=m_thread_q; thread_now_p; thread_now_p=thread_next_p )
    {
    thread_next_p = thread_now_p->next_exist();
    delete thread_now_p;
    }
#endif // 0
}

inline
sc_method_handle 
sc_process_table::method_q_head()
{
    return m_method_q;
}

inline
void
sc_process_table::push_front( sc_method_handle handle_ )
{
    handle_->set_next_exist(m_method_q);
    m_method_q = handle_;
}

inline
void
sc_process_table::push_front( sc_thread_handle handle_ )
{
    handle_->set_next_exist(m_thread_q);
    m_thread_q = handle_;
}

sc_method_handle
sc_process_table::remove( sc_method_handle handle_ )
{
    sc_method_handle now_p; // Entry now examining.
    sc_method_handle prior_p;   // Entry prior to one now examining.

    prior_p = 0;
    for ( now_p = m_method_q; now_p; now_p = now_p->next_exist() )
    {
    if ( now_p == handle_ )
    {
        if ( prior_p )
        prior_p->set_next_exist( now_p->next_exist() );
        else
        m_method_q = now_p->next_exist();
        return handle_;
    }
    }
    return 0;
}

sc_thread_handle
sc_process_table::remove( sc_thread_handle handle_ )
{
    sc_thread_handle now_p; // Entry now examining.
    sc_thread_handle prior_p;   // Entry prior to one now examining.

    prior_p = 0;
    for ( now_p = m_thread_q; now_p; now_p = now_p->next_exist() )
    {
    if ( now_p == handle_ )
    {
        if ( prior_p )
        prior_p->set_next_exist( now_p->next_exist() );
        else
        m_thread_q = now_p->next_exist();
        return handle_;
    }
    }
    return 0;
}

inline
sc_thread_handle 
sc_process_table::thread_q_head()
{
    return m_thread_q;
}

int
sc_notify_time_compare( const void* p1, const void* p2 )
{
    const sc_event_timed* et1 = static_cast<const sc_event_timed*>( p1 );
    const sc_event_timed* et2 = static_cast<const sc_event_timed*>( p2 );

    const sc_time& t1 = et1->notify_time();
    const sc_time& t2 = et2->notify_time();
    
    if( t1 < t2 ) {
    return 1;
    } else if( t1 > t2 ) {
    return -1;
    } else {
    return 0;
    }
}


// +============================================================================
// | CLASS sc_invoke_method - class to invoke sc_method's to support 
// |                          sc_simcontext::preempt_with().
// +============================================================================
SC_MODULE(sc_invoke_method)
{
    // 08/20/2015 GL TODO: clean up the codes in the future

#if 0
    SC_CTOR(sc_invoke_method)
    {
      // remove from object hierarchy
      detach();
    }

    virtual ~sc_invoke_method()
    {
    m_invokers.resize(0);
    }

    // Method to call to execute a method's semantics. 
    
    void invoke_method( sc_method_handle method_h )
    {
    sc_process_handle invoker_h;  // handle for invocation thread to use.

        // number of invocation threads available.
    std::vector<sc_process_handle>::size_type invokers_n;

    m_method = method_h;

    // There is not an invocation thread to use, so allocate one.

    invokers_n = m_invokers.size();
    if ( invokers_n == 0 )
    {
        sc_spawn_options options;
        options.dont_initialize();
        options.set_stack_size(0x100000);
        options.set_sensitivity(&m_dummy);
        invoker_h = sc_spawn(sc_bind(&sc_invoke_method::invoker,this), 
                 sc_gen_unique_name("invoker"), &options);
        ((sc_process_b*)invoker_h)->detach();
    }

    // There is an invocation thread to use, use the last one on the list.

    else
    {
        invoker_h = m_invokers[invokers_n-1];
        m_invokers.pop_back();
    }

    // Fire off the invocation thread to invoke the method's semantics,
    // When it blocks put it onto the list of invocation threads that
    // are available.

        sc_get_curr_simcontext()->preempt_with( (sc_thread_handle)invoker_h );
    DEBUG_MSG( DEBUG_NAME, m_method, "back from preemption" ); 
    m_invokers.push_back(invoker_h);
    }

    // Thread to call method from:

    void invoker()
    {
    sc_simcontext* csc_p = sc_get_curr_simcontext();
    sc_process_b*  me = sc_get_current_process_b();

    DEBUG_MSG( DEBUG_NAME, me, "invoker initialization" );
        for (;; )
        {
            DEBUG_MSG( DEBUG_NAME, m_method, "invoker executing method" );
            csc_p->set_curr_proc( (sc_process_b*)m_method );
            csc_p->get_active_invokers().push_back((sc_thread_handle)me);
            m_method->run_process();
            csc_p->set_curr_proc( me );
            csc_p->get_active_invokers().pop_back();
            DEBUG_MSG( DEBUG_NAME, m_method, "back from executing method" );
            wait();
            assert( 0 );
        }
    }

    sc_event                       m_dummy;    // dummy event to wait on.
    sc_method_handle               m_method;   // method to be invoked.
    std::vector<sc_process_handle> m_invokers; // list of invoking threads.
#endif
};

// ----------------------------------------------------------------------------
//  CLASS : sc_simcontext
//
//  The simulation context.
// ----------------------------------------------------------------------------

void
sc_simcontext::init()
{

    // ALLOCATE VARIOUS MANAGERS AND REGISTRIES:

    m_object_manager = new sc_object_manager;
    m_module_registry = new sc_module_registry( *this );
    m_port_registry = new sc_port_registry( *this );
    m_export_registry = new sc_export_registry( *this );
    m_prim_channel_registry = new sc_prim_channel_registry( *this );
    m_phase_cb_registry = new sc_phase_callback_registry( *this );
    m_name_gen = new sc_name_gen;
    m_process_table = new sc_process_table;
    //m_current_writer = 0;

    m_curr_proc_queue.clear();
    //m_curr_proc_num = 0;


    // CHECK FOR ENVIRONMENT VARIABLES THAT MODIFY SIMULATOR EXECUTION:

    const char* write_check = std::getenv("SC_SIGNAL_WRITE_CHECK");
    m_write_check = ( (write_check==0) || strcmp(write_check,"DISABLE") ) ?
      true : false;


    // FINISH INITIALIZATIONS:

    reset_curr_proc();
    m_next_proc_id = -1;
    m_timed_events = new sc_ppq<sc_event_timed*>( 128, sc_notify_time_compare );
    m_something_to_trace = false;
    m_runnable = new sc_runnable;
    m_collectable = new sc_process_list;
    m_time_params = new sc_time_params;
    //m_curr_time = SC_ZERO_TIME; // 08/19/2015 GL: to be removed
    m_max_time = SC_ZERO_TIME;
    m_change_stamp = 0;
    //m_delta_count = 0; // 08/19/2015 GL: to be removed
    m_forced_stop = false;
    m_paused = false;
    m_ready_to_simulate = false;
    m_elaboration_done = false;
    m_execution_phase = phase_initialize;
    m_error = NULL;
    m_cor_pkg = 0;
    m_method_invoker_p = NULL;
    m_cor = 0;
    m_in_simulator_control = false;
    m_start_of_simulation_called = false;
    m_end_of_simulation_called = false;
    m_simulation_status = SC_ELABORATION;
    workload_table=new long[_OoO_Combined_Data_Conflict_Table_Size];
    visits=new long[_OoO_Combined_Data_Conflict_Table_Size];
    old_sys_time=time(0);

    print_mode_message=getenv("SYSC_PRINT_MODE_MESSAGE");
    sych_mode_defined=getenv("SYSC_SYNC_PAR_SIM");
    num_cpus=getenv("SYSC_PAR_SIM_CPUS");

    if(getenv(_SYSC_VERBOSITY_FLAG_1)||getenv(_SYSC_VERBOSITY_FLAG)) verbosity_flag_1 = true;
    else verbosity_flag_1 = false;
    if(getenv(_SYSC_VERBOSITY_FLAG_2)||getenv(_SYSC_VERBOSITY_FLAG)) verbosity_flag_2 = true;
    else verbosity_flag_2 = false;
    if(getenv(_SYSC_VERBOSITY_FLAG_3)||getenv(_SYSC_VERBOSITY_FLAG)) verbosity_flag_3 = true;
    else verbosity_flag_3 = false;
    if(getenv(_SYSC_VERBOSITY_FLAG_4)||getenv(_SYSC_VERBOSITY_FLAG)) verbosity_flag_4 = true;
    else verbosity_flag_4 = false;
    if(getenv(_SYSC_VERBOSITY_FLAG_5)||getenv(_SYSC_VERBOSITY_FLAG)) verbosity_flag_5 = true;
    else verbosity_flag_5 = false;
    if(getenv(_SYSC_VERBOSITY_FLAG_6)||getenv(_SYSC_VERBOSITY_FLAG)) verbosity_flag_6 = true;
    else verbosity_flag_6 = false;

    verbosity_flag = verbosity_flag_1||verbosity_flag_2||verbosity_flag_3||verbosity_flag_4||verbosity_flag_5;

    if(!getenv(_SYSC_PREDICTION_SWITCH_FLAG_ENV_VAR)) prediction_switch=true;
    else prediction_switch=false;
}

void
sc_simcontext::clean()
{
    delete m_object_manager;
    delete m_module_registry;
    delete m_port_registry;
    delete m_export_registry;
    delete m_prim_channel_registry;
    delete m_phase_cb_registry;
    delete m_name_gen;
    delete m_process_table;
    m_child_objects.resize(0);
    m_delta_events.resize(0);
    delete m_timed_events;
    for( int i = m_trace_files.size() - 1; i >= 0; -- i ) {
    delete m_trace_files[i];
    }
    m_trace_files.resize(0);
    delete m_runnable;
    delete m_collectable;
    delete m_time_params;
    delete m_cor_pkg;
    delete m_error;

    m_curr_proc_queue.clear();
    //m_curr_proc_num = 0;
}


sc_simcontext::sc_simcontext() :
    m_object_manager(0), m_module_registry(0), m_port_registry(0),
    m_export_registry(0), m_prim_channel_registry(0),
    m_phase_cb_registry(0), m_name_gen(0),
    m_process_table(0), m_curr_proc_queue(),
    m_write_check(false), m_next_proc_id(-1), m_child_events(),
    m_child_objects(), m_delta_events(), m_timed_events(0), m_trace_files(),
    m_something_to_trace(false), m_runnable(0), m_collectable(0), 
    m_time_params(), m_max_time(SC_ZERO_TIME), 
    //m_curr_time(SC_ZERO_TIME), // 08/19/2015 GL: to be removed
    m_change_stamp(0), m_forced_stop(false), m_paused(false),
    //m_delta_count(0), // 08/19/2015 GL: to be removed
    m_ready_to_simulate(false), m_elaboration_done(false),
    m_execution_phase(phase_initialize), m_error(0),
    m_in_simulator_control(false), m_end_of_simulation_called(false),
    m_simulation_status(SC_ELABORATION), m_start_of_simulation_called(false),
    m_cor_pkg(0), m_cor(0), m_one_delta_cycle(false), m_one_timed_cycle(false),
    m_finish_time(SC_ZERO_TIME),
    workload_table(0),visits(0),old_sys_time(0),m_synch_thread_queue(0),
    m_starvation_policy(SC_RUN_TO_TIME),last_seg_id(-1),
    m_invokers(0), //DM 05/27/2019
    event_notification_update(false),
    running_methods(0)
{
    init();
}

sc_simcontext::~sc_simcontext()
{
    clean();
}

/* 
    10:13 2017/3/10 ZC
    add a sc_process_b to wait queue
*/
void sc_simcontext::add_to_wait_queue(sc_process_b* process_h)
{
    m_waiting_proc_queue.push_back(process_h);  
}

/* 
    10:13 2017/3/10 ZC
    remove a sc_process_b from wait queue
*/
void sc_simcontext::remove_from_wait_queue(sc_process_b* process_h)
{
    m_waiting_proc_queue.remove(process_h); 
}

// +----------------------------------------------------------------------------
// |"sc_simcontext::active_object"
// | 
// | This method returns the currently active object with respect to 
// | additions to the hierarchy. It will be the top of the object hierarchy
// | stack if it is non-empty, or it will be the active process, or NULL 
// | stack if it is non-empty, or it will be the active process, or NULL 
// | if there is no active process.
// +----------------------------------------------------------------------------
sc_object*
sc_simcontext::active_object() 
{
    sc_object* result_p; // pointer to return.

    result_p = m_object_manager->hierarchy_curr();
    if ( !result_p )
        result_p = (sc_object*)get_curr_proc();
    return result_p;
}

#if 0
// +----------------------------------------------------------------------------
// |"sc_simcontext::mapper"
// | 
// | This method dispatches thread and method processes based on the READY 
// | queue and RUN queue, and it will suspend the scheduler when no more process
// | can be added to the RUN queue, or resume the scheduler if both the READY and
// | RUN queue are empty. 
// +----------------------------------------------------------------------------
void
sc_simcontext::mapper( sc_cor* cor_p )
{
    // running process handle: NULL if it is the root thread
    sc_process_b* m_process_b = get_curr_proc();

    while ( true )
    {
        // check for errors
        if( m_error ) return;

        // check for call(s) to sc_stop
        if( m_forced_stop ) {
            if ( stop_mode == SC_STOP_IMMEDIATE ) return;
        }

        if ( m_runnable->is_empty() )
        {
            if ( m_curr_proc_queue.size() == 0 )
            {
                if ( cor_p == m_cor )
                    return;
                else
                {
                    m_cor_pkg->go( m_cor );
                    return;
                }
            }
            else
            {
                if ( cor_p == m_cor )
                    m_cor_pkg->wait( cor_p );
                else return;
            }
        }
        else
        {
            while ( !m_runnable->is_empty() && 
                    m_curr_proc_queue.size()<_SYSC_NUM_SIM_CPUs )
            {
                // execute method processes

                m_runnable->toggle_methods();
                sc_method_handle method_h = pop_runnable_method();
                while( method_h != 0 ) {
                    if ( method_h->m_cor_p != NULL ) break;
                    method_h = pop_runnable_method();
                }

                if (method_h != 0) {
                    empty_eval_phase = false;
                    m_curr_proc_queue.push_back( (sc_process_b*)method_h );

                    // do not switch to myself!
                    if ( m_process_b != (sc_process_b*)method_h )
                        m_cor_pkg->go( method_h->m_cor_p );

                    continue;
                }

                // execute (c)thread processes

                m_runnable->toggle_threads();
                sc_thread_handle thread_h = pop_runnable_thread();
                while( thread_h != 0 ) {
                    if ( thread_h->m_cor_p != NULL ) break;
                    thread_h = pop_runnable_thread();
                }

                if( thread_h != 0 ) {
                    empty_eval_phase = false;
                    m_curr_proc_queue.push_back( (sc_process_b*)thread_h );

                    // do not switch to myself!
                    if ( m_process_b != (sc_process_b*)thread_h )
                        m_cor_pkg->go( thread_h->m_cor_p );
                }
            }

            if ( cor_p == m_cor )
            {
                m_cor_pkg->wait( cor_p ); // suspend the scheduler
            }
            else return;
        }
    }
}
#endif

#if 0
// +----------------------------------------------------------------------------
// |"sc_simcontext::schedule"
// | 
// | This method dispatches thread and method processes based on the READY queue
// | and RUN queue, and performs one or more delta cycles and timed cycles.
// | 
// | Each delta cycle consists of an evaluation phase, an update phase, and a 
// | notification phase. During the evaluation phase any processes that are  
// | ready to run are executed. After all the processes have been executed the  
// | update phase is entered. During the update phase the values of any signals  
// | that have changed are updated. After the updates have been performed the  
// | notification phase is entered. During that phase any notifications that need 
// | to occur because of of signal values changes are performed. This will result 
// | in the queueing of processes for execution that are sensitive to those 
// | notifications. At that point a delta cycle is complete, and the process is 
// | started again unless 'm_one_delta_cycle' is true.
// | 
// | If the READY queue is empty at the end of the notification phase, the 
// | current timed cycle finishes. At this point, the simulation time will 
// | advance to the time of next timed event. If the new simulation time does 
// | not exceed 'm_finish_time', timed notifications at the new time will be 
// | processed, which results in the queueing of processes for execution that 
// | are sensitive to those notifications. A new timed cycle is started again 
// | unless 'm_one_timed_cycle' is true. When the simulation time execeeds 
// | 'm_finish_time', it resumes the root thread and exits.
// |
// | Notes:
// |   (1) This code always run with an SC_EXIT_ON_STARVATION starvation policy,
// |       so the simulation time on return will be the minimum of the 
// |       simulation on entry plus the duration, and the maximum time of any 
// |       event present in the simulation. If the simulation policy is
// |       SC_RUN_TO_TIME starvation it is implemented by the caller of this 
// |       method, e.g., sc_start(), by artificially setting the simulation
// |       time forward after this method completes.
// |
// | (05/18/2015 GL)
// +----------------------------------------------------------------------------
void
sc_simcontext::schedule( sc_cor* cor_p )
{
    // assume we have acquired the kernel lock upon here
#ifdef SC_LOCK_CHECK
    assert( is_locked_and_owner() );
#endif /* SC_LOCK_CHECK */

    // running process handle: NULL if it is the root thread
    sc_process_b* m_process_b = get_curr_proc();
    sc_time t; // current simulaton time.

    do {
        while ( true )
        {

            // EVALUATE PHASE

            m_execution_phase = phase_evaluate;
            bool empty_eval_phase = true;

            if ( !m_runnable->is_empty() )
            {
                while ( !m_runnable->is_empty() && 
                        m_curr_proc_queue.size()<_SYSC_NUM_SIM_CPUs )
                {
                    // execute method processes

                    m_runnable->toggle_methods();
                    sc_method_handle method_h = pop_runnable_method();
                    while( method_h != 0 ) {
                        if ( method_h->m_cor_p != NULL ) break;
                        method_h = pop_runnable_method();
                    }

                    if ( method_h != 0 ) {
                        empty_eval_phase = false;
                        m_curr_proc_queue.push_back( (sc_process_b*)method_h );

                        // do not switch to myself!
                        if ( m_process_b != (sc_process_b*)method_h )
                            m_cor_pkg->go( method_h->m_cor_p );

                        continue;
                    }

                    // execute (c)thread processes

                    m_runnable->toggle_threads();
                    sc_thread_handle thread_h = pop_runnable_thread();
                    while( thread_h != 0 ) {
                        if ( thread_h->m_cor_p != NULL ) break;
                        thread_h = pop_runnable_thread();
                    }

                    if( thread_h != 0 ) {
                        empty_eval_phase = false;
                        m_curr_proc_queue.push_back( (sc_process_b*)thread_h );

                        // do not switch to myself!
                        if ( m_process_b != (sc_process_b*)thread_h )
                            m_cor_pkg->go( thread_h->m_cor_p );
                    }
                }

                if ( cor_p == m_cor )
                {
                    m_cor_pkg->wait( cor_p ); // suspend the root thread
                }

                reset_curr_proc();
                if ( m_error ) {
                    throw *m_error; // re-throw propagated error
                }
                return;
            }

            //if ( m_runnable->is_empty() == true )

            if ( m_curr_proc_queue.size() != 0 ) {
                assert( cor_p != m_cor );

                reset_curr_proc();
                if ( m_error ) {
                    throw *m_error; // re-throw propagated error
                }
                return;
            }

            //if ( m_runnable->is_empty() == true && 
            //     m_curr_proc_queue.size() == 0 )

            // check for errors
            if( m_error ) {
                break;
            }

            // check for call(s) to sc_stop
            if( m_forced_stop ) {
                if ( stop_mode == SC_STOP_IMMEDIATE ) {
                    break;
                }
            }


            // UPDATE PHASE
        //
            // The change stamp must be updated first so that event_occurred()
            // will work.

            m_execution_phase = phase_update;
            if ( !empty_eval_phase ) 
            {
//              SC_DO_PHASE_CALLBACK_(evaluation_done);
                m_change_stamp++;
                m_delta_count ++;
            }
            m_prim_channel_registry->perform_update();
            SC_DO_PHASE_CALLBACK_(update_done);
            m_execution_phase = phase_notify;

#if SC_SIMCONTEXT_TRACING_
            if( m_something_to_trace ) {
                trace_cycle( /* delta cycle? */ true );
            }
#endif

            // check for call(s) to sc_stop
            if( m_forced_stop ) {
                break;
            }

#ifdef DEBUG_SYSTEMC
            // check for possible infinite loops
            if( ++ num_deltas > SC_MAX_NUM_DELTA_CYCLES ) {
            ::std::cerr << "SystemC warning: "
                     << "the number of delta cycles exceeds the limit of "
                     << SC_MAX_NUM_DELTA_CYCLES
                     << ", defined in sc_constants.h.\n"
                     << "This is a possible sign of an infinite loop.\n"
                     << "Increase the limit if this warning is invalid.\n";
                break;
            }
#endif

            // NOTIFICATION PHASE:
            //
            // Process delta notifications which will queue processes for 
            // subsequent execution.

            int size = m_delta_events.size();
            if ( size != 0 )
            {
                sc_event** l_events = &m_delta_events[0];
                int i = size - 1;
                do {
                    l_events[i]->trigger();
                } while( -- i >= 0 );
                m_delta_events.resize(0);
            }

            // IF ONLY DOING ONE CYCLE, WE ARE DONE. OTHERWISE EXECUTE NEW 
            // CALLBACKS

            if ( m_one_delta_cycle ) {
                if ( cor_p != m_cor ) {
                    m_cor_pkg->go( m_cor ); // resume the root thread
                }

                reset_curr_proc();
                if ( m_error ) {
                    throw *m_error; // re-throw propagated error
                }
                return;
            }

            if( m_runnable->is_empty() ) {
                // no more runnable processes
                break;
            }

            // if sc_pause() was called we are done.

            if ( m_paused ) break;
        }

        // When this point is reached the processing of delta cycles is 
        // complete if the completion was because of an error throw the 
        // exception specified by '*m_error'.
        reset_curr_proc();
        if( m_error ) {
            throw *m_error; // re-throw propagated error
        }

        if ( m_one_timed_cycle ) {
            if ( cor_p != m_cor ) {
                m_cor_pkg->go( m_cor ); // resume the root thread
            }
            return;
        }

        if( m_error ) {
            m_in_simulator_control = false;
            if ( cor_p != m_cor ) {
                m_cor_pkg->go( m_cor ); // resume the root thread
            }
            return;
        }
#if SC_SIMCONTEXT_TRACING_
        if( m_something_to_trace ) {
            trace_cycle( false );
        }
#endif
        // check for call(s) to sc_stop() or sc_pause().
        if( m_forced_stop ) {
            do_sc_stop_action();
            if ( cor_p != m_cor ) {
                m_cor_pkg->go( m_cor ); // resume the root thread
            }
            return;
        }
        if( m_paused ) // return via explicit pause
        {
            m_execution_phase      = phase_evaluate;
            m_in_simulator_control = false;
            SC_DO_PHASE_CALLBACK_(simulation_paused);

            if ( cor_p != m_cor ) {
                m_cor_pkg->go( m_cor ); // resume the root thread
            }
            return;
        }

        // Advance Time

        t = m_curr_time; 

        do {
            // See note 1 above:

            if ( !next_time(t) || (t > m_finish_time ) )
            {
                if ( t > m_curr_time && t <= m_finish_time )
                {
                    SC_DO_PHASE_CALLBACK_(before_timestep);
                    m_curr_time = t;
                    m_change_stamp++;
                }

                m_execution_phase      = phase_evaluate;
                m_in_simulator_control = false;
                SC_DO_PHASE_CALLBACK_(simulation_paused);

                if ( cor_p != m_cor ) {
                    m_cor_pkg->go( m_cor ); // resume the root thread
                }
                return;
            }
            if ( t > m_curr_time ) 
            {
                SC_DO_PHASE_CALLBACK_(before_timestep);
                m_curr_time = t;
                m_change_stamp++;
            }

            // PROCESS TIMED NOTIFICATIONS AT THE CURRENT TIME

            do {
                sc_event_timed* et = m_timed_events->extract_top();
                sc_event* e = et->event();
                delete et;
                if( e != 0 ) {
                    e->trigger();
                }
            } while( m_timed_events->size() &&
                     m_timed_events->top()->notify_time() == t );
        } while( m_runnable->is_empty() );
    } while( t < m_finish_time );

    if ( cor_p != m_cor ) {
        m_cor_pkg->go( m_cor ); // resume the root thread
    }
    return;
}
#endif


// +----------------------------------------------------------------------------
// |"sc_simcontext::print_threads_states"
// | 
// | This function shows the states of threads
// | if verbosity_flag_1
// | show thread state(running, ready, waiting), timestamp, seg id, inst id
// | if verbosity_flag_3
// | show events the thread is waiting for
// +----------------------------------------------------------------------------
void sc_simcontext::print_threads_states(){
    if(verbosity_flag_1 || verbosity_flag_3)
    {
        printf("\nThread States\n");
        printf("----------------------------------------------\n");
        printf("             Thread Name             |");
    }
    if(verbosity_flag_1)
        printf("     State     |  Seg ID  |  Inst ID  |       Time       |");
    if(verbosity_flag_3)
        printf("    Waiting for Event(s)    ");
    if(verbosity_flag_1 || verbosity_flag_3)
        printf("\n");

    for ( std::list<sc_process_b*>::iterator 
        it = m_all_proc.begin();  
        it != m_all_proc.end(); 
        it++ )
    {
        if(verbosity_flag_1 || verbosity_flag_3)
        {   //thread name
            const char *Name = (*it)->name();
            unsigned Len = strlen(Name);
            if (Len > 35)
                printf("  ...%s|", Name + (Len-32));
            else
                printf("  %-35s|", Name);
        }
        if(verbosity_flag_1){
            //state
            int state = (*it)->m_process_state;
        if(state == 0)
                printf("    Running    |");
            else if(state == 1)
                printf("     Ready     |");
            else if(state == 2)
                printf("    Waiting    |");
            else if(state == 3)
                printf(" Wait-for-time |");
            else if(state == 4)
                printf("      Done     |");

            //segid instid
            printf("%6d    |", (*it)->get_segment_id());
            printf("%6d     |", (*it)->get_instance_id());

            //time
            std::string t = (*it)->get_timestamp().to_string();
            printf("%13s     |",t.c_str());
        }
        if(verbosity_flag_3)
        {
            printf("    %s ", (*it)->event_names().c_str());
        }
        if(verbosity_flag_1 || verbosity_flag_3)
            printf("\n");
    }
}

void sc_simcontext::print_events_states()
{
    if(verbosity_flag_2)
    {   
        
        printf("\nEvent States\n");
        for(std::vector<sc_event*>::iterator event_it=m_delta_events.begin();
            event_it!=m_delta_events.end();
            event_it++)
        {
            printf("----------------------------------------------\n");
            printf("event name        : %s\n" ,(*event_it)->name());
            printf("notification time : ");
            bool flag = false;
            for(std::set<sc_timestamp>::iterator 
                it = (*event_it)->m_notify_timestamp_set.begin();
                it != (*event_it)->m_notify_timestamp_set.end();
                ++ it)
            {
                std::string t = it->to_string();
                if(!flag)
                {
                    flag = true;
                    printf("%s ",t.c_str());
                }
                else
                    printf("| %s  ",t.c_str());
                
            }
            printf("\n");

            flag = false;
            printf("waiting threads   : ");
            for(std::vector<sc_thread_handle>::iterator
                it = (*event_it)->m_threads_dynamic.begin();
                it != (*event_it)->m_threads_dynamic.end();
                ++ it)
            {
                std::string name = (*it)->name();
                if(!flag)
                {
                    flag = true;
                    printf("%s ",name.c_str());
                }
                else
                    printf("| %s  ",name.c_str());
            }
            printf("\n");
        }
        if(m_delta_events.empty())
        {
            printf("----------------------------------------------\n");
            printf("No events in m_delta_events\n");
        }
    }
}

// +----------------------------------------------------------------------------
// |"sc_simcontext::oooschedule"
// | 
// | This method dispatches thread and method processes in an out-of-order
// | mannder, based on the READY queue and RUN queue. Each thread or method 
// | process has its local time stamp, which may be different from each other.
// |
// | At the beginning of the function, it processes all notified events and
// | timed notification. Then, it issues thread and method processes if they
// | have no conflicts with other processes having an earlier time. This
// | function resumes the root thread when both the READY queue and RUN queue
// | are empty.
// |
// | TODO: 'm_one_delta_cycle' 'm_one_timed_cycle' 'm_finish_time'
// |
// | (08/12/2015 GL)
// +----------------------------------------------------------------------------
void
sc_simcontext::oooschedule( sc_cor* cor_p )
{  
    static cycles_t ooo_total_cycles = 0;
    cycles_t ooo_start_cycles = currentcycles();

    //if verbosity flag is turned on, show debugging information
    if(verbosity_flag){
        if(get_curr_proc())
            std::cout << "\n\nEntering oooschedule(), by " << get_curr_proc()->name() 
                << std::endl;
        else std::cout << "\n\nEntering oooschedule(), by main() thread" << std::endl;
        print_threads_states();
        print_events_states();
    }


    // assume we have acquired the kernel lock upon here
    //bool show_log = print_verbose_message; 
#ifdef SC_LOCK_CHECK
    assert( is_locked_and_owner() );
#endif /* SC_LOCK_CHECK */
    // 12/22/2016 GL: advance_time is only used in the synchronized parallel mode
    sc_time advance_time;

    // running process handle: NULL if it is the root thread
    sc_process_b* m_process_b = get_curr_proc();

    event_notification_update = false; //DM 06/23/2019 for notify() that happen while simulation isn't running

    // 07/03/2016 GL: only the last process in the running queue can perfrom
    // channel updates, and delta notifications
    if ( m_curr_proc_queue.size() == 0 && m_runnable->is_empty() ) {

        // 06/16/2016 GL: only update primitive channels in the synchronized mode
        // Perform update functions in primitive channels
        if ( _SYSC_SYNC_PAR_SIM == true )
        {
            m_prim_channel_registry->perform_update();
        }
    }

    // Process timed notifications
    // for now, it is mainly for wait(time)
    while( m_timed_events->size() )
    {
        sc_event_timed* et = m_timed_events->extract_top();
        sc_event* e = et->event();
        delete et;
        if( e != 0 ) {
            e->trigger();
        }
    }

    //needed legacy code
    int initial_has_running_thread_flag; //DM 4/10/2018
    sc_timestamp time_earliest_running_ready_threads(-1,-1);
    for ( std::list<sc_process_b*>::iterator it = m_all_proc.begin();  
        it != m_all_proc.end(); 
        it++ )
    {
        //running and ready
        if((*it)->m_process_state==0 
            || (*it)->m_process_state==1
            || (*it)->m_process_state==3
            || (*it)->m_process_state==32) //32 is for synchronous mode
        {
            initial_has_running_thread_flag = 1; //DM's variable
            time_earliest_running_ready_threads
                = (*it)->get_timestamp() < time_earliest_running_ready_threads
                ? (*it)->get_timestamp() : time_earliest_running_ready_threads;
        }
    }

    static cycles_t event_total_cycles = 0;
    cycles_t event_start_cycles = currentcycles();
    //Event delivery
    while(check_and_deliver_events());
    cycles_t event_end_cycles = currentcycles();
    cycles_t event_curr_cycles = event_end_cycles - event_start_cycles;
    event_total_cycles += event_curr_cycles;

    /*****************SYNCHRONOUS PDES*****************/
    
    if (  _SYSC_SYNC_PAR_SIM==true ){
        sc_timestamp oldest_synch_thread_time;
    
        int has_synch_thread_flag=0;
        
        for ( std::list<sc_process_b*>::iterator it = m_synch_thread_queue.begin(); 
                it != m_synch_thread_queue.end(); it++ )
        {
            if(has_synch_thread_flag==0){
                has_synch_thread_flag=1;
                oldest_synch_thread_time=(*it)->get_timestamp();
            }
            else
            {
                if((*it)->get_timestamp()<oldest_synch_thread_time){
                    oldest_synch_thread_time=(*it)->get_timestamp();
                }
            }
        }

        //push ALL earliest synch threads to ready queue
        if(has_synch_thread_flag==1 && 
            m_runnable->is_empty() && 
            m_curr_proc_queue.size()==0)
        {
            for ( std::list<sc_process_b*>::iterator 
                it = m_synch_thread_queue.begin(); 
                it != m_synch_thread_queue.end();
                )
            {
                if((*it)->get_timestamp() == oldest_synch_thread_time){
                    if(dynamic_cast<sc_thread_process*>(*it) != NULL) { //DM 05/27/2019 support both methods and threads
                        push_runnable_thread( dynamic_cast<sc_thread_process*>(*it) );
                    }
                    else {
                        push_runnable_method( dynamic_cast<sc_method_process*>(*it) );
                    }
                    it=m_synch_thread_queue.erase(it);
                }
                else
                {
                    it++;
                }
            }
        }
        
        //DM 4/10/2018 final trace_cycle call for synchPDES, already in synchPDES region
        if(m_runnable->is_empty() && 
            m_synch_thread_queue.size()==0 &&
            initial_has_running_thread_flag==0) 
        {
            if(time_earliest_running_ready_threads.get_infinite())
            {
                oldest_untraced_time = m_oldest_time;
            }else
            {
                if(time_earliest_running_ready_threads.get_time_count() > oldest_untraced_time) 
                {
                    oldest_untraced_time = time_earliest_running_ready_threads.get_time_count();
                }
            }
            trace_cycle(false);
        } 
        //endcode tracing           
    }

    /*********************************************************************/
    if(verbosity_flag_4)
    {
        std::cout << "Dispatching threads" << std::endl;
        if (m_runnable->is_empty())
        {
            printf("----------------------------------------------\n");
            std::cout << "No Ready threads" << std::endl;
        }
    }

    if ( !m_runnable->is_empty() )
    {

        // 08/17/2015 GL: containers that keep processes with conflicts
        std::list<sc_method_handle> conflict_methods;
        std::list<sc_thread_handle> conflict_threads;
    
        while ( !m_runnable->is_empty()
                    && m_curr_proc_queue.size()<_SYSC_NUM_SIM_CPUs )
        {
            // execute method processes
            m_runnable->toggle_methods();
            sc_method_handle method_h = pop_runnable_method();
            while( method_h != 0 ) {
                if ( method_h->m_cor_p != NULL ) break;
                    method_h = pop_runnable_method();
            }

            if ( method_h != 0 ) 
            {
                // has no conflicts
                if ( has_no_conflicts( (sc_process_b*)method_h, conflict_methods, conflict_threads ) )
                {
                    
                    m_curr_proc_queue.push_back( (sc_process_b*)method_h );
                    if( running_invokers.count(method_to_invoker_map[ (sc_process_b*)method_h ]) == 0 ) 
                    {
                        method_h->m_process_state=0;
                        method_to_invoker_map[ (sc_process_b*)method_h ]->method_queue.push_back((sc_process_b*)method_h);
                        if ( ready_invokers.count(method_to_invoker_map[ (sc_process_b*)method_h ]) == 0 ) 
                        {
                            ready_invokers.insert(method_to_invoker_map[ (sc_process_b*)method_h ]);
                        }
                    }
                    else 
                    {
                        conflict_methods.push_back( method_h );//DM 05/20/2019 for now, consider as a method with conflicts
                        m_curr_proc_queue.pop_back();
                    }
                }
                else
                {
                    conflict_methods.push_back( method_h );
                }
                //continue; DM 07/24/2019
            }

            // execute (c)thread processes

            m_runnable->toggle_threads();
            sc_thread_handle thread_h = pop_runnable_thread();
            while( thread_h != 0 ) {
                if ( thread_h->m_cor_p != NULL ) break;
                    thread_h = pop_runnable_thread();
            }

            if( thread_h != 0 ) {

                if((thread_h->get_timestamp()) <= m_simulation_duration)
		{
		    m_simulation_time = std::max(m_simulation_time, thread_h->get_timestamp());
                }

                //this if() is implemented for sc_start(time)
                //check if it exceeds the max duration
                //if true 
                //push to paused queue
                //( then on another sc_start, push them all back to ready )
		// RD: there could be threads at times later than m_simulation_duration
		// RD: those are the ones that we put into the PAUSED state
                if((thread_h->get_timestamp()) >= m_simulation_duration)
		{
                    thread_h->m_process_state = 5;
                    m_paused_processes.push_back( (sc_process_b*)thread_h );
                    continue;
                }

                // has no conflicts
                if(verbosity_flag_4){
                    printf("----------------------------------------------\n");
                    std::cout << "checking conflicts for " << thread_h->name() << "\n" << std::endl;
                }

                if ( has_no_conflicts( (sc_process_b*)thread_h, conflict_methods, conflict_threads)) 
                {
                    //code for trace_cycle 4/10/2018 DM
                    if (  _SYSC_SYNC_PAR_SIM==true )
                    {
                        if( ((sc_process_b*) thread_h)->get_timestamp().get_time_count() > oldest_untraced_time) 
                        {
                            trace_cycle(false);
                            oldest_untraced_time = ((sc_process_b*) thread_h)->get_timestamp().get_time_count();
                        }
                    }
                    //end
                    m_curr_proc_queue.push_back( (sc_process_b*)thread_h );
                    thread_h->m_process_state=0;
                    // do not switch to myself!
                    if ( m_process_b != (sc_process_b*)thread_h )
                    {
                        thread_h->m_process_state=0; //10:44 2017/3/10 ZC
                        if(verbosity_flag_4) 
                            std::cout << thread_h->name() << " is issued to run\n" << std::endl;
                        m_cor_pkg->go( thread_h->m_cor_p );
                    }
                }
                else
                {   
                    if(verbosity_flag_4) 
                        std::cout << thread_h->name() << " cannot run and put back to ready queue\n" << std::endl;
                    conflict_threads.push_back( thread_h );
                }
            }
        }

        //DM 05/16/2019
        for(std::set<Invoker*>::iterator invok_iter = ready_invokers.begin();
            invok_iter != ready_invokers.end(); invok_iter++) {
            running_invokers.insert(*invok_iter);
            if ( m_process_b != (sc_process_b*) (*invok_iter)->proc_handle ){
                            //thread_h->m_process_state=0; //10:44 2017/3/10 ZC
                                    m_cor_pkg->go( ((*invok_iter)->proc_handle)->m_cor_p );
                }
            ready_invokers.erase(invok_iter);
        }

        if(verbosity_flag_4)
            printf("----------------------------------------------\n");

        // 08/17/2015 GL: move all the methods with conflicts back to the ready
        //                queue
        while ( !conflict_methods.empty() )
        {
            push_runnable_method_front( conflict_methods.back() );
            conflict_methods.pop_back();
        }

        // 08/17/2015 GL: move all the threads with conflicts back to the ready
        //                queue
        while ( !conflict_threads.empty() )
        {
            push_runnable_thread_front( conflict_threads.back() );
            conflict_threads.pop_back();
        } 

        if ( cor_p == m_cor && m_curr_proc_queue.size() != 0) //DM 9/25/2018
        {
            m_cor_pkg->wait( cor_p ); // suspend the root thread
        }
 
        
        //return;
    }

    //remove out of date event notifications
    clean_up_old_event_notifications();
    
    cycles_t ooo_stop_cycles = currentcycles();
    cycles_t ooo_curr_cycles = ooo_stop_cycles - ooo_start_cycles;
    ooo_total_cycles += ooo_curr_cycles;

    if(get_curr_proc() == NULL && verbosity_flag_5)
    {
        std::cout << "event delivery runs for total of " 
            << event_total_cycles << " cycles" << std::endl;
        std::cout << "oooschedule runs for total of " 
            << ooo_total_cycles - ooo_curr_cycles << " cycles" << std::endl;   
        std::cout << "simulation runs for total of " 
            << ooo_curr_cycles << " cycles" << std::endl;
        // std::cout << "oooschedule accounts for " 
        //     << ((double)(ooo_total_cycles - ooo_curr_cycles))/ooo_curr_cycles*100 
        //     << "% of total run-time" << std::endl;
    }
    
    if ( m_curr_proc_queue.size() != 0 ) {
        return;
    }


    //when no running or ready thread, resume the sc_main
    if ( cor_p != m_cor ) {
        m_cor_pkg->go( m_cor ); // resume the root thread
    } 
    else { //DM 9/25/2018
        m_simulation_status = SC_PAUSED; 
    }
}

//4/10/2018 DM extra functions for synchPDES tracing
const sc_time&
sc_simcontext::get_oldest_untraced_time() {
    return oldest_untraced_time;
}

const sc_time&
get_current_trace_time(){
    return sc_get_curr_simcontext()->get_oldest_untraced_time();
}
//end extra functions for tracing

//start: helper function for event delivery
void sc_simcontext::predict_wakeup_time_by_running_ready_threads(std::unordered_map<sc_process_b*, sc_timestamp>& wkup_t_prd_run_rdy)
{
    int _OoO_Prediction_Event_Notification_Table_Size=sqrt(_OoO_Combined_Data_Conflict_Table_Size);
    //now we fill wkup_t_prd_run_rdy
    std::list<sc_method_handle> runnable_methods_copy;
    std::list<sc_thread_handle> runnable_threads_copy;

    //DM's code for sc_methods I guess
    while ( !m_runnable->is_empty()  )
    {
        m_runnable->toggle_methods();
        sc_method_handle method_it2 = pop_runnable_method();
        while( method_it2 != 0 ) {
            if ( method_it2->m_cor_p != NULL ) break;
            method_it2 = pop_runnable_method();
        }

        if ( method_it2 != 0 ) {
            runnable_methods_copy.push_back(method_it2);
        }

        m_runnable->toggle_threads();
        sc_thread_handle thread_it2 = pop_runnable_thread();
        while( thread_it2 != 0 ) {
            if ( thread_it2->m_cor_p != NULL ) break;
            thread_it2 = pop_runnable_thread();
        }

            if( thread_it2 != 0 ) {
            runnable_threads_copy.push_back(thread_it2);
        }
    }       
    //end

    for ( std::list<sc_process_b*>::iterator 
        it1 = m_waiting_proc_queue.begin();  
        it1 != m_waiting_proc_queue.end(); 
        it1 ++ )
    {
    
        (*it1)->possible_wakeup_time = sc_timestamp(-1,-1); //reset possible_wakeup_time, 04082019
        if(wkup_t_prd_run_rdy.count(*it1) == 0)
            wkup_t_prd_run_rdy[*it1] = sc_timestamp(-1,-1);

        int it1_seg = (*it1)->get_segment_id();
        int it1_inst = (*it1)->get_instance_id();
        if(it1_seg == -1 || it1_seg == -2) continue;
        //check when is the proper time for it1's event to wake it1 up

        //for each waiting thread, check the predicted wakeup time by the
        //runnings and readys
        for ( std::list<sc_process_b*>::iterator 
            it2 = m_curr_proc_queue.begin();  
            it2 != m_curr_proc_queue.end(); 
            it2++ )
        {
            //initial_has_running_thread_flag = 1;
            int it2_seg = (*it2)->get_segment_id();
            int it2_inst = (*it2)->get_instance_id();
            if(it2_seg == -1 || it2_seg == -2) continue;
            //do the prediction

            int ETP_id1 = event_prediction_table_index_lookup( it1_seg, it1_inst );
            int ETP_id2 = event_prediction_table_index_lookup( it2_seg, it2_inst );
            long long pred_t = _OoO_Prediction_Event_Notification_Table_Time_Units[
                _OoO_Prediction_Event_Notification_Table_Size*ETP_id2+ETP_id1];
            long long pred_d = _OoO_Prediction_Event_Notification_Table_Delta[
                _OoO_Prediction_Event_Notification_Table_Size*ETP_id2+ETP_id1];

            double res = sc_dt::uint64_to_double( m_time_params->time_resolution);
            if( pred_t!=-1 && pred_d!=-1 )
            {
                sc_timestamp it2_t = (*it2)->get_timestamp();
                sc_timestamp pred_it1_t = 
                    it2_t + sc_timestamp( pred_t/res,pred_d);
                if(pred_it1_t < wkup_t_prd_run_rdy[*it1])
                    wkup_t_prd_run_rdy[*it1] = pred_it1_t;
            }

        }
        for ( std::list<sc_method_handle>::iterator 
            it2 = runnable_methods_copy.begin();  
            it2 != runnable_methods_copy.end(); 
            it2++ )
        {

            //initial_has_running_thread_flag = 1;
            int it2_seg = (*it2)->get_segment_id();
            int it2_inst = (*it2)->get_instance_id();
            if(it2_seg == -1 || it2_seg == -2) continue;
            //do the prediction

            int ETP_id1 = event_prediction_table_index_lookup( it1_seg, it1_inst );
            int ETP_id2 = event_prediction_table_index_lookup( it2_seg, it2_inst );
            long long pred_t = _OoO_Prediction_Event_Notification_Table_Time_Units[
                _OoO_Prediction_Event_Notification_Table_Size*ETP_id2+ETP_id1];
            long long pred_d = _OoO_Prediction_Event_Notification_Table_Delta[
                _OoO_Prediction_Event_Notification_Table_Size*ETP_id2+ETP_id1];

            double res = sc_dt::uint64_to_double( m_time_params->time_resolution);
            if( pred_t!=-1 && pred_d!=-1 )
            {
                sc_timestamp it2_t = (*it2)->get_timestamp();
                sc_timestamp pred_it1_t = 
                    it2_t + sc_timestamp( pred_t/res,pred_d);
                if(pred_it1_t < wkup_t_prd_run_rdy[*it1])
                    wkup_t_prd_run_rdy[*it1] = pred_it1_t;
            }

        }
        for ( std::list<sc_thread_handle>::iterator 
            it2 = runnable_threads_copy.begin();  
            it2 != runnable_threads_copy.end(); 
            it2++ )
        {

            //initial_has_running_thread_flag = 1;
            int it2_seg = (*it2)->get_segment_id();
            int it2_inst = (*it2)->get_instance_id();
            if(it2_seg == -1 || it2_seg == -2) continue;
            //do the prediction

            int ETP_id1 = event_prediction_table_index_lookup( it1_seg, it1_inst );
            int ETP_id2 = event_prediction_table_index_lookup( it2_seg, it2_inst );
            long long pred_t = _OoO_Prediction_Event_Notification_Table_Time_Units[
                _OoO_Prediction_Event_Notification_Table_Size*ETP_id2+ETP_id1];
            long long pred_d = _OoO_Prediction_Event_Notification_Table_Delta[
                _OoO_Prediction_Event_Notification_Table_Size*ETP_id2+ETP_id1];

            double res = sc_dt::uint64_to_double( m_time_params->time_resolution);
            if( pred_t!=-1 && pred_d!=-1 )
            {
                sc_timestamp it2_t = (*it2)->get_timestamp();
                sc_timestamp pred_it1_t = 
                    it2_t + sc_timestamp( pred_t/res,pred_d);
                if(pred_it1_t < wkup_t_prd_run_rdy[*it1])
                    wkup_t_prd_run_rdy[*it1] = pred_it1_t;
            }

        }

    }
    while ( !runnable_methods_copy.empty() )
    {
        push_runnable_method_front( runnable_methods_copy.back() );
        runnable_methods_copy.pop_back();
    }
    while ( !runnable_threads_copy.empty() )
    {       
        push_runnable_thread_front( runnable_threads_copy.back() );
        runnable_threads_copy.pop_back();
    }
}

void sc_simcontext::predict_wakeup_time_by_events(std::unordered_map<sc_process_b*,
        std::map<sc_event*, sc_timestamp> >& wkup_t_evnt)
{
    for(std::vector<sc_event*>::iterator 
        event_it  = m_delta_events.begin();
        event_it != m_delta_events.end();
        ++ event_it)
    {   
        //iterate over all the registerred waiting threads on this event
        for(std::vector<sc_thread_handle>::iterator 
            it1  = (*event_it)->m_threads_dynamic.begin();
            it1 != (*event_it)->m_threads_dynamic.end();
            ++ it1)
        {
            sc_timestamp it1_t = (*it1)->get_timestamp();
            sc_timestamp event_t = (*event_it)->get_earliest_time_after_certain_time(it1_t);
            //TODO, we may need to consider list wait here
            //TODO done
            wkup_t_evnt[(sc_process_b*)(*it1)][*event_it] = event_t;
        }
        for(std::vector<sc_method_handle>::iterator 
            it1  = (*event_it)->m_methods_dynamic.begin();
            it1 != (*event_it)->m_methods_dynamic.end();
            ++ it1)
        {
            sc_timestamp it1_t = (*it1)->get_timestamp();
            sc_timestamp event_t = (*event_it)->get_earliest_time_after_certain_time(it1_t);
            //TODO, we may need to consider list wait here
            //TODO done
            wkup_t_evnt[(sc_process_b*)(*it1)][*event_it] = event_t;
        }

    }
}

void sc_simcontext::predict_wakeup_time_by_waiting_threads(
    std::unordered_map<sc_process_b*, sc_timestamp>& wkup_t_pred_and_evnt,
    std::unordered_map<sc_process_b*, std::map<sc_event*, sc_timestamp> >& wkup_t_evnt,
    std::unordered_map<sc_process_b*, sc_timestamp>& wkup_t_prd_run_rdy)
{

    int _OoO_Prediction_Event_Notification_Table_Size=sqrt(_OoO_Combined_Data_Conflict_Table_Size);
    
    sc_process_b* earliest_waiting_thread = NULL;
    sc_timestamp earliest_wakeup_time (-1,-1);

    //now, summerize the earliest time a waiting thread may wakeup
    //it is the min of evnt and pred
    for ( std::list<sc_process_b*>::iterator 
        it1 = m_waiting_proc_queue.begin();  
        it1 != m_waiting_proc_queue.end(); 
        it1 ++ )
    {

        wkup_t_pred_and_evnt[*it1] = sc_timestamp(-1,-1); 
        
        if(wkup_t_prd_run_rdy.count(*it1) && wkup_t_evnt.count(*it1))
        {
            wkup_t_pred_and_evnt[*it1] = wkup_t_prd_run_rdy[*it1];
            for(std::map<sc_event*, sc_timestamp>::iterator
                it2  = wkup_t_evnt[*it1].begin();
                it2 != wkup_t_evnt[*it1].end();
                ++it2)
            {
                if(it2->second < wkup_t_pred_and_evnt[*it1])
                    wkup_t_pred_and_evnt[*it1] = it2->second + sc_timestamp(0,1);
            }
        }
        else if(wkup_t_evnt.count(*it1))
        {
            for(std::map<sc_event*, sc_timestamp>::iterator
                it2  = wkup_t_evnt[*it1].begin();
                it2 != wkup_t_evnt[*it1].end();
                ++it2)
            {
                if(it2->second < wkup_t_pred_and_evnt[*it1])
                    wkup_t_pred_and_evnt[*it1] = it2->second + sc_timestamp(0,1);
            }
        }
        else if(wkup_t_prd_run_rdy.count(*it1))
        {
            wkup_t_pred_and_evnt[*it1] = wkup_t_prd_run_rdy[*it1];
        }

        //added for later use
        if(wkup_t_pred_and_evnt[*it1] < earliest_wakeup_time)
        {
            earliest_wakeup_time = wkup_t_pred_and_evnt[*it1];
            earliest_waiting_thread = *it1;
        }
            
    }
 
    //Stops early if a lot threads never wakes up
    std::set<sc_process_b*> visited;
    while(!earliest_wakeup_time.get_infinite() && earliest_waiting_thread)
    {
        //now we can use earliest_wakeup_time to update tau of other waiting threads
        //ZC reuses DM's code here

        //we start by getting the first waiting thread that is not visited
        sc_process_b* unvisited_th = earliest_waiting_thread;
        sc_timestamp unvisited_t = earliest_wakeup_time;
        sc_timestamp earlist_time_this_round (-1,-1);
        visited.insert(unvisited_th);

        int earliest_seg = unvisited_th->get_segment_id();
        int earliest_inst = unvisited_th->get_instance_id();
        if(earliest_seg == -1 || earliest_seg == -2) continue;

        int ETP_id1 = event_prediction_table_index_lookup( earliest_seg, earliest_inst );
        //update waiting thread time with prediction
        for(std::unordered_map<sc_process_b*, sc_timestamp>::iterator
            p_iter  = wkup_t_pred_and_evnt.begin();
            p_iter != wkup_t_pred_and_evnt.end();
            ++p_iter)
        {
            sc_process_b* th = p_iter->first;
            sc_timestamp& t = p_iter->second;

            int th_seg = th->get_segment_id();
            int th_inst = th->get_instance_id();
            if(th_seg == -1 || th_seg == -2) continue;

            int ETP_id2 = event_prediction_table_index_lookup( th_seg, th_inst );
            long long pred_t = _OoO_Prediction_Event_Notification_Table_Time_Units[
                _OoO_Prediction_Event_Notification_Table_Size*ETP_id1+ETP_id2];
            long long pred_d = _OoO_Prediction_Event_Notification_Table_Delta[
                _OoO_Prediction_Event_Notification_Table_Size*ETP_id1+ETP_id2];
            if( pred_t!=-1 && pred_d!=-1 )
            {
                double res = sc_dt::uint64_to_double( m_time_params->time_resolution);
                sc_timestamp pred_th_t = unvisited_t + sc_timestamp( pred_t/res, pred_d);
                if(pred_th_t < t) t = pred_th_t;
            }
            if(t < earlist_time_this_round && t >= earliest_wakeup_time)
            {
                if(visited.count(th) == 0) 
                {
                    earlist_time_this_round = t;
                    earliest_waiting_thread = th;
                }
            }
        }
        if(earlist_time_this_round != earliest_wakeup_time) visited.clear();
        earliest_wakeup_time = earlist_time_this_round;
    }

}

bool sc_simcontext::check_and_deliver_events()
{
   /*2019 version
    Event Delivery
    
    say we have a thread th1 with segid 1, and th2 with segid 2 in the wait queue,
    th3 with segid 3, t=3 and th4 with segid 4, t=4 in the running queue,
    th5 with segid 5, t=5 and th6 with segid 6, t=6 in the ready queue.


    earlist event that wakes up th1 is e1, t = 10
    earlist event that wakes up th2 is e2, t = 20

    in the 2018 version, we cannot wakeup th1 neither th2, because we cannot know that if th3, th4, th5 or th6 will notify event e1 or e2 at an earlier time.

    Now, I go through this again and think the strategy is too conservative. Actually, we have the predicted event notification table that points out the earliest time that a segment can notify another segment.

    Take th1 as an example. We first check if th3 can wake it up before time 10. We is able to know that segment 3 may wakes up 1 in 5 seconds (though may not be by event e1), then we know that th1 may wake up on time 8 which is before time 10, so we dont move th1 from wait to ready.

    on the other hand, for the example of th2. According to the predicted event notification table, we know that seg1, seg3, seg4, seg5 and seg6 will not by any chance to wake up seg2 before time t = 20. So, event e2 is the earlist one that wakeup th2, and we is able to move th2 from wait queue to ready queue. Note that since th1's earliest wakeup time is now 8 instead of 10, when we do the prediction, we need to treat segment 1's current time to be 8 but not 10.

    The detail of algorithm is as follows
    first, for any threads in the wait queue, update its wakeup time according to the predicted wakeup time computed with the ready and running threads
    second, update the wakeup time with the predicted wakeup time of other waiting threads.

    for the data conflict analysis, we still need to take waiting threads into consideration. For example, th1 may wake up by th3 at time t=3, and segment 1 has connflict with segment 4, then, th4 cannot goto running queue because of the conflict. 

    //ZC 2019/03/04
    Different from previous implementation, we no longer "deliver events". Instead, we check the waiting thread if it can be waken up by any chance.
    Previously, the idea is to find the earliest event, deliver it and remove it from the event list. This has a lot of limitations. First, the event time has to be before all the running/ready threads. Second, it only delivers the earliest event.

    Now what we do is different. We check all the waiting thread. 
    for a given thread, we check first the earlist waking up time t1 by the running/ready threads using the event prediction table. We then check when will the thread be waken up by an event. For example, e has time 10 and 20, the thread is at 15, then, the event delivery time t2 on 20. So, the earlist time for a thread to wake up at this moment is t = min(t1,t2). 
    At this stage, we have t for all the waiting threads. We also need to check if a waking up thread will predictingly wake up other waiting threads. So, we need to start from the thread of earlist t and keep updating t of other threads
    
**********************************************************************/
    if(verbosity_flag_4)
    {
        std::cout << "\nDeliver Event Notifications" << std::endl;
        printf("----------------------------------------------\n");
    }
    //std::cout << "_SYSC_NUM_SIM_CPUs = " << _SYSC_NUM_SIM_CPUs << std::endl;
    if(m_curr_proc_queue.size() >= _SYSC_NUM_SIM_CPUs) return false;
    //we first collect the information about the predicted wakeup time by
    //running and ready threads in wkup_t_prd_run_rdy
    //then we collect when is it waken up by event
    //in wkup_t_evnt
    std::unordered_map<sc_process_b*, sc_timestamp> wkup_t_prd_run_rdy;
    predict_wakeup_time_by_running_ready_threads(wkup_t_prd_run_rdy);
    //then decide the wake up time due to events
    //we need to iterate over all the events
    std::unordered_map<sc_process_b*,
        std::map<sc_event*, sc_timestamp> > wkup_t_evnt;
    predict_wakeup_time_by_events(wkup_t_evnt);

    //now, summerize the earliest time a waiting thread may wakeup
    //it is the min of evnt and pred
    std::unordered_map<sc_process_b*, sc_timestamp> wkup_t_pred_and_evnt;
    predict_wakeup_time_by_waiting_threads( wkup_t_pred_and_evnt,
                                            wkup_t_evnt,
                                            wkup_t_prd_run_rdy );
    //now we know the earliest time a thread can wake up given the wkup_t_pred_and_evnt table. The next thing is to check if the earlist time is smaller than the event wake up time for the thread. If yes, then it is not safe to move the thread from the wait queue. Otherwise, we can wakeup the thread and set its time to event wakeup time collected in wkup_t_evnt.
    //in one sentence, we need a function to trigger a specific event at a specific time for a specific waiting thread.
    //I follow the convention and implement the function: deliver_event_at_time(event, sc_timestamp) in sc_thread_process
    
    bool has_waking_up_thread = false;
    bool has_event_delivered = false;
    for(std::unordered_map<sc_process_b*, sc_timestamp>::iterator
        p_iter1  = wkup_t_pred_and_evnt.begin();
        p_iter1 != wkup_t_pred_and_evnt.end();
        ++p_iter1)
    {
        sc_process_b* th1 = p_iter1->first;
        //get the predicted wakeup time of th1
        sc_timestamp t1 = p_iter1->second;

        th1->possible_wakeup_time = t1;//set the possible_wakeup_time, used in predicted data conflict analysis, ZC 20190804
        //if th1 is already a waken up thread, then dont need try to wake it up again.
        if(th1->m_process_state!=2) continue;
        if(wkup_t_evnt.count(th1))
        {
            for(std::map<sc_event*, sc_timestamp>::iterator
                p_iter2  = wkup_t_evnt[th1].begin();
                p_iter2 != wkup_t_evnt[th1].end();
                ++p_iter2)
            {
                //get the event that wakes up th1
                sc_event* e = p_iter2->first;
                //get the time that e wakes up th1
                sc_timestamp t2 = p_iter2->second;

                //if t2 <= t1, meaning e can wake up th1 at t2
                if(t2 <= t1)
                {
                    has_event_delivered = true;
                    if(dynamic_cast<sc_thread_process*>(th1) != NULL){ 
                        bool flag = dynamic_cast<sc_thread_process*>(th1)->deliver_event_at_time(e, t2);
                        has_waking_up_thread |= flag;

                        if(verbosity_flag_4)
                        {
                            std::cout << "deliver event " << e->name() << " at timestamp "
                                << t2.to_string() << " to wake up thread " << th1->name() 
                                << std::endl;
                            if(flag) std::cout << "thread wakes up" << std::endl;
                            else std::cout << "thread does not wake up" << std::endl;
                        }
                    }
                    else 
                    {
                        dynamic_cast<sc_method_process*>(th1)->deliver_event_at_time(e, t2);
                    }
                }
            }
        }
    }
    if(verbosity_flag_4)
    {
        if(has_event_delivered) std::cout << std::endl;
        if(!has_waking_up_thread) std::cout << "Event Delivery Done. No thread waked up" << std::endl;
        else std::cout << "Event Delivery Done" << std::endl; 
        std::cout << "===================\n" << std::endl;
    }
    return (!has_waking_up_thread) && (has_event_delivered);
}

void sc_simcontext::clean_up_old_event_notifications()
{
    //for example,
    //an event notification is at time (1,0)
    //however, the earliest running/ready thread is at (2,0)
    //so it will not take effect any more and should be removed
    //to save space
    sc_timestamp time_earliest_all_threads(-1,-1);
    for ( std::list<sc_process_b*>::iterator it = m_all_proc.begin();  
        it != m_all_proc.end(); 
        it++ )
    {
        //running and ready and wait-for-time and wait-for-and-list-events
        if((*it)->m_process_state==0 
            || (*it)->m_process_state==1
            || (*it)->m_process_state==3
            || (*it)->m_process_state==2    
            || (*it)->m_process_state==32)  
        {
            if((*it)->m_process_state!=2)
                time_earliest_all_threads
                    = (*it)->get_timestamp() < time_earliest_all_threads
                    ? (*it)->get_timestamp() : time_earliest_all_threads;
            else    
            {
                //we need to also consider and events
                //for example, a thread is waiting for e1&e2, and
                //th is at time 1
                //e1 is already notified at 2
                //e2 is not notified yet.
                //
                //if we dont consider th, then e1 at 2 is removed
                //which is wrong
                if((*it)->m_event_list_p!=NULL && 
                    (*it)->m_event_list_p->and_list())
                {
                    time_earliest_all_threads
                    = (*it)->get_timestamp() < time_earliest_all_threads
                    ? (*it)->get_timestamp() : time_earliest_all_threads;
                }
            }
        }
    }

    std::vector<sc_event*> events_to_be_removed;
    for(std::vector<sc_event*>::iterator event_it=m_delta_events.begin();
        event_it!=m_delta_events.end();
        event_it++)
    {
        std::vector<sc_timestamp> times_to_be_removed;
        for(std::set<sc_timestamp>::iterator 
            it = (*event_it)->m_notify_timestamp_set.begin();
            it != (*event_it)->m_notify_timestamp_set.end();
            ++it)
        {
            if((*it) < time_earliest_all_threads) {
                times_to_be_removed.push_back(*it);
            }
        }
        for(std::vector<sc_timestamp>::iterator 
            it2 = times_to_be_removed.begin();
            it2 != times_to_be_removed.end();
            ++it2)
        {
            (*event_it) -> erase_notification_time(*it2);
        }
        if((*event_it)->m_notify_timestamp_set.empty())
                events_to_be_removed.push_back((*event_it));
    }

    for(std::vector<sc_event*>::iterator event_it=events_to_be_removed.begin();
        event_it!=events_to_be_removed.end();
        event_it++)
    {
        remove_delta_event((*event_it));
    }
}
//end: helper function for event delivery

// +----------------------------------------------------------------------------
// |"sc_simcontext::has_no_conflicts"
// | 
// | This method checks for potential conflicts with all concurrent threads and
// | methods in the RUN and READY queues with an earlier time than process_h.
// |
// | (08/17/2015 GL)
// |
// | Now this method also checks for potential conflicts with threads and 
// | methods in the RUN and READY queues with the same timestamp as process_h.
// |
// | (12/21/2016 GL)
// +----------------------------------------------------------------------------
bool
sc_simcontext::has_no_conflicts( sc_process_b* process_h,
        std::list<sc_method_handle> conflict_methods,
        std::list<sc_thread_handle> conflict_threads )
{
    // assume we have acquired the kernel lock upon here
#ifdef SC_LOCK_CHECK
    assert( is_locked_and_owner() );
#endif /* SC_LOCK_CHECK */
    
    sc_method_handle method_h;
    sc_thread_handle thread_h;
    bool earlier_process = false;

    // the local timestamp of the target process
    sc_timestamp ts = process_h->get_timestamp();

    // concurrent processes in the RUN and READY queues with an earlier time
    // 12/21/2016 GL: or the same timestamp
    std::list<sc_process_b*> concurrent_processes;
    for ( std::list<sc_method_handle>::iterator it = conflict_methods.begin();
          it != conflict_methods.end(); it++ )
    {
        if ( (*it)->get_timestamp() < ts ) {
            concurrent_processes.push_back( ( sc_process_b*)(*it) );
            earlier_process = true;
        } 
    }

    for ( std::list<sc_thread_handle>::iterator it = conflict_threads.begin();
          it != conflict_threads.end(); it++ )
    {
        if ( (*it)->get_timestamp() < ts ) {
            concurrent_processes.push_back( ( sc_process_b*)(*it) );
            earlier_process = true;
        } 
    }
    // push in all processes in the RUN queues with an earlier time
    // 12/21/2016 GL: or the same timestamp
    for ( std::list<sc_process_b*>::iterator it = m_curr_proc_queue.begin();
          it != m_curr_proc_queue.end(); it++ )
    {
        if((*it)->get_segment_id()==-2) continue;
         
        if ( (*it)->get_timestamp() < ts ) {
            concurrent_processes.push_back( *it );
            earlier_process = true;
        } 
        else if ( (*it)->get_timestamp() == ts ){
            if( !(dynamic_cast<sc_method_process*>(process_h)!=NULL 
            && dynamic_cast<sc_method_process*>(*it)!=NULL //DM 05/22/2019 adding invoker exception for sc_method
            && method_to_invoker_map[process_h] == method_to_invoker_map[(*it)]) ) 
            {
                concurrent_processes.push_back( *it );
            }   
        }
    }

    // push in all methods in the READY push queues with an earlier time
    // 12/21/2016 GL: or the same timestamp
    method_h = m_runnable->get_methods_push_first();
    while ( !m_runnable->is_methods_push_end( method_h ) )
    {
        if ( method_h->get_timestamp() < ts ) {
            concurrent_processes.push_back( (sc_process_b*)method_h );
            earlier_process = true;
        }
        method_h = method_h->next_runnable();
    }

    // push in all methods in the READY pop queues with an earlier time
    // 12/21/2016 GL: or the same timestamp
    method_h = m_runnable->get_methods_pop_first();
    while ( !m_runnable->is_methods_pop_end( method_h ) )
    {
        if ( method_h->get_timestamp() < ts ) {
            concurrent_processes.push_back( (sc_process_b*)method_h );
            earlier_process = true;
        }
        method_h = method_h->next_runnable();
    }
    
    // push in all threads in the READY push queues with an earlier time
    // 12/21/2016 GL: or the same timestamp
    thread_h = m_runnable->get_threads_push_first();
    while ( !m_runnable->is_threads_push_end( thread_h ) )
    {
        if ( thread_h->get_timestamp() < ts ) {
            concurrent_processes.push_back( (sc_process_b*)thread_h );
            earlier_process = true;
        }
        thread_h = thread_h->next_runnable();
    }

    // push in all threads in the READY pop queues with an earlier time
    // 12/21/2016 GL: or the same timestamp
    thread_h = m_runnable->get_threads_pop_first();
    while ( !m_runnable->is_threads_pop_end( thread_h ) )
    {
        if ( thread_h->get_timestamp() < ts ) {
            concurrent_processes.push_back( (sc_process_b*)thread_h );
            earlier_process = true;
        }
        thread_h = thread_h->next_runnable();
    }

    //ZC, 2018-12-25. Maybe wait threads also need to be analyzed
    //ZC, 2019
    if (prediction_switch)
    {
        for ( std::list<sc_process_b*>::iterator it = m_all_proc.begin();  it != m_all_proc.end(); it++ )
        {
            sc_timestamp ts_tmp=(*it)->possible_wakeup_time;
            if ( ts_tmp < ts && (*it)->m_process_state == 2) {
                concurrent_processes.push_back( (*it) );
            }
            
        }
    }

    // 06/16/2016 GL: if synchronized parallel simulation is enabled,
    //                and there exists a thread with an earlier time
    if ( _SYSC_SYNC_PAR_SIM == true && earlier_process == true ) {
        ::std::cerr << "There exists a thread running ahead"
                    << "in the synchronized parallel mode." << ::std::endl;
        exit( 1 );
    }        
    if(concurrent_processes.empty())
    {
        if(verbosity_flag_4)
            std::cout << "no thread has timestamp earlier than " << process_h->name() << std::endl;
    }
    while ( !concurrent_processes.empty() )
    {
        sc_process_b* concur_process_h = concurrent_processes.front();
        concurrent_processes.pop_front();
        
        if (prediction_switch){

            if ( conflict_between_with_prediction( process_h, concur_process_h ) ){
                return false;
            }
        }
        else{
            if ( conflict_between( process_h, concur_process_h ) ){
                return false;
            }
        }
    }
    return true;
}

int
sc_simcontext::combined_data_conflict_table_index_lookup( int seg_id, int inst_id )
{
    //new version: consecutive segment id 13:55 2017/3/10 ZC
    assert( seg_id >= -1 ); // Valid segment IDs are non-negative.
    assert( inst_id >= 0 ); // Valid instance IDs are non-negative.
    
    int i= inst_id*_OoO_Combined_Data_Conflict_Lookup_Table_Number_Segments+seg_id;
    
    return _OoO_Combined_Data_Conflict_Lookup_Table[i];
    
}


// 02/14/2017 ZC: use index segment id and instance id to get index id,
// this function is used in the conflict_between_two_segs() function

int
sc_simcontext::event_prediction_table_index_lookup( int seg_id, int inst_id )
{
    //new version: consecutive segment id 13:55 2017/3/10 ZC
    
    assert( seg_id >= -1 ); // Valid segment IDs are non-negative.
    assert( inst_id >= 0 ); // Valid instance IDs are non-negative.
    
    int i= inst_id*_OoO_Prediction_Event_Notification_Table_Number_Segments+seg_id;
    
    return _OoO_Prediction_Event_Notification_Lookup_Table[i];
    
}


// 02/14/2017 ZC: use index segment id and instance id to get index id,
// this function is used in the conflict_between_two_segs() function


int
sc_simcontext::prediction_time_advance_table_index_lookup( int seg_id )
{
    
    //old version: nonconsecutive segment id
    
    static bool first_time = true;
    static std::map<int, int> column_index_table;

    // // TODO: assert statements are optional in the future
    assert( seg_id >= -1 ); // Valid segment IDs are non-negative.

    if ( first_time )
    {
        for (unsigned int i = 0; i < _OoO_Prediction_Time_Advance_Table_Number_Segments; i++ )
            column_index_table[_OoO_Prediction_Time_Advance_Lookup_Table[i]] = i;

        first_time = false;
    }

    std::map<int, int>::iterator it;
    it = column_index_table.find( seg_id );

    if ( it != column_index_table.end() )
    {
        int result = it->second;
        assert( result >= 0 );
        return result;
    }
    else
    {
        ::std::cerr << "Time advance table index lookup: invalid segment ID.\n";
        assert(0);
    exit(0);
    }
    
    
    //new version: consecutive segment id 13:55 2017/3/10 ZC
    /*assert( seg_id >= 0 ); // Valid segment IDs are non-negative.
    
    int i = seg_id;
    
    return i;*/
    //return _OoO_Prediction_Time_Advance_Lookup_Table[i];
    
}

// 02/14/2017 ZC: this is used for recursion
// it first checks the predicted data conflict, which is also called directed conflict
// then checks for indrect hazard. 
bool
sc_simcontext::conflict_between_two_segs(
    int seg_id, //the segment under test
    int seg_id1, //the concurrent segment
    int inst_id,
    int inst_id1,
    sc_timestamp id_time,
    sc_timestamp id1_time
){

    if(seg_id < 0 || seg_id1 < 0) return false;
    if(seg_id == -2 || seg_id1 == -2) return false;
    if(inst_id < 0 || inst_id1 < 0) return false;
    
    int CDCT_id = combined_data_conflict_table_index_lookup( seg_id, inst_id );
    int CDCT_id1 = combined_data_conflict_table_index_lookup( seg_id1, inst_id1 );

    int N=sqrt(_OoO_Combined_Data_Conflict_Table_Size); //this should be changed. 
    int m = _OoO_Combined_Data_Conflict_Table[CDCT_id1 * N + CDCT_id] - 1;
    
    if( m == -1)
    {
        if(verbosity_flag_4){
            std::cout << "    No Conflict : thread1:{" << seg_id << "," << inst_id << "}" 
                << " is directly conflict free with " 
                << "thread2:{" << seg_id1 << "," << inst_id1 << "}" 
                << std::endl;
        }
    }
    if (m==0) // meaning that the two segments have direct conflict with each other
    {
        sc_timestamp new_ts; 
        
        new_ts = id1_time;
        
        if ( new_ts <= id_time ) {
            if(verbosity_flag_4){
                std::cout << "    Has Conflict : thread1:{" << seg_id << "," << inst_id 
                    << "}" 
                    << " at timestamp (" << id_time.to_string() 
                    << ") has direct data conflict with " 
                    << "thread2:{" << seg_id1 << "," << inst_id1 << "}" 
                    << " at timestamp (" << new_ts.to_string() << ")"
                    << std::endl;
            }
            return true;
        }
        else{ 
            if(verbosity_flag_4){
                std::cout << "    No Conflict : thread1:{" << seg_id << "," << inst_id << "}" 
                    << " at timestamp (" << id_time.to_string() 
                    << ") have direct data conflict with " 
                    << "thread2:{" << seg_id1 << "," << inst_id1 << "}" 
                    << " at timestamp (" << new_ts.to_string() << "), however the timing prevents the conflict"
                    << std::endl;
                }
            }
            return false;
    }
    
    if (m>0) {  //predicted data hazard
        sc_timestamp new_ts; 
        int PTAT_id1 = prediction_time_advance_table_index_lookup( seg_id1 );

        new_ts = id1_time + sc_timestamp( 
                        _OoO_Prediction_Time_Advance_Table_Time_Units[
                                    (_OoO_Prediction_Time_Advance_Table_Number_Steps+1)*PTAT_id1+m-1]/sc_dt::uint64_to_double( m_time_params->time_resolution ) , 
                        _OoO_Prediction_Time_Advance_Table_Delta[
                                    (_OoO_Prediction_Time_Advance_Table_Number_Steps+1)*PTAT_id1+m-1]
                        );
        
        if ( new_ts <= id_time ) {
            if(verbosity_flag_4)
            {
                std::cout << "    Has Conflict by Prediction : thread1:{" << seg_id << "," 
                    << inst_id 
                    << "}" << " at timestamp (" << id_time.to_string()  << ")"
                    << " is predicted to have data conflict with " 
                    << "thread2:{" << seg_id1 << "," << inst_id1 << "}" 
                    << " 's future timestamp (" << new_ts.to_string()  << ")"
                    << std::endl;
            }
            return true;
        }
        else
        {
            if(verbosity_flag_4)
            {
                std::cout << "    No Conflict by Prediction : thread1{" << seg_id << "," 
                    << inst_id 
                    << "}" << " at timestamp (" << id_time.to_string()  << ")"
                    << " is predicted to have data conflict with " 
                    << "thread2:{" << seg_id1 << "," << inst_id1 << "}" 
                    << " 's future timestamp (" << new_ts.to_string()  << "), however the timing prevents the conflict"
                    << std::endl;
            }
        }
    }
    return false;
}

// 2/15/2017 ZC: new conflict detection
// process_h1 is under test
bool
sc_simcontext::conflict_between_with_prediction( sc_process_b* process_h1, sc_process_b* process_h2)
{
    int seg_id1 = process_h1->get_segment_id();
    int seg_id2 = process_h2->get_segment_id();
    if(seg_id1 == -1 || seg_id2 == -1) return false;
    if(seg_id1 == -2 || seg_id2 == -2) return false;
    int inst_id1 = process_h1->get_instance_id();
    int inst_id2 = process_h2->get_instance_id();
    if(inst_id1 == -2 || inst_id2 == -2) return false;

    sc_timestamp ts1, ts2;
    ts1 = process_h1->get_timestamp();
    if(process_h2->m_process_state != 2) ts2 = process_h2->get_timestamp();
    else ts2 = process_h2->possible_wakeup_time; //ZC 20190804
    if(verbosity_flag_4)
        std::cout << "checking thread1: "
            << process_h1->name() << " against thread2: "
            << process_h2->name() 
            << std::endl;
    bool flag=conflict_between_two_segs(seg_id1,seg_id2,inst_id1,inst_id2,ts1,ts2);
    
    if(verbosity_flag_4){
        std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
    }

    return flag;
}

// +----------------------------------------------------------------------------
// |"sc_simcontext::conflict_between"
// | 
// | This methods checks for the data and timing hazards between two processes.
// |
// | (08/17/2015 GL)
// +----------------------------------------------------------------------------

bool
sc_simcontext::conflict_between( sc_process_b* process_h1,
                                 sc_process_b* process_h2 )
{
    
    //printf("from conflict between\n");
    int seg_id1 = process_h1->get_segment_id();
    int seg_id2 = process_h2->get_segment_id();
    if(seg_id1 == -1 || seg_id2 == -1) return false;
    if(seg_id1 == -2 || seg_id2 == -2) return false;
    int inst_id1 = process_h1->get_instance_id();
    int inst_id2 = process_h2->get_instance_id();
    /*std::cout << "process_h1 = " << process_h1->name() << std::endl;
    std::cout << "process_h2 = " << process_h2->name() << std::endl;

    std::cout << "seg_id1 = " << seg_id1 << std::endl;
    std::cout << "seg_id2 = " << seg_id2 << std::endl;
    std::cout << "inst_id1 = " << inst_id1 << std::endl;
    std::cout << "inst_id2 = " << inst_id2 << std::endl;*/
    if(inst_id1 < 0 || inst_id2 < 0) return false;
    unsigned int conflict_table_index1;
    unsigned int conflict_table_index2;
    conflict_table_index1 = conflict_table_index_lookup( seg_id1, inst_id1 );
    conflict_table_index2 = conflict_table_index_lookup( seg_id2, inst_id2 );

   /* std::cout << "conflict_table_index1 = " << conflict_table_index1 << std::endl;
    std::cout << "conflict_table_index2 = " << conflict_table_index2 << std::endl;*/
    // TODO: optional in the future
    assert( conflict_table_index1 < _OoO_Data_Conflict_Table_Size );
    assert( conflict_table_index2 < _OoO_Data_Conflict_Table_Size );

    // check data hazards
    if ( _OoO_Data_Conflict_Table[conflict_table_index1 * 
             _OoO_Data_Conflict_Table_Size + conflict_table_index2] )
        return true;
    
    
    // check direct timing hazards
    sc_timestamp ts1, ts2, new_ts2;
    ts1 = process_h1->get_timestamp();
    ts2 = process_h2->get_timestamp();
    unsigned int time_adv_table_index2;
    time_adv_table_index2 = time_adv_table_index_lookup( seg_id2 );
    assert( time_adv_table_index2 < _OoO_Next_Time_Advance_Table_Size );
    new_ts2 = ts2 + sc_timestamp( 
                    _OoO_Next_Time_Advance_Table_Time[time_adv_table_index2]/sc_dt::uint64_to_double( m_time_params->time_resolution ) ,
                    _OoO_Next_Time_Advance_Table_Delta[time_adv_table_index2]);
    if ( new_ts2 < ts1 )
        return true;

    // check indirect timing hazards
    sc_timestamp inc_delta_ts2;
    inc_delta_ts2 = ts2 + sc_timestamp( 0, 1 );
    if ( inc_delta_ts2 < ts1 )
    {
        int events_num = m_delta_events.size();
        for ( int i = 0; i < events_num; i++ )
        {
            int static_methods_num = m_delta_events[i]->m_methods_static.
                                         size();
            for ( int j = 0; j < static_methods_num; j++ )
            {
                int seg_id3;
                seg_id3 = m_delta_events[i]->m_methods_static[j]->
                              get_segment_id();
                int inst_id3;
                inst_id3 = m_delta_events[i]->m_methods_static[j]->
                               get_instance_id();
                if(inst_id3 == -2) return false;
                if(seg_id3 < 0) return false;
                unsigned int conflict_table_index3;
                conflict_table_index3 = conflict_table_index_lookup( seg_id3, 
                                            inst_id3 );
                assert( conflict_table_index3 < _OoO_Data_Conflict_Table_Size );
                // if seg2 notifies seg3
                if ( _OoO_Event_Notify_Table[conflict_table_index2 * 
                         _OoO_Event_Notify_Table_Size + conflict_table_index3] )
                    return true;
            }

            int dynamic_methods_num = m_delta_events[i]->m_methods_dynamic.
                                          size();
            for ( int j = 0; j < dynamic_methods_num; j++ )
            {
                int seg_id3;
                seg_id3 = m_delta_events[i]->m_methods_dynamic[j]->
                              get_segment_id();
                int inst_id3;
                inst_id3 = m_delta_events[i]->m_methods_dynamic[j]->
                               get_instance_id();
                if(inst_id3 == -2) return false;
                if(seg_id3 < 0) return false;
                unsigned int conflict_table_index3;
                conflict_table_index3 = conflict_table_index_lookup( seg_id3, 
                                            inst_id3 );
                assert( conflict_table_index3 < _OoO_Data_Conflict_Table_Size );
                // if seg2 notifies seg3
                if ( _OoO_Event_Notify_Table[conflict_table_index2 * 
                         _OoO_Event_Notify_Table_Size + conflict_table_index3] )
                    return true;
            }

            int static_threads_num = m_delta_events[i]->m_threads_static.
                                         size();
            for ( int j = 0; j < static_threads_num; j++ )
            {
                int seg_id3;
                seg_id3 = m_delta_events[i]->m_threads_static[j]->
                              get_segment_id();
                int inst_id3;
                inst_id3 = m_delta_events[i]->m_threads_static[j]->
                               get_instance_id();
                if(inst_id3 == -2) return false;
                if(seg_id3 < 0) return false;
                unsigned int conflict_table_index3;
                conflict_table_index3 = conflict_table_index_lookup( seg_id3, 
                                            inst_id3 );
                assert( conflict_table_index3 < _OoO_Data_Conflict_Table_Size );
                // if seg2 notifies seg3
                if ( _OoO_Event_Notify_Table[conflict_table_index2 * 
                         _OoO_Event_Notify_Table_Size + conflict_table_index3] )
                    return true;
            }

            int dynamic_threads_num = m_delta_events[i]->m_threads_dynamic.
                                          size();
            for ( int j = 0; j < dynamic_threads_num; j++ )
            {
                int seg_id3;
                seg_id3 = m_delta_events[i]->m_threads_dynamic[j]->
                              get_segment_id();
                int inst_id3;
                inst_id3 = m_delta_events[i]->m_threads_dynamic[j]->
                               get_instance_id();
                if(seg_id3 < 0) return false;
                if(inst_id3 == -2) return false;
                unsigned int conflict_table_index3;
                conflict_table_index3 = conflict_table_index_lookup( seg_id3, 
                                            inst_id3 );
                assert( conflict_table_index3 < _OoO_Data_Conflict_Table_Size );
                // if seg2 notifies seg3
                if ( _OoO_Event_Notify_Table[conflict_table_index2 * 
                         _OoO_Event_Notify_Table_Size + conflict_table_index3] )
                    return true;
            }
        }
    }

    return false;
}

// helper functions
sc_process_b*
sc_simcontext::get_curr_proc() const
{
    if ( m_cor_pkg )
        return (sc_process_b*)m_cor_pkg->get_thread_specific();
    else
        return NULL;
}

void
sc_simcontext::acquire_sched_mutex()
{
    if ( m_cor_pkg ) // m_cor_pkg is 0 in the elaboration phase
        m_cor_pkg->acquire_sched_mutex();
    else
        assert( !m_elaboration_done ); // being in the elaboration phase
}

void
sc_simcontext::release_sched_mutex()
{
    if ( m_cor_pkg ) // m_cor_pkg is 0 in the elaboration phase
        m_cor_pkg->release_sched_mutex();
    else
        assert( !m_elaboration_done ); // being in the elaboration phase
}

// for the following two functions, assume that the running thread has already
// acquired the mutex
void
sc_simcontext::suspend_cor( sc_cor* cor_p )
{
    m_cor_pkg->wait( cor_p );
}

void
sc_simcontext::resume_cor( sc_cor* cor_p )
{
    m_cor_pkg->go( cor_p );
}

// 04/29/2015 GL: get the state of the kernel lock
bool
sc_simcontext::is_locked()
{
    if ( m_cor_pkg ) // m_cor_pkg is 0 in the elaboration phase
        return m_cor_pkg->is_locked();
    else {
        assert( !m_elaboration_done ); // being in the elaboration phase
        return false;
    }
}

bool
sc_simcontext::is_unlocked()
{
    if ( m_cor_pkg ) // m_cor_pkg is 0 in the elaboration phase
        return m_cor_pkg->is_unlocked();
    else {
        assert( !m_elaboration_done ); // being in the elaboration phase
        return true;
    }
}

bool
sc_simcontext::is_lock_owner()
{
    if ( m_cor_pkg ) // m_cor_pkg is 0 in the elaboration phase
        return m_cor_pkg->is_lock_owner();
    else {
        assert( !m_elaboration_done ); // being in the elaboration phase
        return false;
    }
}

bool
sc_simcontext::is_not_owner()
{
    if ( m_cor_pkg ) // m_cor_pkg is 0 in the elaboration phase
        return m_cor_pkg->is_not_owner();
    else {
        assert( !m_elaboration_done ); // being in the elaboration phase
        return true;
    }
}

bool
sc_simcontext::is_locked_and_owner()
{
    if ( m_cor_pkg ) // m_cor_pkg is 0 in the elaboration phase
        return m_cor_pkg->is_locked_and_owner();
    else {
        assert( !m_elaboration_done ); // being in the elaboration phase
        return false;
    }
}

// 09/02/2015 GL: index lookup functions
unsigned int
sc_simcontext::conflict_table_index_lookup( int seg_id, int inst_id )
{
    //old version: nonconsecutive segment id
    /*
    static bool first_time = true;
    static std::map<int, unsigned int> column_index_table;

    // TODO: assert statements are optional in the future
    assert( seg_id >= 0 ); // Valid segment IDs are non-negative.
    assert( inst_id >= 0 ); // Valid instance IDs are non-negative.
    assert( (unsigned int)inst_id < _OoO_Max_Number_of_Instances );

    if ( first_time )
    {
        for ( unsigned int i = 0; i < _OoO_Number_of_Segments; i++ )
            column_index_table[_OoO_Conflict_Index_Lookup_Table[i]] = i;

        first_time = false;
    }

    std::map<int, unsigned int>::iterator it;
    it = column_index_table.find( seg_id );

    if ( it != column_index_table.end() )
    {
        int result = _OoO_Conflict_Index_Lookup_Table[ (inst_id + 1) *
                         _OoO_Number_of_Segments + it->second ];
        assert( result >= 0 );
        return result;
    }
    else
    {
        ::std::cerr << "Conlict table index lookup: invalid segment ID.\n";
        exit(0);
    }
    */
    //new version: consecutive segment id 14:01 2017/3/10 ZC
    
    assert( seg_id >= -1 ); // Valid segment IDs are non-negative.
    assert( inst_id >= 0 ); // Valid instance IDs are non-negative.
    
    int i= inst_id*_OoO_Number_of_Segments+seg_id;
    /*std::cout << "seg_id=" << seg_id << std::endl;
    std::cout << "inst_id=" << inst_id << std::endl;
    std::cout << "i=" << i << std::endl;*/
    return _OoO_Conflict_Index_Lookup_Table[i];
    
}

unsigned int
sc_simcontext::time_adv_table_index_lookup( int seg_id )
{
    /*
    
    static bool first_time = true;
    static std::map<int, unsigned int> column_index_table;

    // TODO: assert statements are optional in the future
    assert( seg_id >= 0 ); // Valid segment IDs are non-negative.

    if ( first_time )
    {
        for ( unsigned int i = 0; i < _OoO_Time_Advance_Index_Lookup_Table_Size; i++ )
            column_index_table[_OoO_Time_Advance_Index_Lookup_Table[i]] = i;

        first_time = false;
    }

    std::map<int, unsigned int>::iterator it;
    it = column_index_table.find( seg_id );

    if ( it != column_index_table.end() )
    {
        int result = it->second;
        assert( result >= 0 );
        return result;
    }
    else
    {
        ::std::cerr << "Time advance table index lookup: invalid segment ID.\n";
        exit(0);
    }
    
    */
    //new version: consecutive segment id 14:02 2017/3/10 ZC
    assert( seg_id >= -1 ); // Valid segment IDs are non-negative.
    
    int i= seg_id;
    
    return _OoO_Time_Advance_Index_Lookup_Table[i];
    
}

void 
sc_simcontext::update_oldest_time( sc_time& curr_time )
{
    sc_time proc_time;

    if ( curr_time == m_oldest_time ) {
        m_oldest_time = m_all_proc.front()->get_timestamp().get_time_count();
        for ( std::list<sc_process_b*>::iterator it = m_all_proc.begin(); 
              it != m_all_proc.end(); ++it ) {
            proc_time = (*it)->get_timestamp().get_time_count();
            if ( proc_time < m_oldest_time ) {
                m_oldest_time = proc_time;
            }
        }
    }
}

#if 0
// +----------------------------------------------------------------------------
// |"sc_simcontext::crunch"
// | 
// | This method implements the simulator's execution of processes. It performs
// | one or more "delta" cycles. Each delta cycle consists of an evaluation,
// | an update phase, and a notification phase. During the evaluation phase any 
// | processes that are ready to run are executed. After all the processes have
// | been executed the update phase is entered. During the update phase the 
// | values of any signals that have changed are updated. After the updates
// | have been performed the notification phase is entered. During that phase
// | any notifications that need to occur because of of signal values changes
// | are performed. This will result in the queueing of processes for execution
// | that are sensitive to those notifications. At that point a delta cycle
// | is complete, and the process is started again unless 'once' is true.
// |
// | Arguments:
// |     once = true if only one delta cycle is to be performed.
// +----------------------------------------------------------------------------
inline void
sc_simcontext::crunch( bool once )
{
#ifdef DEBUG_SYSTEMC
    int num_deltas = 0;  // number of delta cycles
#endif

    while ( true ) 
    {

        // EVALUATE PHASE

        m_execution_phase = phase_evaluate;
        empty_eval_phase = true;
        m_cor_pkg->acquire_sched_mutex();
    mapper( m_cor );
        m_cor_pkg->release_sched_mutex();

        // check for errors
        if( m_error ) {
            goto out;
        }

        // check for call(s) to sc_stop
        if( m_forced_stop ) {
            if ( stop_mode == SC_STOP_IMMEDIATE ) goto out;
        }

        // remove finally dead zombies:

        while( ! m_collectable->empty() )
        {
        sc_process_b* del_p = m_collectable->front();
        m_collectable->pop_front();
        del_p->reference_decrement();
        }


    // UPDATE PHASE
    //
    // The change stamp must be updated first so that event_occurred()
    // will work.

    m_execution_phase = phase_update;
    if ( !empty_eval_phase ) 
    {
//      SC_DO_PHASE_CALLBACK_(evaluation_done);
        m_change_stamp++;
        m_delta_count ++;
    }
    m_prim_channel_registry->perform_update();
    SC_DO_PHASE_CALLBACK_(update_done);
    m_execution_phase = phase_notify;

#if SC_SIMCONTEXT_TRACING_
    if( m_something_to_trace ) {
        trace_cycle( /* delta cycle? */ true );
    }
#endif

        // check for call(s) to sc_stop
        if( m_forced_stop ) {
            break;
        }

#ifdef DEBUG_SYSTEMC
        // check for possible infinite loops
        if( ++ num_deltas > SC_MAX_NUM_DELTA_CYCLES ) {
        ::std::cerr << "SystemC warning: "
         << "the number of delta cycles exceeds the limit of "
         << SC_MAX_NUM_DELTA_CYCLES
         << ", defined in sc_constants.h.\n"
         << "This is a possible sign of an infinite loop.\n"
         << "Increase the limit if this warning is invalid.\n";
        break;
    }
#endif

    // NOTIFICATION PHASE:
    //
    // Process delta notifications which will queue processes for 
    // subsequent execution.

        int size = m_delta_events.size();
    if ( size != 0 )
    {
        sc_event** l_events = &m_delta_events[0];
        int i = size - 1;
        do {
        l_events[i]->trigger();
        } while( -- i >= 0 );
        m_delta_events.resize(0);
    }

    if( m_runnable->is_empty() ) {
        // no more runnable processes
        break;
    }

    // if sc_pause() was called we are done.

    if ( m_paused ) break;

        // IF ONLY DOING ONE CYCLE, WE ARE DONE. OTHERWISE EXECUTE NEW
        // CALLBACKS

        if ( once ) break;
    }

    // When this point is reached the processing of delta cycles is complete,
    // if the completion was because of an error throw the exception specified
    // by '*m_error'.
out:
    this->reset_curr_proc();
    if( m_error ) throw *m_error; // re-throw propagated error
}
#endif


inline
void
sc_simcontext::cycle( const sc_time& t)
{
    assert( 0 ); // 08/20/2015 GL: to support m_one_timed_cycle in the future
#if 0
    sc_time next_event_time;

    m_in_simulator_control = true;
    {
        // 05/25/2015 GL: sc_kernel_lock constructor acquires the kernel lock
        sc_kernel_lock lock;

#ifdef SC_LOCK_CHECK
        assert( is_locked_and_owner() );
#endif /* SC_LOCK_CHECK */
        m_one_timed_cycle = true;
        oooschedule( m_cor );
        m_one_timed_cycle = false;
        // 05/25/2015 GL: sc_kernel_lock destructor releases the kernel lock
    }
#ifdef SC_LOCK_CHECK 
    assert( is_not_owner() );
#endif /* SC_LOCK_CHECK */
    SC_DO_PHASE_CALLBACK_(before_timestep);
#if SC_SIMCONTEXT_TRACING_
    if( m_something_to_trace ) {
        trace_cycle( /* delta cycle? */ false );
    }
#endif
    m_curr_time += t;
    if ( next_time(next_event_time) && next_event_time <= m_curr_time) {
        SC_REPORT_WARNING(SC_ID_CYCLE_MISSES_EVENTS_, "");
    }
    m_in_simulator_control = false;
    SC_DO_PHASE_CALLBACK_(simulation_paused);
#endif
}


void
sc_simcontext::elaborate()
{
    if(_OoO_Table_File_Name != NULL)
    {
        std::ifstream fin(_OoO_Table_File_Name);
        if(!fin.good())
        {
            std::cout << "Can't open the table file: " << _OoO_Table_File_Name << std::endl;
            exit(1);
        }
        //load _OoO_Data_Conflict_Table
        fin.read(reinterpret_cast<char*>(&_OoO_Data_Conflict_Table[0]), 
            _OoO_Data_Conflict_Table_Size * 
            _OoO_Data_Conflict_Table_Size * 
            sizeof(bool)
        );
        // std::cout << _OoO_Data_Conflict_Table_Size*_OoO_Data_Conflict_Table_Size << std::endl;
        // for(unsigned int i = 0; 
        //     i < _OoO_Data_Conflict_Table_Size*_OoO_Data_Conflict_Table_Size;
        //     ++i)
        // {
        //     std::cout << _OoO_Data_Conflict_Table[i]<< " ";
        // }

        //load _OoO_Event_Notify_Table
        if (fin.eof()) 
        {
            std::cout << "Not enough data in the table file" << std::endl;
            exit(1);
        }
        fin.read(reinterpret_cast<char*>(&_OoO_Event_Notify_Table[0]), 
            _OoO_Event_Notify_Table_Size  * 
            _OoO_Event_Notify_Table_Size  *
            sizeof(bool)
        );
        // std::cout << _OoO_Event_Notify_Table_Size*_OoO_Event_Notify_Table_Size << std::endl;
        // for(unsigned int i = 0; 
        //     i < _OoO_Event_Notify_Table_Size*_OoO_Event_Notify_Table_Size;
        //     ++i)
        // {
        //     std::cout << _OoO_Event_Notify_Table[i]<< " ";
        // }

        //load _OoO_Conflict_Index_Lookup_Table
        if (fin.eof()) 
        {
            std::cout << "Not enough data in the table file" << std::endl;
            exit(1);
        }
        fin.read(reinterpret_cast<char*>(&_OoO_Conflict_Index_Lookup_Table[0]), 
            (_OoO_Max_Number_of_Instances+1)  * 
            _OoO_Number_of_Segments  *
            sizeof(int)
        );
        // std::cout << (_OoO_Max_Number_of_Instances+1)*_OoO_Number_of_Segments << std::endl;
        // for(unsigned int i = 0; 
        //     i < (_OoO_Max_Number_of_Instances+1)*_OoO_Number_of_Segments;
        //     ++i)
        // {
        //     std::cout << _OoO_Conflict_Index_Lookup_Table[i]<< " ";
        // }

        //load _OoO_Curr_Time_Advance_Table_Time
        if (fin.eof()) 
        {
            std::cout << "Not enough data in the table file" << std::endl;
            exit(1);
        }
        fin.read(reinterpret_cast<char*>(&_OoO_Curr_Time_Advance_Table_Time[0]), 
            _OoO_Curr_Time_Advance_Table_Size  *
            sizeof(long long int)
        );
        // std::cout << _OoO_Curr_Time_Advance_Table_Size << std::endl;
        // for(unsigned int i = 0; 
        //     i < _OoO_Curr_Time_Advance_Table_Size;
        //     ++i)
        // {
        //     std::cout << _OoO_Curr_Time_Advance_Table_Time[i]<< " ";
        // }

        //load _OoO_Curr_Time_Advance_Table_Delta
        if (fin.eof()) 
        {
            std::cout << "Not enough data in the table file" << std::endl;
            exit(1);
        }
        fin.read(reinterpret_cast<char*>(&_OoO_Curr_Time_Advance_Table_Delta[0]),
            _OoO_Curr_Time_Advance_Table_Size  *
            sizeof(int)
        );
        // std::cout << _OoO_Curr_Time_Advance_Table_Size << std::endl;
        // for(unsigned int i = 0; 
        //     i < _OoO_Curr_Time_Advance_Table_Size;
        //     ++i)
        // {
        //     std::cout << _OoO_Curr_Time_Advance_Table_Delta[i]<< " ";
        // }

        //load _OoO_Next_Time_Advance_Table_Time
        if (fin.eof()) 
        {
            std::cout << "Not enough data in the table file" << std::endl;
            exit(1);
        }
        fin.read(reinterpret_cast<char*>(&_OoO_Next_Time_Advance_Table_Time[0]), 
            _OoO_Next_Time_Advance_Table_Size  *
            sizeof(long long int)
        );
        // std::cout << _OoO_Next_Time_Advance_Table_Size << std::endl;
        // for(unsigned int i = 0; 
        //     i < _OoO_Next_Time_Advance_Table_Size;
        //     ++i)
        // {
        //     std::cout << _OoO_Next_Time_Advance_Table_Time[i]<< " ";
        // }

        //load _OoO_Next_Time_Advance_Table_Delta
        if (fin.eof()) 
        {
            std::cout << "Not enough data in the table file" << std::endl;
            exit(1);
        }
        fin.read(reinterpret_cast<char*>(&_OoO_Next_Time_Advance_Table_Delta[0]),
            _OoO_Next_Time_Advance_Table_Size  *
            sizeof(int)
        );
        // std::cout << _OoO_Next_Time_Advance_Table_Size << std::endl;
        // for(unsigned int i = 0; 
        //     i < _OoO_Next_Time_Advance_Table_Size;
        //     ++i)
        // {
        //     std::cout << _OoO_Next_Time_Advance_Table_Delta[i]<< " ";
        // }

        //load _OoO_Time_Advance_Index_Lookup_Table
        if (fin.eof()) 
        {
            std::cout << "Not enough data in the table file" << std::endl;
            exit(1);
        }
        fin.read(reinterpret_cast<char*>(&_OoO_Time_Advance_Index_Lookup_Table[0]), 
            _OoO_Time_Advance_Index_Lookup_Table_Size  *
            sizeof(int)
        );
        // std::cout << _OoO_Time_Advance_Index_Lookup_Table_Size << std::endl;
        // for(unsigned int i = 0; 
        //     i < _OoO_Time_Advance_Index_Lookup_Table_Size;
        //     ++i)
        // {
        //     std::cout << _OoO_Time_Advance_Index_Lookup_Table[i]<< " ";
        // }

        //load _OoO_Combined_Data_Conflict_Table
        if (fin.eof()) 
        {
            std::cout << "Not enough data in the table file" << std::endl;
            exit(1);
        }
        fin.read(reinterpret_cast<char*>(&_OoO_Combined_Data_Conflict_Table[0]), 
            _OoO_Combined_Data_Conflict_Table_Size *
            sizeof(int)
        ); 
        // std::cout << _OoO_Combined_Data_Conflict_Table_Size << std::endl;
        // for(unsigned int i = 0; 
        //     i < _OoO_Combined_Data_Conflict_Table_Size;
        //     ++i)
        // {
        //     std::cout << _OoO_Combined_Data_Conflict_Table[i]<< " ";
        // }

        //load _OoO_Combined_Data_Conflict_Lookup_Table
        if (fin.eof()) 
        {
            std::cout << "Not enough data in the table file" << std::endl;
            exit(1);
        }
        fin.read(reinterpret_cast<char*>(&_OoO_Combined_Data_Conflict_Lookup_Table[0]), 
            _OoO_Combined_Data_Conflict_Lookup_Table_Number_Segments *
            (1+_OoO_Combined_Data_Conflict_Lookup_Table_Max_Instances) *
            sizeof(int)
        ); 
        // std::cout << _OoO_Combined_Data_Conflict_Lookup_Table_Number_Segments *
        //     (1+_OoO_Combined_Data_Conflict_Lookup_Table_Max_Instances) << std::endl;
        // for(unsigned int i = 0; 
        //     i < _OoO_Combined_Data_Conflict_Lookup_Table_Number_Segments *
        //     (1+_OoO_Combined_Data_Conflict_Lookup_Table_Max_Instances);
        //     ++i)
        // {
        //     std::cout << _OoO_Combined_Data_Conflict_Lookup_Table[i]<< " ";
        // }

        //load _OoO_Prediction_Time_Advance_Table_Time_Units
        if (fin.eof()) 
        {
            std::cout << "Not enough data in the table file" << std::endl;
            exit(1);
        }
        fin.read(reinterpret_cast<char*>(&_OoO_Prediction_Time_Advance_Table_Time_Units[0]), 
            _OoO_Prediction_Time_Advance_Table_Number_Segments *
            (1+_OoO_Prediction_Time_Advance_Table_Number_Steps) *
            sizeof(long long int)
        ); 
        // std::cout << _OoO_Prediction_Time_Advance_Table_Number_Segments *
        //     (1+_OoO_Prediction_Time_Advance_Table_Number_Steps) << std::endl;
        // for(unsigned int i = 0; 
        //     i < _OoO_Prediction_Time_Advance_Table_Number_Segments *
        //     (1+_OoO_Prediction_Time_Advance_Table_Number_Steps);
        //     ++i)
        // {
        //     std::cout << _OoO_Prediction_Time_Advance_Table_Time_Units[i]<< " ";
        // }

        //load _OoO_Prediction_Time_Advance_Table_Delta
        if (fin.eof()) 
        {
            std::cout << "Not enough data in the table file" << std::endl;
            exit(1);
        }
        fin.read(reinterpret_cast<char*>(&_OoO_Prediction_Time_Advance_Table_Delta[0]), 
            _OoO_Prediction_Time_Advance_Table_Number_Segments *
            (1+_OoO_Prediction_Time_Advance_Table_Number_Steps) *
            sizeof(int)
        ); 
        // std::cout << _OoO_Prediction_Time_Advance_Table_Number_Segments *
        //     (1+_OoO_Prediction_Time_Advance_Table_Number_Steps) << std::endl;
        // for(unsigned int i = 0; 
        //     i < _OoO_Prediction_Time_Advance_Table_Number_Segments *
        //     (1+_OoO_Prediction_Time_Advance_Table_Number_Steps);
        //     ++i)
        // {
        //     std::cout << _OoO_Prediction_Time_Advance_Table_Delta[i]<< " ";
        // }

        //load _OoO_Prediction_Time_Advance_Lookup_Table
        if (fin.eof()) 
        {
            std::cout << "Not enough data in the table file" << std::endl;
            exit(1);
        }
        fin.read(reinterpret_cast<char*>(&_OoO_Prediction_Time_Advance_Lookup_Table[0]), 
            _OoO_Prediction_Time_Advance_Table_Number_Segments *
            sizeof(int)
        ); 
        // std::cout << _OoO_Prediction_Time_Advance_Table_Number_Segments << std::endl;
        // for(unsigned int i = 0; 
        //     i < _OoO_Prediction_Time_Advance_Table_Number_Segments;
        //     ++i)
        // {
        //     std::cout << _OoO_Prediction_Time_Advance_Lookup_Table[i]<< " ";
        // }
        
        //load _OoO_Prediction_Event_Notification_Table_Time_Units
        if (fin.eof()) 
        {
            std::cout << "Not enough data in the table file" << std::endl;
            exit(1);
        }
        fin.read(reinterpret_cast<char*>(
            &_OoO_Prediction_Event_Notification_Table_Time_Units[0]), 
            _OoO_Combined_Data_Conflict_Table_Size *
            sizeof(long long int)
        ); 
        // std::cout << _OoO_Combined_Data_Conflict_Table_Size << std::endl;
        // for(unsigned int i = 0; 
        //     i < _OoO_Combined_Data_Conflict_Table_Size;
        //     ++i)
        // {
        //     std::cout << _OoO_Prediction_Event_Notification_Table_Time_Units[i]<< " ";
        // }

        //load _OoO_Prediction_Event_Notification_Table_Delta
        if (fin.eof()) 
        {
            std::cout << "Not enough data in the table file" << std::endl;
            exit(1);
        }
        fin.read(reinterpret_cast<char*>(
            &_OoO_Prediction_Event_Notification_Table_Delta[0]), 
            _OoO_Combined_Data_Conflict_Table_Size *
            sizeof(int)
        ); 
        // std::cout << _OoO_Combined_Data_Conflict_Table_Size << std::endl;
        // for(unsigned int i = 0; 
        //     i < _OoO_Combined_Data_Conflict_Table_Size;
        //     ++i)
        // {
        //     std::cout << _OoO_Prediction_Event_Notification_Table_Delta[i]<< " ";
        // }

        //load _OoO_Prediction_Event_Notification_Lookup_Table
        if (fin.eof()) 
        {
            std::cout << "Not enough data in the table file" << std::endl;
            exit(1);
        }
        fin.read(reinterpret_cast<char*>(
            &_OoO_Prediction_Event_Notification_Lookup_Table[0]), 
            _OoO_Prediction_Event_Notification_Table_Number_Segments *
            (1+_OoO_Prediction_Event_Notification_Table_Max_Instances) *
            sizeof(int)
        ); 



        //load _OoO_Prediction_Event_Notification_Table_No_Indirect_Time_Units
        if (fin.eof()) 
        {
            std::cout << "Not enough data in the table file" << std::endl;
            exit(1);
        }
        fin.read(reinterpret_cast<char*>(
            &_OoO_Prediction_Event_Notification_Table_No_Indirect_Time_Units[0]), 
            _OoO_Combined_Data_Conflict_Table_Size *
            sizeof(long long int)
        ); 
        // std::cout << _OoO_Combined_Data_Conflict_Table_Size << std::endl;
        // for(unsigned int i = 0; 
        //     i < _OoO_Combined_Data_Conflict_Table_Size;
        //     ++i)
        // {
        //     std::cout << _OoO_Prediction_Event_Notification_Table_No_Indirect_Time_Units[i]<< " ";
        // }

        //load _OoO_Prediction_Event_Notification_Table_No_Indirect_Delta
        if (fin.eof()) 
        {
            std::cout << "Not enough data in the table file" << std::endl;
            exit(1);
        }
        fin.read(reinterpret_cast<char*>(
            &_OoO_Prediction_Event_Notification_Table_No_Indirect_Delta[0]), 
            _OoO_Combined_Data_Conflict_Table_Size *
            sizeof(int)
        ); 
        // std::cout << _OoO_Combined_Data_Conflict_Table_Size << std::endl;
        // for(unsigned int i = 0; 
        //     i < _OoO_Combined_Data_Conflict_Table_Size;
        //     ++i)
        // {
        //     std::cout << _OoO_Prediction_Event_Notification_Table_No_Indirect_Delta[i]<< " ";
        // }

        //load _OoO_Prediction_Event_Notification_No_Indirect_Lookup_Table
        if (fin.eof()) 
        {
            std::cout << "Not enough data in the table file" << std::endl;
            exit(1);
        }
        fin.read(reinterpret_cast<char*>(
            &_OoO_Prediction_Event_Notification_No_Indirect_Lookup_Table[0]), 
            _OoO_Prediction_Event_Notification_Table_No_Indirect_Number_Segments *
            (1+_OoO_Prediction_Event_Notification_Table_No_Indirect_Max_Instances) *
            sizeof(int)
        ); 


        char endOfFile;
        fin.read(&endOfFile, sizeof(char));
        if (!fin.eof()) 
        {
            std::cout << "Still extra data in the table file" << std::endl;
            exit(1);
        }
    }
    if( m_elaboration_done || sim_status() != SC_SIM_OK ) {
        return;
    }

    // Instantiate the method invocation module
    // (not added to public object hierarchy)

    //m_method_invoker_p =
    //  new sc_invoke_method("$$$$kernel_module$$$$_invoke_method" );
    // 08/20/2015 GL: clean up sc_invoke_method in the future
    m_method_invoker_p = NULL;

    m_simulation_status = SC_BEFORE_END_OF_ELABORATION;
    for( int cd = 0; cd != 4; /* empty */ )
    {
        cd  = m_port_registry->construction_done();
        cd += m_export_registry->construction_done();
        cd += m_prim_channel_registry->construction_done();
        cd += m_module_registry->construction_done();

        // check for call(s) to sc_stop
        if( m_forced_stop ) {
            do_sc_stop_action();
            return;
        }

    }
    SC_DO_PHASE_CALLBACK_(construction_done);

    // SIGNAL THAT ELABORATION IS DONE
    //
    // We set the switch before the calls in case someone creates a process 
    // in an end_of_elaboration callback. We need the information to flag 
    // the process as being dynamic.

    m_elaboration_done = true;
    m_simulation_status = SC_END_OF_ELABORATION;

    m_port_registry->elaboration_done();
    m_export_registry->elaboration_done();
    m_prim_channel_registry->elaboration_done();
    m_module_registry->elaboration_done();
    SC_DO_PHASE_CALLBACK_(elaboration_done);
    sc_reset::reconcile_resets();

    // check for call(s) to sc_stop
    if( m_forced_stop ) {
        do_sc_stop_action();
        return;
    }
}

void
sc_simcontext::prepare_to_simulate()
{
    sc_method_handle  method_p;  // Pointer to method process accessing.
    sc_thread_handle  thread_p;  // Pointer to thread process accessing.

    if( m_ready_to_simulate || sim_status() != SC_SIM_OK ) {
        return;
    }

    // instantiate the coroutine package
    m_cor_pkg = new sc_cor_pkg_t( this );
    m_cor = m_cor_pkg->get_main();

    // 10/29/2014 GL: initialize the thread-specific data of the root thread
    m_cor_pkg->set_thread_specific( NULL );

    // NOTIFY ALL OBJECTS THAT SIMULATION IS ABOUT TO START:

    m_simulation_status = SC_START_OF_SIMULATION;
    m_port_registry->start_simulation();
    m_export_registry->start_simulation();
    m_prim_channel_registry->start_simulation();
    m_module_registry->start_simulation();
    SC_DO_PHASE_CALLBACK_(start_simulation);
    m_start_of_simulation_called = true;

    // CHECK FOR CALL(S) TO sc_stop 

    if( m_forced_stop ) {
        do_sc_stop_action();
        return;
    }

    // PREPARE ALL METHOD PROCESSES FOR SIMULATION:

    for ( method_p = m_process_table->method_q_head(); 
      method_p; method_p = method_p->next_exist() )
    {
    method_p->prepare_for_simulation();
    }

    // PREPARE ALL (C)THREAD PROCESSES FOR SIMULATION:

    for ( thread_p = m_process_table->thread_q_head(); 
      thread_p; thread_p = thread_p->next_exist() )
    {
    thread_p->prepare_for_simulation();
    }

    // PREPARE ALL INVOKER PROCESSES FOR SIMULATION: DM 05/20/2019

    for ( std::vector<Invoker*>::iterator invok_iter = m_invokers.begin(); 
      invok_iter != m_invokers.end(); invok_iter++ )
    {
    thread_p = (*invok_iter)->proc_handle;
    thread_p->prepare_for_simulation();
    }

    m_simulation_status = SC_RUNNING;
    m_ready_to_simulate = true;
    m_runnable->init();

    // update phase

    m_execution_phase = phase_update;
    m_prim_channel_registry->perform_update();
    m_execution_phase = phase_notify;

    int size;

    // make all method processes runnable

    for ( method_p = m_process_table->method_q_head(); 
      method_p; method_p = method_p->next_exist() )
    {
    if ( ((method_p->m_state & sc_process_b::ps_bit_disabled) != 0) ||
         method_p->dont_initialize() ) 
    {
        if ( method_p->m_sensitivity_events->size() == 0 )
        {
            SC_REPORT_WARNING( SC_ID_DISABLE_WILL_ORPHAN_PROCESS_, 
                           method_p->name() );
        }
        else { //DM 05/27/2019 force the thread to wait on its sensitivity list
            method_p->m_sensitivity_events->add_dynamic(RCAST<sc_method_handle>( method_p ));
        method_p->m_event_list_p = method_p->m_sensitivity_events;
        method_p->m_event_count = method_p->m_sensitivity_events->size();
        method_p->m_trigger_type = sc_process_b::OR_LIST;
        method_p->m_process_state=2;
        add_to_wait_queue( method_p );
        }

    }
    else if ( (method_p->m_state & sc_process_b::ps_bit_suspended) == 0) 
    {
        push_runnable_method_front( method_p );
        }
    else
    {
        method_p->m_state |= sc_process_b::ps_bit_ready_to_run;
    }
    }

    // make thread processes runnable
    // (cthread processes always have the dont_initialize flag set)

    for ( thread_p = m_process_table->thread_q_head(); 
      thread_p; thread_p = thread_p->next_exist() )
    {
    if ( ((thread_p->m_state & sc_process_b::ps_bit_disabled) != 0) || 
         thread_p->dont_initialize() ) 
    {
        if ( thread_p->m_sensitivity_events->size() == 0 )
        {
            SC_REPORT_WARNING( SC_ID_DISABLE_WILL_ORPHAN_PROCESS_, 
                           thread_p->name() );
        }
        else { //DM 05/27/2019 force the thread to wait on its sensitivity list
            thread_p->m_sensitivity_events->add_dynamic(RCAST<sc_thread_handle>( thread_p ));
        thread_p->m_event_list_p = thread_p->m_sensitivity_events;
        thread_p->m_event_count = thread_p->m_sensitivity_events->size();
        thread_p->m_trigger_type = sc_process_b::OR_LIST;
        thread_p->m_process_state=2;
        add_to_wait_queue( thread_p );
        }
    }
    else if ( (thread_p->m_state & sc_process_b::ps_bit_suspended) == 0) 
    {
            push_runnable_thread_front( thread_p );
        }
    else
    {
        thread_p->m_state |= sc_process_b::ps_bit_ready_to_run;
    }
    }


    // process delta notifications

    if( ( size = m_delta_events.size() ) != 0 ) {
        sc_event** l_delta_events = &m_delta_events[0];
        int i = size - 1;
        do {
            l_delta_events[i]->trigger();
        } while( -- i >= 0 );
        m_delta_events.resize(0);
    }

    SC_DO_PHASE_CALLBACK_(initialization_done);
}

void
sc_simcontext::initial_crunch( bool no_crunch )
{
    if( no_crunch || m_runnable->is_empty() ) {
        return;
    }

    // run the delta cycle loop

    {
        // 08/19/2015 GL: to support 'm_one_timed_cycle' in the future
        assert( 0 );

        // 05/25/2015 GL: sc_kernel_lock constructor acquires the kernel lock
        sc_kernel_lock lock;

#ifdef SC_LOCK_CHECK
        assert( is_locked_and_owner() );
#endif /* SC_LOCK_CHECK */
        m_one_timed_cycle = true;
        oooschedule( m_cor );
        m_one_timed_cycle = false;
        // 05/25/2015 GL: sc_kernel_lock destructor releases the kernel lock
    }
#ifdef SC_LOCK_CHECK
    assert( is_not_owner() );
#endif /* SC_LOCK_CHECK */
    if( m_error ) {
        return;
    }

#if SC_SIMCONTEXT_TRACING_
    if( m_something_to_trace ) {
        trace_cycle( false );
    }
#endif

    // check for call(s) to sc_stop
    if( m_forced_stop ) {
        do_sc_stop_action();
    }
}

void
sc_simcontext::initialize( bool no_crunch )
{
    m_in_simulator_control = true;
    elaborate();

    prepare_to_simulate();
    initial_crunch(no_crunch);
    m_in_simulator_control = false;
}

#if 0
// +----------------------------------------------------------------------------
// |"sc_simcontext::simulate"
// | 
// | This method runs the simulation for the specified amount of time.
// |
// | Notes:
// |   (1) This code always run with an SC_EXIT_ON_STARVATION starvation policy,
// |       so the simulation time on return will be the minimum of the 
// |       simulation on entry plus the duration, and the maximum time of any 
// |       event present in the simulation. If the simulation policy is
// |       SC_RUN_TO_TIME starvation it is implemented by the caller of this 
// |       method, e.g., sc_start(), by artificially setting the simulation
// |       time forward after this method completes.
// |
// | Arguments:
// |     duration = amount of time to simulate.
// +----------------------------------------------------------------------------
void
sc_simcontext::simulate( const sc_time& duration )
{
    initialize( true );
    //for ( std::list<sc_process_b*>::iterator it = m_all_proc.begin();  it != m_all_proc.end(); it++ )
    //{
    //  printf("thread %s state: %d\n",(*it)->name(),(*it)->m_process_state);
    //}
    
    if (sim_status() != SC_SIM_OK) {
    return;
    }

    sc_time non_overflow_time = sc_max_time() - m_curr_time;
    if ( duration > non_overflow_time )
    {
    SC_REPORT_ERROR(SC_ID_SIMULATION_TIME_OVERFLOW_, "");
    return;
    }
    else if ( duration < SC_ZERO_TIME )
    {
        SC_REPORT_ERROR(SC_ID_NEGATIVE_SIMULATION_TIME_,"");
    }

    m_in_simulator_control = true;
    m_paused = false;

    m_finish_time = m_curr_time + duration;

    // IF DURATION WAS ZERO WE ONLY CRUNCH ONCE:
    //
    // We duplicate the code so that we don't add the overhead of the
    // check to each loop in the do below.
    if ( duration == SC_ZERO_TIME ) 
    {
    m_in_simulator_control = true;
        {
            // 05/25/2015 GL: sc_kernel_lock constructor acquires the kernel 
            //                lock
            sc_kernel_lock lock;

#ifdef SC_LOCK_CHECK
            assert( is_locked_and_owner() );
#endif /* SC_LOCK_CHECK */
            m_one_delta_cycle = true;
        oooschedule( m_cor );
            m_one_delta_cycle = false;
            // 05/25/2015 GL: sc_kernel_lock destructor releases the kernel 
            //                lock
        }
#ifdef SC_LOCK_CHECK
        assert( is_not_owner() );
#endif /* SC_LOCK_CHECK */
    if( m_error ) {
        m_in_simulator_control = false;
        return;
    }
#if SC_SIMCONTEXT_TRACING_
        if( m_something_to_trace )
            trace_cycle( /* delta cycle? */ false );
#endif
        if( m_forced_stop ) {
            do_sc_stop_action();
            return;
        }
        // return via implicit pause
        m_execution_phase      = phase_evaluate;
        m_in_simulator_control = false;
        SC_DO_PHASE_CALLBACK_(simulation_paused);
    }
    // NON-ZERO DURATION: EXECUTE UP TO THAT TIME, OR UNTIL EVENT STARVATION:
    else {
        // 05/25/2015 GL: sc_kernel_lock constructor acquires the kernel lock
        sc_kernel_lock lock;

#ifdef SC_LOCK_CHECK
        assert( is_locked_and_owner() );
#endif /* SC_LOCK_CHECK */
        oooschedule( m_cor );
        // 05/25/2015 GL: sc_kernel_lock destructor releases the kernel lock
    }
#ifdef SC_LOCK_CHECK
    assert( is_not_owner() );
#endif /* SC_LOCK_CHECK */

    // remove finally dead zombies:

    while( ! m_collectable->empty() )
    {
        sc_process_b* del_p = m_collectable->front();
        m_collectable->pop_front();
        del_p->reference_decrement();
    }
}
#endif

// +----------------------------------------------------------------------------
// |"sc_simcontext::simulate"
// | 
// | This method runs the simulation. This is a wrapper around ooo_schedule()
// | which does the actual job.
// |
// | Arguments:
// |     duration = amount of time to simulate.
// +----------------------------------------------------------------------------
void
sc_simcontext::simulate( const sc_time& duration )
{
    initialize(true);
    if (sim_status() != SC_SIM_OK)
    {
	return;
    }

    sc_time non_overflow_time = sc_max_time() - m_simulation_time.m_time_count;
    if ( duration > non_overflow_time )
    {
        SC_REPORT_ERROR(SC_ID_SIMULATION_TIME_OVERFLOW_, "");
        return;
    }
    else if ( duration < SC_ZERO_TIME )
    {
        SC_REPORT_ERROR(SC_ID_NEGATIVE_SIMULATION_TIME_,"");
    }

    for(std::vector<sc_process_b*>::iterator
	process_it  = m_paused_processes.begin();
	process_it != m_paused_processes.end();
	process_it ++)
    {
	if(_SYSC_SYNC_PAR_SIM==true)
	{
	    m_synch_thread_queue.push_back( *process_it );
	    ( *process_it )->m_process_state=32;
	}
	else
	{
	    push_runnable_thread(RCAST<sc_thread_handle>(*process_it));
	}
    }
    m_paused_processes.clear();
    m_paused = false;

    m_in_simulator_control = true;
    {
	// 05/25/2015 GL: sc_kernel_lock constructor acquires the kernel lock
	sc_kernel_lock lock;
	#ifdef SC_LOCK_CHECK
	assert( is_locked_and_owner() );
	#endif /* SC_LOCK_CHECK */

	oooschedule( m_cor );

	// 05/25/2015 GL: sc_kernel_lock destructor releases the kernel lock
    }
    #ifdef SC_LOCK_CHECK
    assert( is_not_owner() );
    #endif /* SC_LOCK_CHECK */

    if( m_forced_stop )
    {
	do_sc_stop_action();
	return;
    }
    m_in_simulator_control = false;
}

void
sc_simcontext::do_sc_stop_action()
{
    SC_REPORT_INFO("/OSCI/SystemC","Simulation stopped by user.");
    if (m_start_of_simulation_called) {
	end();
	m_in_simulator_control = false;
    }
    m_simulation_status = SC_STOPPED;
    SC_DO_PHASE_CALLBACK_(simulation_stopped);
}

void
sc_simcontext::mark_to_collect_process( sc_process_b* zombie )
{
    m_collectable->push_back( zombie );
}


//------------------------------------------------------------------------------
//"sc_simcontext::stop"
//
// This method stops the simulator after some amount of further processing.
// How much processing is done depends upon the value of the global variable
// stop_mode:
//     SC_STOP_IMMEDIATE - aborts the execution phase of the current delta
//                         cycle and performs whatever updates are pending.
//     SC_STOP_FINISH_DELTA - finishes the current delta cycle - both execution
//                            and updates.
// If sc_stop is called outside of the purview of the simulator kernel 
// (e.g., directly from sc_main), the end of simulation notifications 
// are performed. From within the purview of the simulator kernel, these
// will be performed at a later time.
//------------------------------------------------------------------------------

void
sc_simcontext::stop()
{
    static bool stop_warning_issued = false;
    if (m_forced_stop)
    {
        if ( !stop_warning_issued )
        {
            stop_warning_issued = true; // This must be before the WARNING!!!
            SC_REPORT_WARNING(SC_ID_SIMULATION_STOP_CALLED_TWICE_, "");
        }
        return;
    }
    // Accellera does this:
//  if ( stop_mode == SC_STOP_IMMEDIATE ) m_runnable->init();
//  m_forced_stop = true;

    // We can replicate SC_STOP_FINISH_DELTA accurately,
    // but not the inherently non-deterministic
    // SC_STOP_IMMEDIATE behavior,
    // so we take a best-effort approach for that.
    //
    // In order to stop, we short-cut the m_simulation_duration.
    // For SC_STOP_IMMEDIATE,
    // we set it to this thread's current time.
    // For SC_STOP_FINISH_DELTA,
    // we set it to this thread's current time, plus one delta.
    sc_process_b* proc = get_curr_proc();
    m_simulation_time =
	m_simulation_duration = proc->get_timestamp();
    if (stop_mode == SC_STOP_FINISH_DELTA)
    {
	m_simulation_duration.m_delta_count++;
    }
    m_forced_stop = true;
    if ( !m_in_simulator_control  )
    {
        do_sc_stop_action();
    } 
}

void
sc_simcontext::reset()
{
    clean();
    init();
}

void
sc_simcontext::end()
{
    m_simulation_status = SC_END_OF_SIMULATION;
    m_ready_to_simulate = false;
    m_port_registry->simulation_done();
    m_export_registry->simulation_done();
    m_prim_channel_registry->simulation_done();
    m_module_registry->simulation_done();
    SC_DO_PHASE_CALLBACK_(simulation_done);
    m_end_of_simulation_called = true;
}

void
sc_simcontext::hierarchy_push( sc_module* mod )
{
    m_object_manager->hierarchy_push( mod );
}

sc_module*
sc_simcontext::hierarchy_pop()
{
    return static_cast<sc_module*>( m_object_manager->hierarchy_pop() );
}

sc_module*
sc_simcontext::hierarchy_curr() const
{
    return static_cast<sc_module*>( m_object_manager->hierarchy_curr() );
}
    
sc_object*
sc_simcontext::first_object()
{
    return m_object_manager->first_object();
}

sc_object*
sc_simcontext::next_object()
{
    return m_object_manager->next_object();
}

sc_object*
sc_simcontext::find_object( const char* name )
{
    static bool warn_find_object=true;
    if ( warn_find_object )
    {
    warn_find_object = false;
    SC_REPORT_INFO(SC_ID_IEEE_1666_DEPRECATION_,
        "sc_simcontext::find_object() is deprecated,\n" \
            " use sc_find_object()" );
    }
    return m_object_manager->find_object( name );
}

// to generate unique names for objects in an MT-Safe way

const char*
sc_simcontext::gen_unique_name( const char* basename_, bool preserve_first )
{
    return m_name_gen->gen_unique_name( basename_, preserve_first );
}


sc_process_handle 
sc_simcontext::create_cthread_process( 
    const char* name_p, bool free_host, SC_ENTRY_FUNC method_p,         
    sc_process_host* host_p, const sc_spawn_options* opt_p,
    int seg_id, int inst_id )
{
    sc_thread_handle handle = 
        new sc_cthread_process(name_p, free_host, method_p, host_p, opt_p);

    // 08/17/2015 GL: set the starting segment ID of this cthread
    handle->set_segment_id( seg_id );

    // 09/02/2015 GL: set the instance ID of this cthread
    handle->set_instance_id( inst_id );

    // 12/22/2016 GL: add this cthread to m_all_proc
    m_all_proc.push_back( handle );
    m_oldest_time = sc_time();

    if ( m_ready_to_simulate ) 
    {
        handle->prepare_for_simulation();
    } else {
        m_process_table->push_front( handle );
    }
    return sc_process_handle(handle);
}




sc_process_handle 
sc_simcontext::create_method_process( 
    const char* name_p, bool free_host, SC_ENTRY_FUNC method_p,         
    sc_process_host* host_p, const sc_spawn_options* opt_p,
    int seg_id, int inst_id )
{

static int method_count = 0;
std::stringstream invok_strstr;
invok_strstr << "invoker" << method_count;
Invoker* new_invoker = new Invoker(invok_strstr.str().c_str());
m_invokers.push_back(new_invoker);
method_count++;

    sc_method_handle handle = 
        new sc_method_process(name_p, free_host, method_p, host_p, opt_p);

    ((sc_process_b*)handle)->m_trigger_type = sc_process_b::STATIC;

        method_to_invoker_map[ (sc_process_b*)handle ] = m_invokers[m_invokers.size() -1 ];

    // 08/17/2015 GL: set the starting segment ID of this thread
    handle->set_segment_id( seg_id );

    // 09/02/2015 GL: set the instance ID of this thread
    handle->set_instance_id( inst_id );

    // 12/22/2016 GL: add this thread to m_all_proc
    m_all_proc.push_back( handle );
    m_oldest_time = sc_time();

    if ( m_ready_to_simulate ) { // dynamic process
        handle->prepare_for_simulation();
        if ( !handle->dont_initialize() )
        {
#ifdef SC_HAS_PHASE_CALLBACKS_
            if( SC_UNLIKELY_( m_simulation_status
                            & (SC_END_OF_UPDATE|SC_BEFORE_TIMESTEP) ) )
            {
                std::stringstream msg;
                msg << m_simulation_status 
                    << ":\n\t immediate thread spawning of "
                       "`" << handle->name() << "' ignored";
                SC_REPORT_WARNING( SC_ID_PHASE_CALLBACK_FORBIDDEN_
                                 , msg.str().c_str() );
            }
            else
#endif // SC_HAS_PHASE_CALLBACKS_
            {
                //push_runnable_thread( handle );
        push_runnable_method( handle );

            }
        }
        else if ( handle->m_static_events.size() == 0 )
        {
            SC_REPORT_WARNING( SC_ID_DISABLE_WILL_ORPHAN_PROCESS_,
                               handle->name() );
        }

    } else {
        m_process_table->push_front( handle );
    }
    return sc_process_handle(handle);


/* DM 05/20/2019
    sc_method_handle handle = 
        new sc_method_process(name_p, free_host, method_p, host_p, opt_p);

    // 08/17/2015 GL: set the starting segment ID of this method
    handle->set_segment_id( seg_id );

    // 09/02/2015 GL: set the instance ID of this method
    handle->set_instance_id( inst_id );

    // 12/22/2016 GL: add this method to m_all_proc
    m_all_proc.push_back( handle );
    m_oldest_time = sc_time();

    if ( m_ready_to_simulate ) { // dynamic process
        // 11/13/2014 GL: create a coroutine for this method process
        handle->prepare_for_simulation();

        if ( !handle->dont_initialize() )
        {
#ifdef SC_HAS_PHASE_CALLBACKS_
            if( SC_UNLIKELY_( m_simulation_status
                            & (SC_END_OF_UPDATE|SC_BEFORE_TIMESTEP) ) )
            {
                std::stringstream msg;
                msg << m_simulation_status 
                    << ":\n\t immediate method spawning of "
                       "`" << handle->name() << "' ignored";
                SC_REPORT_WARNING( SC_ID_PHASE_CALLBACK_FORBIDDEN_
                                 , msg.str().c_str() );
            }
            else
#endif // SC_HAS_PHASE_CALLBACKS_
            {
                push_runnable_method( handle );
            }
        }
        else if ( handle->m_static_events.size() == 0 )
        {
            SC_REPORT_WARNING( SC_ID_DISABLE_WILL_ORPHAN_PROCESS_,
                               handle->name() );
        }

    } else {
        m_process_table->push_front( handle );
    }
    return sc_process_handle(handle);
*/
}


sc_process_handle 
sc_simcontext::create_thread_process( 
    const char* name_p, bool free_host, SC_ENTRY_FUNC method_p,         
    sc_process_host* host_p, const sc_spawn_options* opt_p,
    int seg_id, int inst_id )
{
    sc_thread_handle handle = 
        new sc_thread_process(name_p, free_host, method_p, host_p, opt_p);

    // 08/17/2015 GL: set the starting segment ID of this thread
    handle->set_segment_id( seg_id );

    // 09/02/2015 GL: set the instance ID of this thread
    handle->set_instance_id( inst_id );

    // 12/22/2016 GL: add this thread to m_all_proc
    m_all_proc.push_back( handle );
    m_oldest_time = sc_time();

    if ( m_ready_to_simulate ) { // dynamic process
        handle->prepare_for_simulation();
        if ( !handle->dont_initialize() )
        {
#ifdef SC_HAS_PHASE_CALLBACKS_
            if( SC_UNLIKELY_( m_simulation_status
                            & (SC_END_OF_UPDATE|SC_BEFORE_TIMESTEP) ) )
            {
                std::stringstream msg;
                msg << m_simulation_status 
                    << ":\n\t immediate thread spawning of "
                       "`" << handle->name() << "' ignored";
                SC_REPORT_WARNING( SC_ID_PHASE_CALLBACK_FORBIDDEN_
                                 , msg.str().c_str() );
            }
            else
#endif // SC_HAS_PHASE_CALLBACKS_
            {
                push_runnable_thread( handle );
            }
        }
        else if ( handle->m_static_events.size() == 0 )
        {
            SC_REPORT_WARNING( SC_ID_DISABLE_WILL_ORPHAN_PROCESS_,
                               handle->name() );
        }

    } else {
        m_process_table->push_front( handle );
    }
    return sc_process_handle(handle);
}

sc_process_handle 
sc_simcontext::create_invoker_process( 
    const char* name_p, bool free_host, SC_ENTRY_FUNC method_p,         
    sc_process_host* host_p, const sc_spawn_options* opt_p,
    int invoker_id )
{
    sc_thread_handle handle = 
        new sc_thread_process(name_p, free_host, method_p, host_p, opt_p);
    
    ((sc_process_b*)handle)->invoker = true;
    // 08/17/2015 GL: set the starting segment ID of this thread
    // 09/02/2015 GL: set the instance ID of this thread
    //handle->set_instance_id( invoker_id );

    // 12/22/2016 GL: add this thread to m_all_proc
    //m_all_proc.push_back( handle );
    //m_oldest_time = sc_time();

    return sc_process_handle(handle);
}


void
sc_simcontext::add_trace_file( sc_trace_file* tf )
{
    m_trace_files.push_back( tf );
    m_something_to_trace = true;
}

void
sc_simcontext::remove_trace_file( sc_trace_file* tf )
{
    m_trace_files.erase(
        std::remove( m_trace_files.begin(), m_trace_files.end(), tf )
    );
    m_something_to_trace = ( m_trace_files.size() > 0 );
}

sc_cor*
sc_simcontext::next_cor()
{
    if( m_error ) {
    return m_cor;
    }
    
    sc_thread_handle thread_h = pop_runnable_thread();
    while( thread_h != 0 ) {
    if ( thread_h->m_cor_p != NULL ) break;
    thread_h = pop_runnable_thread();
    }
    
    if( thread_h != 0 ) {
    return thread_h->m_cor_p;
    } else {
        // 05/25/2015 GL: sc_kernel_lock constructor acquires the kernel lock
        sc_kernel_lock lock;

#ifdef SC_LOCK_CHECK
        assert( is_locked_and_owner() );
#endif /* SC_LOCK_CHECK */
        if (m_curr_proc_queue.size() == 0) {
        return m_cor;
            // 05/25/2015 GL: sc_kernel_lock destructor releases the kernel 
            //                lock
        } else {
        return 0;
            // 05/25/2015 GL: sc_kernel_lock destructor releases the kernel 
            //                lock
        }
    }
}

const ::std::vector<sc_object*>&
sc_simcontext::get_child_objects() const
{
    static bool warn_get_child_objects=true;
    if ( warn_get_child_objects )
    {
    warn_get_child_objects = false;
    SC_REPORT_INFO(SC_ID_IEEE_1666_DEPRECATION_,
        "sc_simcontext::get_child_objects() is deprecated,\n" \
            " use sc_get_top_level_objects()" );
    }
    return m_child_objects;
}

void
sc_simcontext::add_child_event( sc_event* event_ )
{
    // no check if object_ is already in the set
    m_child_events.push_back( event_ );
}

void
sc_simcontext::add_child_object( sc_object* object_ )
{
    // no check if object_ is already in the set
    m_child_objects.push_back( object_ );
}

void
sc_simcontext::remove_child_event( sc_event* event_ )
{
    int size = m_child_events.size();
    for( int i = 0; i < size; ++ i ) {
    if( event_ == m_child_events[i] ) {
        m_child_events[i] = m_child_events[size - 1];
        m_child_events.resize(size-1);
        return;
    }
    }
    // no check if event_ is really in the set
}

void
sc_simcontext::remove_child_object( sc_object* object_ )
{
    int size = m_child_objects.size();
    for( int i = 0; i < size; ++ i ) {
    if( object_ == m_child_objects[i] ) {
        m_child_objects[i] = m_child_objects[size - 1];
        m_child_objects.resize(size-1);
        return;
    }
    }
    // no check if object_ is really in the set
}

sc_dt::uint64
sc_simcontext::delta_count() const
{
    static bool warn_delta_count=true;
    if ( warn_delta_count )
    {
    warn_delta_count = false;
    SC_REPORT_INFO(SC_ID_IEEE_1666_DEPRECATION_,
        "sc_simcontext::delta_count() is deprecated, use sc_delta_count()" );
    }
    //return m_delta_count;

    // 08/20/2015 GL: get the local delta count instead of the global one
    sc_process_b* proc = get_curr_proc();

    if ( proc )
        return proc->get_timestamp().get_delta_count();
    else
        return 0;
}

bool
sc_simcontext::is_running() const
{
    static bool warn_is_running=true;
    if ( warn_is_running )
    {
    warn_is_running = false;
    SC_REPORT_INFO(SC_ID_IEEE_1666_DEPRECATION_,
        "sc_simcontext::is_running() is deprecated, use sc_is_running()" );
    }
    return m_ready_to_simulate;
}

// +----------------------------------------------------------------------------
// |"sc_simcontext::next_time"
// | 
// | This method returns the time of the next event. If there are no events
// | it returns false.
// | 
// | Arguments:
// |     result = where to place time of the next event, if no event is 
// |              found this value will not be changed.
// | Result is true if an event is found, false if not.
// +----------------------------------------------------------------------------
bool
sc_simcontext::next_time( sc_time& result ) const
{
    while( m_timed_events->size() ) {
    sc_event_timed* et = m_timed_events->top();
    if( et->event() != 0 ) {
        result = et->notify_time();
        return true;
    }
    delete m_timed_events->extract_top();
    }
//DM 9/25/2018
sc_timestamp tmp_next_time(-1,-1);
if(m_paused_processes.size() != 0) {
for(std::vector<sc_process_b*>::const_iterator process_it  = m_paused_processes.begin();
            process_it != m_paused_processes.end();
            process_it++)
{
    if((*process_it)->get_timestamp() < tmp_next_time) {
        tmp_next_time = (*process_it)->get_timestamp();
    }
}
}
if(!tmp_next_time.get_infinite()) {
    result = tmp_next_time.m_time_count;
    return true;
}
else {
    return false;
}
}

void
sc_simcontext::remove_delta_event( sc_event* e )
{
    for(std::vector<sc_event*>::iterator it = m_delta_events.begin();
        it != m_delta_events.end();
        /* no need to put it++ here */)
    {
        if((*it) == e){
            it = m_delta_events.erase(it);
            break;
        } else {
            ++it;
        }
    }
    /*
    int i = e->m_delta_event_index;
    int j = m_delta_events.size() - 1;

    assert( i >= 0 && i <= j );
    if( i != j ) {
    sc_event** l_delta_events = &m_delta_events[0];
    l_delta_events[i] = l_delta_events[j];
    l_delta_events[i]->m_delta_event_index = i;
    }
    m_delta_events.resize(m_delta_events.size()-1);
    e->m_delta_event_index = -1;
    */
}

// +----------------------------------------------------------------------------
// |"sc_simcontext::preempt_with"
// | 
// | This method executes the supplied method immediately, suspending the
// | caller. After executing the supplied method the caller's execution will
// | be restored. It is used to allow a method to immediately throw an 
// | exception, e.g., when the method's kill_process() method was called.
// | There are three cases to consider:
// |   (1) The caller is a method, e.g., murder by method.
// |   (2) The caller is a thread instance, e.g., murder by thread.
// |   (3) The caller is this method instance, e.g., suicide.
// |
// | Arguments:
// |     method_h -> method to be executed.
// +----------------------------------------------------------------------------
void 
sc_simcontext::preempt_with( sc_method_handle method_h )
{
    assert( 0 ); // 10/28/2014 GL TODO: clean up the codes in the future
/*
    sc_curr_proc_info caller_info;     // process info for caller.
    sc_method_handle  active_method_h; // active method or null.
    sc_thread_handle  active_thread_h; // active thread or null.

    // Determine the active process and take the thread to be run off the
    // run queue, if its there, since we will be explicitly causing its 
    // execution.

    active_method_h = DCAST<sc_method_handle>(sc_get_current_process_b());
    active_thread_h = DCAST<sc_thread_handle>(sc_get_current_process_b());
    if ( method_h->next_runnable() != NULL )
    remove_runnable_method( method_h );

    // CALLER IS THE METHOD TO BE RUN:
    //
    // Should never get here, ignore it unless we are debugging.

    if ( method_h == active_method_h )
    {
        DEBUG_MSG(DEBUG_NAME,method_h,"self preemption of active method");
    }

    // THE CALLER IS A METHOD:
    //
    //   (a) Set the current process information to our method.
    //   (b) Invoke our method directly by-passing the run queue.
    //   (c) Restore the process info to the caller.
    //   (d) Check to see if the calling method should throw an exception
    //       because of activity that occurred during the preemption.

    else if ( active_method_h != NULL )
    {
    caller_info = m_curr_proc_info;
        DEBUG_MSG( DEBUG_NAME, method_h,
               "preempting active method with method" );
    sc_get_curr_simcontext()->set_curr_proc( (sc_process_b*)method_h );
    method_h->run_process();
    sc_get_curr_simcontext()->set_curr_proc((sc_process_b*)active_method_h);
    active_method_h->check_for_throws();
    }

    // CALLER IS A THREAD:
    //
    //   (a) Use an invocation thread to execute the method.

    else if ( active_thread_h != NULL )
    {
        DEBUG_MSG( DEBUG_NAME, method_h,
               "preempting active thread with method" );
    m_method_invoker_p->invoke_method(method_h);
    }

    // CALLER IS THE SIMULATOR:
    //
    // That is not allowed.

    else
    {
    caller_info = m_curr_proc_info;
        DEBUG_MSG( DEBUG_NAME, method_h,
               "preempting no active process with method" );
    sc_get_curr_simcontext()->set_curr_proc( (sc_process_b*)method_h );
    method_h->run_process();
    m_curr_proc_info = caller_info;
    }*/
}

//------------------------------------------------------------------------------
//"sc_simcontext::requeue_current_process"
//
// This method requeues the current process at the beginning of the run queue
// if it is a thread. This is called by sc_process_handle::throw_it() to assure
// that a thread that is issuing a throw will execute immediately after the
// processes it notifies via the throw.
//------------------------------------------------------------------------------
void sc_simcontext::requeue_current_process()
{
    sc_thread_handle thread_p;
    thread_p = DCAST<sc_thread_handle>(get_curr_proc());
    if ( thread_p )
    {
    execute_thread_next( thread_p );
    }
}

//------------------------------------------------------------------------------
//"sc_simcontext::suspend_current_process"
//
// This method suspends the current process if it is a thread. This is called 
// by sc_process_handle::throw_it() to allow the processes that have received
// a throw to execute.
//------------------------------------------------------------------------------
void sc_simcontext::suspend_current_process()
{
    sc_thread_handle thread_p;
    thread_p = DCAST<sc_thread_handle>(get_curr_proc());
    if ( thread_p )
    {
    thread_p->suspend_me(); 
    }
}

void
sc_simcontext::trace_cycle( bool delta_cycle )
{
//DM 4/12/2018
#ifdef SC_LOCK_CHECK
    assert( is_locked_and_owner() );
#endif /* SC_LOCK_CHECK */

    int size;
    if( ( size = m_trace_files.size() ) != 0 ) {
    sc_trace_file** l_trace_files = &m_trace_files[0];
    int i = size - 1;
    do {
        l_trace_files[i]->cycle( delta_cycle );
    } while( -- i >= 0 );
    }
}

// ----------------------------------------------------------------------------

#if 1
#ifdef PURIFY
    static sc_simcontext sc_default_global_context;
    sc_simcontext* sc_curr_simcontext = &sc_default_global_context;
#else
    sc_simcontext* sc_curr_simcontext = 0;
    sc_simcontext* sc_default_global_context = 0;
#endif
#else
// Not MT-safe!
static sc_simcontext* sc_curr_simcontext = 0;


sc_simcontext*
sc_get_curr_simcontext()
{
    if( sc_curr_simcontext == 0 ) {
#ifdef PURIFY
        static sc_simcontext sc_default_global_context;
        sc_curr_simcontext = &sc_default_global_context;
#else
        static sc_simcontext* sc_default_global_context = new sc_simcontext;
        sc_curr_simcontext = sc_default_global_context;
#endif
    }
    return sc_curr_simcontext;
}
#endif // 0

// Generates unique names within each module.

const char*
sc_gen_unique_name( const char* basename_, bool preserve_first )
{
    sc_simcontext* simc = sc_get_curr_simcontext();
    sc_module* curr_module = simc->hierarchy_curr();
    if( curr_module != 0 ) {
    return curr_module->gen_unique_name( basename_, preserve_first );
    } else {
        sc_process_b* curr_proc_p = sc_get_current_process_b();
    if ( curr_proc_p )
    {
        return curr_proc_p->gen_unique_name( basename_, preserve_first );
    }
    else
    {
        return simc->gen_unique_name( basename_, preserve_first );
    }
    }
}

// Get a handle for the current process
//
// Note that this method should not be called if the current process is
// in the act of being deleted, it will mess up the reference count management
// of sc_process_b instance the handle represents. Instead, use the a 
// pointer to the raw sc_process_b instance, which may be acquired via
// sc_get_current_process_b().

sc_process_handle
sc_get_current_process_handle()
{
    return ( sc_is_running() ) ?
    sc_process_handle(sc_get_current_process_b()) : 
    sc_get_last_created_process_handle();
}

// THE FOLLOWING FUNCTION IS DEPRECATED IN 2.1
sc_process_b*
sc_get_curr_process_handle()
{
    static bool warn=true;
    if ( warn )
    {
        warn = false;
        SC_REPORT_INFO(SC_ID_IEEE_1666_DEPRECATION_,
       "sc_get_curr_process_handle deprecated use sc_get_current_process_handle"
       );
    }

    return sc_get_curr_simcontext()->get_curr_proc();
}

// Return indication if there are more processes to execute in this delta phase

bool
sc_simcontext::pending_activity_at_current_time() const
{
    //std::cout << "in pending_activity_at_current_time, "
    //        << "testing a bug reported by DM" << std::endl;
    //std::cout << "m_delta_events.size()= " <<  m_delta_events.size() << std::endl;
    /*std::cout << "m_runnable->is_initialized() && !m_runnable->is_empty()= " 
        <<  (m_runnable->is_initialized() && !m_runnable->is_empty()) << std::endl;
    std::cout << "m_prim_channel_registry->pending_updates()= " 
        <<  m_prim_channel_registry->pending_updates()<< std::endl;
    */

    //if any events can trigger any thread, then return true
    //however, this is only for use of pending_activity_at_current_time 
    //in sc_main,
    //for other use, the check shall be more strict
    //such that, we need to see if the event_time is larger or equal to
    //the thread_time
    // if(m_delta_events.size() != 0){
    //     for(std::vector<sc_event*>::const_iterator event_it=m_delta_events.begin();
    //         event_it != m_delta_events.end();
    //         event_it ++)
    //     {
    //        if((*event_it)->m_threads_dynamic.size() > 0) //some thread can wake up
    //         {
    //             return true;
    //         }
    //     }
    // }
    //DM 9/25/2018  
    if(m_paused_processes.size() != 0) {
    for(std::vector<sc_process_b*>::const_iterator
            process_it  = m_paused_processes.begin();
            process_it != m_paused_processes.end();
            process_it ++)
        {
        if( (*process_it)->get_timestamp().m_time_count <= m_simulation_duration.m_time_count) {
            return true;
        }
    }
    }
    
    
    //return false
    
    return //( m_delta_events.size() != 0) || //ZC thinks this is wrong
            /*(!m_paused_processes.empty()) ||*/
           event_notification_update || ( m_runnable->is_initialized() && !m_runnable->is_empty() ) ||
           m_prim_channel_registry->pending_updates();
}

// Return time of next activity.

sc_time sc_time_to_pending_activity( const sc_simcontext* simc_p ) 
{
    // If there is an activity pending at the current time
    // return a delta of zero.
    
    sc_time result=SC_ZERO_TIME; // time of pending activity.

    if ( simc_p->pending_activity_at_current_time() )
    {
        return result;
    }

    // Any activity will take place in the future pick up the next event's time.

    else
    {
        result = simc_p->max_time();
        simc_p->next_time(result);
        result -= sc_time_stamp();
    }
    return result;
}

// Set the random seed for controlled randomization -- not yet implemented

void
sc_set_random_seed( unsigned int )
{
    SC_REPORT_WARNING( SC_ID_NOT_IMPLEMENTED_,
               "void sc_set_random_seed( unsigned int )" );
}


// +----------------------------------------------------------------------------
// |"set_number_sim_cpus"
// | 
// | This function sets _SYSC_NUM_SIM_CPUs, the number of simulation cores.
// |
// | (04/06/2015 GL)
// +----------------------------------------------------------------------------
void set_number_sim_cpus()
{
    const char* sim_cpus_var = _SYSC_PAR_SIM_CPUS_ENV_VAR;
    char* sim_cpus_str;

    sim_cpus_str = getenv( sim_cpus_var );

    if ( sim_cpus_str )
    {
        // 04/06/2015 GL: why not using atoi()? Because it does not handle 
        //                errors (out-of-range?!) properly
        unsigned int len = strlen( sim_cpus_str );
        unsigned int i = 0;
        _SYSC_NUM_SIM_CPUs = 0;
        while ( len )
        {
            if ( sim_cpus_str[i] >= 0x30 && sim_cpus_str[i] <= 0x39 )
            {
                _SYSC_NUM_SIM_CPUs = _SYSC_NUM_SIM_CPUs * 10 + sim_cpus_str[i]
                                      - 0x30;
                len --;
                i ++;
            }
            else
            {
#ifdef _SYSC_DEFAULT_PAR_SIM_CPUS
                _SYSC_NUM_SIM_CPUs = _SYSC_DEFAULT_PAR_SIM_CPUS;
#else
                _SYSC_NUM_SIM_CPUs = 64;
#endif
                break;
            }
        }
    }
    else
    {
#ifdef _SYSC_DEFAULT_PAR_SIM_CPUS
        _SYSC_NUM_SIM_CPUs = _SYSC_DEFAULT_PAR_SIM_CPUS;
#else
        _SYSC_NUM_SIM_CPUs = 64;
#endif
    }
}

// +----------------------------------------------------------------------------
// |"enable_synch_par_sim"
// | 
// | This function enables _SYSC_SYNC_PAR_SIM, synchronized parallel simulation,
// | depending on the environmental variable _SYSC_SYNC_PAR_SIM_ENV_VAR.
// |
// | (06/16/2016 GL)
// +----------------------------------------------------------------------------
void enable_synch_par_sim()
{
    const char* sync_par_sim_var = _SYSC_SYNC_PAR_SIM_ENV_VAR;
    char* sync_par_sim_str;

    sync_par_sim_str = getenv( sync_par_sim_var );

    if ( sync_par_sim_str )
    {
        if ( sync_par_sim_str )
            _SYSC_SYNC_PAR_SIM = true;
    }
}


// +----------------------------------------------------------------------------
// |"set_run_ahead_max"
// | 
// | This function sets _SYSC_RUN_AHEAD_MAX, the maximum run-ahead time interval.
// |
// | (12/22/2016 GL)
// +----------------------------------------------------------------------------

/*
void set_run_ahead_max()
{
    const char* run_ahead_max_var = _SYSC_RUN_AHEAD_MAX_ENV_VAR;
    char* run_ahead_max_str;

    run_ahead_max_str = getenv( run_ahead_max_var );

    if ( run_ahead_max_str )
    {
        unsigned int len = strlen( run_ahead_max_str );
        unsigned int i = 0;
        double run_ahead_max_in_ms = 0;
        bool decimal = false;
        double fraction = 0.1;

        while ( len )
        {
            if ( run_ahead_max_str[i] == 0x2E ) {
                if ( decimal == true ) {
#ifdef _SYSC_DEFAULT_RUN_AHEAD_MAX_IN_MS
                    run_ahead_max_in_ms = _SYSC_DEFAULT_RUN_AHEAD_MAX_IN_MS;
#else
                    run_ahead_max_in_ms = 1000;
#endif
                    break;
                } else {
                    decimal = true;
                }

                len --;
                i ++;
            } else if ( run_ahead_max_str[i] >= 0x30 && 
                        run_ahead_max_str[i] <= 0x39 ) {
                if ( decimal == true ) {
                    run_ahead_max_in_ms = run_ahead_max_in_ms + 
                        ( run_ahead_max_str[i] - 0x30 ) * fraction;
                    fraction /= 10;
                } else {
                    run_ahead_max_in_ms = run_ahead_max_in_ms * 10 +
                        run_ahead_max_str[i] - 0x30;
                }

                len --;
                i ++;
            } else {
#ifdef _SYSC_DEFAULT_RUN_AHEAD_MAX_IN_MS
                run_ahead_max_in_ms = _SYSC_DEFAULT_RUN_AHEAD_MAX_IN_MS;
#else
                run_ahead_max_in_ms = 1000;
#endif
                break;
            }
        }

        _SYSC_RUN_AHEAD_MAX = sc_time( run_ahead_max_in_ms, SC_MS );
    }
    else
    {
#ifdef _SYSC_DEFAULT_RUN_AHEAD_MAX_IN_MS
        _SYSC_RUN_AHEAD_MAX = sc_time( _SYSC_DEFAULT_RUN_AHEAD_MAX_IN_MS, SC_MS );
#else
        _SYSC_RUN_AHEAD_MAX = sc_time( 1000, SC_MS );
#endif
    }
}
*/

#if 0
// +----------------------------------------------------------------------------
// |"sc_start"
// | 
// | This function starts, or restarts, the execution of the simulator.
// |
// | Arguments:
// |     duration = the amount of time the simulator should execute.
// |     p        = event starvation policy.
// +----------------------------------------------------------------------------
void
sc_start( const sc_time& duration, sc_starvation_policy p )
{
    
    sc_simcontext* context_p;      // current simulation context.
    sc_time        entry_time;     // simulation time upon entry.
    sc_time        exit_time;      // simulation time to set upon exit.
    sc_dt::uint64  starting_delta; // delta count upon entry.
    int            status;         // current simulation status.

    set_number_sim_cpus(); // 04/06/2015 GL: set the number of simulation cores
#ifdef DEBUG_SYSTEMC
    printf( "The maximum number of concurrent pthreads is %d.\n", 
            _SYSC_NUM_SIM_CPUs );
#endif

    // Set up based on the arguments passed to us:

    context_p = sc_get_curr_simcontext();
    starting_delta = sc_delta_count();
    entry_time = context_p->m_curr_time;
    if ( p == SC_RUN_TO_TIME )
        exit_time = context_p->m_curr_time + duration;

    // called with duration = SC_ZERO_TIME for the first time
    static bool init_delta_or_pending_updates =
         ( starting_delta == 0 && exit_time == SC_ZERO_TIME );

    // If the simulation status is bad issue the appropriate message:

    status = context_p->sim_status();
    if( status != SC_SIM_OK ) 
    {
        if ( status == SC_SIM_USER_STOP )
            SC_REPORT_ERROR(SC_ID_SIMULATION_START_AFTER_STOP_, "");
        if ( status == SC_SIM_ERROR )
            SC_REPORT_ERROR(SC_ID_SIMULATION_START_AFTER_ERROR_, "");
        return;
    }

    if ( context_p->m_prim_channel_registry->pending_updates() )
        init_delta_or_pending_updates = true;

    // If the simulation status is good perform the simulation:

    context_p->simulate( duration );

    // Re-check the status:

    status = context_p->sim_status();

    // Update the current time to the exit time if that is the starvation
    // policy:

    if ( p == SC_RUN_TO_TIME && !context_p->m_paused && status == SC_SIM_OK )
    {
        context_p->m_curr_time = exit_time;
    }

    // If there was no activity and the simulation clock did not move warn
    // the user, except if we're in a first sc_start(SC_ZERO_TIME) for
    // initialisation (only) or there have been pending updates:

    if ( !init_delta_or_pending_updates &&
         starting_delta == sc_delta_count() &&
         context_p->m_curr_time == entry_time &&
         status == SC_SIM_OK )
    {
        SC_REPORT_WARNING(SC_ID_NO_SC_START_ACTIVITY_, "");
    }

    // reset init/update flag for subsequent calls
    init_delta_or_pending_updates = false;
}
#endif
// +----------------------------------------------------------------------------
// |"sc_start"
// | 
// | This function starts the execution of the simulator. This is a simply 
// | version, and we need to refine it in the future.
// |
// | Arguments:
// |     duration = the amount of time the simulator should execute.
// |     p        = event starvation policy.
// |
// | (08/20/2015 GL)
// +----------------------------------------------------------------------------
void
sc_start( const sc_time& duration, sc_starvation_policy p )
{
    /*
        when sc_starts start, there should NOT be events 
        which are registerd in the previous sc_start in the event queue
        
        for example, the duration is 10;
        5 threads, th1, th2, th3, th4, th5
        th5 is always running at time 1
        th3 waits for e1 on time 2
        th4 waits for e2 on time 3

        th1 notifies e1 on time 5,
        th2 notifies e2 on time 6.
        
        th1 waits for 10, goes to paused
        th2 waits for 10, goes to paused
        
        in event queue, there are e1:5, e2:6

        THEN:
        th5 wait for 10, 
        time_earliest_running_ready_thread becomes 11
        e1 is notified, th3 wake up, goes to ready queue.
        Now,
        in event queue, there is e2:6

        th3 will be running, and for no reason brings another oooschdule
        and e2 will get a chance to be triggered as well.

        (ZC)
    */

    sc_simcontext* context_p;      // current simulation context.
    int            status;         // current simulation status.
    static bool is_first_simulation = true;
    //static int count = 1;
    //bool show_log = false;

    // if(show_log){
    //     if(_SYSC_SYNC_PAR_SIM==true)
    //         std::cout << "in SYNC mode" << std::endl;
    //     else
    //         std::cout << "in OoO mode" << std::endl;
    // }
    if(is_first_simulation){
        set_number_sim_cpus(); // 04/06/2015 GL: set the number of simulation cores
        enable_synch_par_sim(); // 06/16/2016 GL: enable or disable synchronized 
                                //                parallel simulation
        //set_run_ahead_max(); // 12/22/2016 GL: set the maximum run-ahead time 
                             //                interval
        
        if(getenv("SYSC_PRINT_MODE_MESSAGE")) {
            
                
                
                if((!getenv("SYSC_SYNC_PAR_SIM"))&&_SYSC_SYNC_PAR_SIM==true) {
                    //printf("***     Synchronous Simulation is auto-enabled        ***\n");
                    printf("***%-66s     ***\n","     RISC simulator mode: synchronous parallel (auto-enabled)");
                }
                
            
        }
    #ifdef DEBUG_SYSTEMC
        ::std::cout << "The maximum number of concurrent pthreads is " 
                    << _SYSC_NUM_SIM_CPUs << ::std::endl;
        ::std::cout << "The state of synchronized parallel simulation is "
                    << ( _SYSC_SYNC_PAR_SIM ? "on" : "off" ) << ::std::endl;
        //::std::cout << "The maximum run-ahead time interval is "
        //            << _SYSC_RUN_AHEAD_MAX << ::std::endl;
    #endif
    }   
    // Set up based on the arguments passed to us:

    context_p = sc_get_curr_simcontext();
    context_p->m_starvation_policy = p;
    if(duration == SC_ZERO_TIME)
    {
        context_p->m_simulation_duration.m_time_count  = context_p->m_simulation_time.m_time_count;
        context_p->m_simulation_duration.m_delta_count = context_p->m_simulation_time.m_delta_count + 1;
    }
    else
    {
        context_p->m_simulation_duration.m_time_count  = context_p->m_simulation_time.m_time_count + duration;
        context_p->m_simulation_duration.m_delta_count = 0;
    }

    // If the simulation status is bad issue the appropriate message:

    status = context_p->sim_status();
    if( status != SC_SIM_OK ) 
    { 
        if ( status == SC_SIM_USER_STOP )
            SC_REPORT_ERROR(SC_ID_SIMULATION_START_AFTER_STOP_, "");
        if ( status == SC_SIM_ERROR )
            SC_REPORT_ERROR(SC_ID_SIMULATION_START_AFTER_ERROR_, "");
        return;
    }


    // If the simulation status is good perform the simulation:
    context_p->simulate( duration );
    is_first_simulation = false;
    // Re-check the status:


    if (p == SC_EXIT_ON_STARVATION)	// (09/19/19, RD)
    {   // we likely didn't reach the target time,
	// but we don't need to do anything
	assert(context_p->m_simulation_duration >= context_p->m_simulation_time);
    }
    else // SC_RUN_TO_TIME
    {   // we are asked to reach the target time,
	// so we need to increase the simulation time
	context_p->m_simulation_time = context_p->m_simulation_duration;
    }
}

void
sc_start()	// (RD: no argument implies maximum duration with SC_EXIT_ON_STARVATION)
{
    sc_start( sc_max_time() - sc_time_stamp(),
              SC_EXIT_ON_STARVATION );
}

// for backward compatibility with 1.0
#if 0
void
sc_start( double duration )  // in default time units
{
    
    static bool warn_sc_start=true;
    if ( warn_sc_start )
    {
    warn_sc_start = false;
    SC_REPORT_INFO(SC_ID_IEEE_1666_DEPRECATION_,
        "sc_start(double) deprecated, use sc_start(sc_time) or sc_start()");
    }

    if( duration == -1 )  // simulate forever
    {
        sc_start( 
            sc_time(~sc_dt::UINT64_ZERO, false) - sc_time_stamp() );
    }
    else
    {
        sc_start( sc_time( duration, true ) );
    }
}
#endif // 

void
sc_stop()
{
    sc_get_curr_simcontext()->stop();
}


// The following function is deprecated in favor of sc_start(SC_ZERO_TIME):

void
sc_initialize()
{
    static bool warning_initialize = true;

    if ( warning_initialize )
    {
        warning_initialize = false;
        SC_REPORT_INFO(SC_ID_IEEE_1666_DEPRECATION_,
        "sc_initialize() is deprecated: use sc_start(SC_ZERO_TIME)" );
    }
    sc_get_curr_simcontext()->initialize();
}

// The following function has been deprecated in favor of sc_start(duration):

void
sc_cycle( const sc_time& duration )
{
    static bool warning_cycle = true;

    if ( warning_cycle )
    {
        warning_cycle = false;
        SC_REPORT_INFO(SC_ID_IEEE_1666_DEPRECATION_,
        "sc_cycle is deprecated: use sc_start(sc_time)" );
    }
    sc_get_curr_simcontext()->cycle( duration );
}

sc_event* sc_find_event( const char* name )
{
    return sc_get_curr_simcontext()->get_object_manager()->find_event( name );
}

sc_object* sc_find_object( const char* name )
{
    return sc_get_curr_simcontext()->get_object_manager()->find_object( name );
}


const sc_time&
sc_max_time()
{
    return sc_get_curr_simcontext()->max_time();
}

const sc_time&
sc_time_stamp()
{
    return sc_get_curr_simcontext()->time_stamp();
}

const sc_time&
sc_simcontext::time_stamp()
{
    //return m_curr_time;

    // 08/20/2015 GL: get the local timed count instead of the global one
    sc_process_b* proc = get_curr_proc();
    if ( proc )
        return proc->get_timestamp().get_time_count();

    // root thread
    return m_simulation_time.m_time_count;
}

double
sc_simulation_time()
{
    static bool warn_simulation_time=true;
    if ( warn_simulation_time )
    {
        warn_simulation_time=false;
        SC_REPORT_INFO(SC_ID_IEEE_1666_DEPRECATION_,
        "sc_simulation_time() is deprecated use sc_time_stamp()" );
    }
    return sc_get_curr_simcontext()->time_stamp().to_default_time_units();
}

void
sc_defunct_process_function( sc_module* )
{
    // This function is pointed to by defunct sc_thread_process'es and
    // sc_cthread_process'es. In a correctly constructed world, this
    // function should never be called; hence the assert.
    assert( false );
}

//------------------------------------------------------------------------------
//"sc_set_stop_mode"
//
// This function sets the mode of operation when sc_stop() is called.
//     mode = SC_STOP_IMMEDIATE or SC_STOP_FINISH_DELTA.
//------------------------------------------------------------------------------
void sc_set_stop_mode(sc_stop_mode mode)
{
    if ( sc_is_running() )
    {
        SC_REPORT_ERROR(SC_ID_STOP_MODE_AFTER_START_,"");
    }
    else
    {
        switch( mode )
        {
          case SC_STOP_IMMEDIATE:
          case SC_STOP_FINISH_DELTA:
              stop_mode = mode;
              break;
          default:
              break;
        }
    }
}

sc_stop_mode
sc_get_stop_mode()
{
    return stop_mode;
}

bool sc_is_unwinding()
{ 
    return sc_get_current_process_handle().is_unwinding();
}

// The IEEE 1666 Standard for 2011 designates that the treatment of
// certain process control interactions as being "implementation dependent".
// These interactions are:
//   (1) What happens when a resume() call is performed on a disabled, 
//       suspended process.
//   (2) What happens when sync_reset_on() or sync_reset_off() is called
//       on a suspended process.
//   (3) What happens when the value specified in a reset_signal_is()
//       call changes value while a process is suspended.
//
// By default this Proof of Concept implementation reports an error
// for these interactions. However, the implementation also provides
// a non-error treatment. The non-error treatment for the interactions is:
//   (1) A resume() call performed on a disabled, suspended process will
//       mark the process as no longer suspended, and if it is capable
//       of execution (not waiting on any events) it will be placed on
//       the queue of runnable processes. See the state diagram below.
//   (2) A call to sync_reset_on() or sync_reset_off() will set or clear
//       the synchronous reset flag. Whether the process is in reset or
//       not will be determined when the process actually executes by
//       looking at the flag's value at that time.
//   (3) If a suspended process has a reset_signal_is() specification
//       the value of the reset variable at the time of its next execution 
//       will determine whether it is in reset or not.
//      
// TO GET THE NON-ERROR BEHAVIOR SET THE VARIABLE BELOW TO TRUE.
//
// This can be done in this source before you build the library, or you
// can use an assignment as the first statement in your sc_main() function:
//    sc_core::sc_allow_process_control_corners = true;

bool sc_allow_process_control_corners = false;

// The state transition diagram for the interaction of disable and suspend
// when sc_allow_process_control_corners is true is shown below:
//
// ......................................................................
// .         ENABLED                    .           DISABLED            .
// .                                    .                               .
// .                 +----------+    disable      +----------+          .
// .   +------------>|          |-------.-------->|          |          .
// .   |             | runnable |       .         | runnable |          .
// .   |     +-------|          |<------.---------|          |------+   .
// .   |     |       +----------+     enable      +----------+      |   .
// .   |     |          |    ^          .            |    ^         |   .
// .   |     |  suspend |    | resume   .    suspend |    | resume  |   .
// .   |     |          V    |          .            V    |         |   .
// .   |     |       +----------+    disable      +----------+      |   .
// .   |     |       | suspend  |-------.-------->| suspend  |      |   .
// . t |   r |       |          |       .         |          |      | r .
// . r |   u |       |  ready   |<------.---------|  ready   |      | u .
// . i |   n |       +----------+     enable      +----------+      | n .
// . g |   / |         ^                .                           | / .
// . g |   w |  trigger|                .                           | w .
// . e |   a |         |                .                           | a .
// . r |   i |       +----------+    disable      +----------+      | i .
// .   |   t |       | suspend  |-------.-------->| suspend  |      | t .
// .   |     |       |          |       .         |          |      |   .
// .   |     |       | waiting  |<------.---------| waiting  |      |   .
// .   |     |       +----------+     enable      +----------+      |   .
// .   |     |          |    ^          .            |    ^         |   .
// .   |     |  suspend |    | resume   .    suspend |    | resume  |   .
// .   |     |          V    |          .            V    |         |   .
// .   |     |       +----------+    disable      +----------+      |   .
// .   |     +------>|          |-------.-------->|          |      |   .
// .   |             | waiting  |       .         | waiting  |      |   .
// .   +-------------|          |<------.---------|          |<-----+   .
// .                 +----------+     enable      +----------+          .
// .                                    .                               .
// ......................................................................

// ----------------------------------------------------------------------------

static std::ostream&
print_status_expression( std::ostream& os, sc_status s );

// utility helper to print a simulation status
std::ostream& operator << ( std::ostream& os, sc_status s )
{
    // print primitive values
    switch(s)
    {
#   define PRINT_STATUS( Status ) \
      case Status: { os << #Status; } break

      PRINT_STATUS( SC_UNITIALIZED );
      PRINT_STATUS( SC_ELABORATION );
      PRINT_STATUS( SC_BEFORE_END_OF_ELABORATION );
      PRINT_STATUS( SC_END_OF_ELABORATION );
      PRINT_STATUS( SC_START_OF_SIMULATION );

      PRINT_STATUS( SC_RUNNING );
      PRINT_STATUS( SC_PAUSED );
      PRINT_STATUS( SC_STOPPED );
      PRINT_STATUS( SC_END_OF_SIMULATION );

      PRINT_STATUS( SC_END_OF_INITIALIZATION );
//      PRINT_STATUS( SC_END_OF_EVALUATION );
      PRINT_STATUS( SC_END_OF_UPDATE );
      PRINT_STATUS( SC_BEFORE_TIMESTEP );

      PRINT_STATUS( SC_STATUS_ANY );

#   undef PRINT_STATUS
    default:

      if( s & SC_STATUS_ANY ) // combination of status bits
        print_status_expression( os, s );
      else                    // invalid number, print hex value
        os << "0x" << std::hex << +s;
    }

    return os;
}

// pretty-print a combination of sc_status bits (i.e. a callback mask)
static std::ostream&
print_status_expression( std::ostream& os, sc_status s )
{
    std::vector<sc_status> bits;
    unsigned               is_set = SC_ELABORATION;

    // collect bits
    while( is_set <= SC_STATUS_LAST )
    {
        if( s & is_set )
            bits.push_back( (sc_status)is_set );
        is_set <<= 1;
    }
    if( s & ~SC_STATUS_ANY ) // remaining bits
        bits.push_back( (sc_status)( s & ~SC_STATUS_ANY ) );

    // print expression
    std::vector<sc_status>::size_type i=0, n=bits.size();
    if ( n>1 )
        os << "(";
    for( ; i<n-1; ++i )
        os << bits[i] << "|";
    os << bits[i];
    if ( n>1 )
        os << ")";
    return os;
}

}

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date: Ali Dasdan, Synopsys, Inc.
  Description of Modification: - Added sc_stop() detection into initial_crunch
                                 and crunch. This makes it possible to exit out
                                 of a combinational loop using sc_stop().

      Name, Affiliation, Date: Andy Goodrich, Forte Design Systems 20 May 2003
  Description of Modification: - sc_stop mode
                               - phase callbacks

      Name, Affiliation, Date: Bishnupriya Bhattacharya, Cadence Design Systems,
                               25 August 2003
  Description of Modification: - support for dynamic process
                               - support for sc export registry
                               - new member methods elaborate(), 
                 prepare_to_simulate(), and initial_crunch()
                 that are invoked by initialize() in that order
                               - implement sc_get_last_created_process_handle() for use
                                 before simulation starts
                               - remove "set_curr_proc(handle)" from 
                                 register_method_process and 
                                 register_thread_process - led to bugs
                               
      Name, Affiliation, Date: Andy Goodrich, Forte Design Systems 04 Sep 2003
  Description of Modification: - changed process existence structures to
                 linked lists to eliminate exponential 
                 execution problem with using sc_pvector.
 *****************************************************************************/
// $Log: sc_simcontext.cpp,v $
// Revision 1.37  2011/08/29 18:04:32  acg
//  Philipp A. Hartmann: miscellaneous clean ups.
//
// Revision 1.36  2011/08/26 20:46:10  acg
//  Andy Goodrich: moved the modification log to the end of the file to
//  eliminate source line number skew when check-ins are done.
//
// Revision 1.35  2011/08/24 22:05:51  acg
//  Torsten Maehne: initialization changes to remove warnings.
//
// Revision 1.34  2011/08/04 17:15:28  acg
//  Andy Goodrich: added documentation to crunch() routine.
//
// Revision 1.32  2011/07/24 11:16:36  acg
//  Philipp A. Hartmann: fix reference counting on deferred deletions of
//  processes.
//
// Revision 1.31  2011/07/01 18:49:07  acg
//  Andy Goodrich: moved pln() from sc_simcontext.cpp to sc_ver.cpp.
//
// Revision 1.30  2011/05/09 04:07:49  acg
//  Philipp A. Hartmann:
//    (1) Restore hierarchy in all phase callbacks.
//    (2) Ensure calls to before_end_of_elaboration.
//
// Revision 1.29  2011/04/08 22:39:09  acg
//  Andy Goodrich: moved method invocation code to sc_method.h so that the
//  details are hidden from sc_simcontext.
//
// Revision 1.28  2011/04/05 20:50:57  acg
//  Andy Goodrich:
//    (1) changes to make sure that event(), posedge() and negedge() only
//        return true if the clock has not moved.
//    (2) fixes for method self-resumes.
//    (3) added SC_PRERELEASE_VERSION
//    (4) removed kernel events from the object hierarchy, added
//        sc_hierarchy_name_exists().
//
// Revision 1.27  2011/04/05 06:14:15  acg
//  Andy Goodrich: fix typo.
//
// Revision 1.26  2011/04/05 06:03:32  acg
//  Philipp A. Hartmann: added code to set ready to run bit for a suspended
//  process that does not have dont_initialize specified at simulation
//  start up.
//
// Revision 1.25  2011/04/01 21:31:55  acg
//  Andy Goodrich: make sure processes suspended before the start of execution
//  don't get scheduled for initial execution.
//
// Revision 1.24  2011/03/28 13:02:52  acg
//  Andy Goodrich: Changes for disable() interactions.
//
// Revision 1.23  2011/03/12 21:07:51  acg
//  Andy Goodrich: changes to kernel generated event support.
//
// Revision 1.22  2011/03/07 17:38:43  acg
//  Andy Goodrich: tightening up of checks for undefined interaction between
//  synchronous reset and suspend.
//
// Revision 1.21  2011/03/06 19:57:11  acg
//  Andy Goodrich: refinements for the illegal suspend - synchronous reset
//  interaction.
//
// Revision 1.20  2011/03/06 15:58:50  acg
//  Andy Goodrich: added escape to turn off process control corner case
//  checks.
//
// Revision 1.19  2011/03/05 04:45:16  acg
//  Andy Goodrich: moved active process calculation to the sc_simcontext class.
//
// Revision 1.18  2011/03/05 01:39:21  acg
//  Andy Goodrich: changes for named events.
//
// Revision 1.17  2011/02/18 20:27:14  acg
//  Andy Goodrich: Updated Copyrights.
//
// Revision 1.16  2011/02/17 19:53:28  acg
//  Andy Goodrich: eliminated use of ready_to_run() as part of process control
//  simplification.
//
// Revision 1.15  2011/02/13 21:47:38  acg
//  Andy Goodrich: update copyright notice.
//
// Revision 1.14  2011/02/11 13:25:24  acg
//  Andy Goodrich: Philipp A. Hartmann's changes:
//    (1) Removal of SC_CTHREAD method overloads.
//    (2) New exception processing code.
//
// Revision 1.13  2011/02/08 08:42:50  acg
//  Andy Goodrich: fix ordering of check for stopped versus paused.
//
// Revision 1.12  2011/02/07 19:17:20  acg
//  Andy Goodrich: changes for IEEE 1666 compatibility.
//
// Revision 1.11  2011/02/02 07:18:11  acg
//  Andy Goodrich: removed toggle() calls for the new crunch() toggle usage.
//
// Revision 1.10  2011/02/01 23:01:53  acg
//  Andy Goodrich: removed dead code.
//
// Revision 1.9  2011/02/01 21:11:59  acg
//  Andy Goodrich:
//  (1) Use of new toggle_methods() and toggle_threads() run queue methods
//      to make sure the thread run queue does not execute when allow preempt_me()
//      is called from an SC_METHOD.
//  (2) Use of execute_thread_next() to allow thread execution in the current
//      delta cycle() rather than push_runnable_thread_front which executed
//      in the following cycle.
//
// Revision 1.8  2011/01/25 20:50:37  acg
//  Andy Goodrich: changes for IEEE 1666 2011.
//
// Revision 1.7  2011/01/19 23:21:50  acg
//  Andy Goodrich: changes for IEEE 1666 2011
//
// Revision 1.6  2011/01/18 20:10:45  acg
//  Andy Goodrich: changes for IEEE1666_2011 semantics.
//
// Revision 1.5  2010/11/20 17:10:57  acg
//  Andy Goodrich: reset processing changes for new IEEE 1666 standard.
//
// Revision 1.4  2010/07/22 20:02:33  acg
//  Andy Goodrich: bug fixes.
//
// Revision 1.3  2008/05/22 17:06:26  acg
//  Andy Goodrich: updated copyright notice to include 2008.
//
// Revision 1.2  2007/09/20 20:32:35  acg
//  Andy Goodrich: changes to the semantics of throw_it() to match the
//  specification. A call to throw_it() will immediately suspend the calling
//  thread until all the throwees have executed. At that point the calling
//  thread will be restarted before the execution of any other threads.
//
// Revision 1.1.1.1  2006/12/15 20:20:05  acg
// SystemC 2.3
//
// Revision 1.21  2006/08/29 23:37:13  acg
//  Andy Goodrich: Added check for negative time.
//
// Revision 1.20  2006/05/26 20:33:16  acg
//   Andy Goodrich: changes required by additional platform compilers (i.e.,
//   Microsoft VC++, Sun Forte, HP aCC).
//
// Revision 1.19  2006/05/08 17:59:52  acg
//  Andy Goodrich: added a check before m_curr_time is set to make sure it
//  is not set to a time before its current value. This will treat
//  sc_event.notify( ) calls with negative times as calls with a zero time.
//
// Revision 1.18  2006/04/20 17:08:17  acg
//  Andy Goodrich: 3.0 style process changes.
//
// Revision 1.17  2006/04/11 23:13:21  acg
//   Andy Goodrich: Changes for reduced reset support that only includes
//   sc_cthread, but has preliminary hooks for expanding to method and thread
//   processes also.
//
// Revision 1.16  2006/03/21 00:00:34  acg
//   Andy Goodrich: changed name of sc_get_current_process_base() to be
//   sc_get_current_process_b() since its returning an sc_process_b instance.
//
// Revision 1.15  2006/03/13 20:26:50  acg
//  Andy Goodrich: Addition of forward class declarations, e.g.,
//  sc_reset, to keep gcc 4.x happy.
//
// Revision 1.14  2006/02/02 23:42:41  acg
//  Andy Goodrich: implemented a much better fix to the sc_event_finder
//  proliferation problem. This new version allocates only a single event
//  finder for each port for each type of event, e.g., pos(), neg(), and
//  value_change(). The event finder persists as long as the port does,
//  which is what the LRM dictates. Because only a single instance is
//  allocated for each event type per port there is not a potential
//  explosion of storage as was true in the 2.0.1/2.1 versions.
//
// Revision 1.13  2006/02/02 21:29:10  acg
//  Andy Goodrich: removed the call to sc_event_finder::free_instances() that
//  was in end_of_elaboration(), leaving only the call in clean(). This is
//  because the LRM states that sc_event_finder instances are persistent as
//  long as the sc_module hierarchy is valid.
//
// Revision 1.12  2006/02/02 21:09:50  acg
//  Andy Goodrich: added call to sc_event_finder::free_instances in the clean()
//  method.
//
// Revision 1.11  2006/02/02 20:43:14  acg
//  Andy Goodrich: Added an existence linked list to sc_event_finder so that
//  the dynamically allocated instances can be freed after port binding
//  completes. This replaces the individual deletions in ~sc_bind_ef, as these
//  caused an exception if an sc_event_finder instance was used more than
//  once, due to a double freeing of the instance.
//
// Revision 1.10  2006/01/31 21:43:26  acg
//  Andy Goodrich: added comments in constructor to highlight environmental
//  overrides section.
//
// Revision 1.9  2006/01/26 21:04:54  acg
//  Andy Goodrich: deprecation message changes and additional messages.
//
// Revision 1.8  2006/01/25 00:31:19  acg
//  Andy Goodrich: Changed over to use a standard message id of
//  SC_ID_IEEE_1666_DEPRECATION for all deprecation messages.
//
// Revision 1.7  2006/01/24 20:49:05  acg
// Andy Goodrich: changes to remove the use of deprecated features within the
// simulator, and to issue warning messages when deprecated features are used.
//
// Revision 1.6  2006/01/19 00:29:52  acg
// Andy Goodrich: Yet another implementation for signal write checking. This
// one uses an environment variable SC_SIGNAL_WRITE_CHECK, that when set to
// DISABLE will disable write checking on signals.
//
// Revision 1.5  2006/01/13 18:44:30  acg
// Added $Log to record CVS changes into the source.
//
// Revision 1.4  2006/01/03 23:18:44  acg
// Changed copyright to include 2006.
//
// Revision 1.3  2005/12/20 22:11:10  acg
// Fixed $Log lines.
//
// Revision 1.2  2005/12/20 22:02:30  acg
// Changed where delta cycles are incremented to match IEEE 1666. Added the
// event_occurred() method to hide how delta cycle comparisions are done within
// sc_simcontext. Changed the boolean update_phase to an enum that shows all
// the phases.
// Taf!
