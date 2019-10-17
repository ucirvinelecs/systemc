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

  sc_simcontext.h -- Definition of the simulation context class.

  Original Author: Stan Y. Liao, Synopsys, Inc.
                   Martin Janssen, Synopsys, Inc.

  CHANGE LOG AT THE END OF THE FILE
 *****************************************************************************/

#ifndef SC_SIMCONTEXT_H
#define SC_SIMCONTEXT_H

#include "sysc/kernel/sc_cmnhdr.h"
#include "sysc/kernel/sc_process.h"
#include "sysc/kernel/sc_status.h"
#include "sysc/kernel/sc_time.h"
#include "sysc/utils/sc_hash.h"
#include "sysc/utils/sc_pq.h"
#include <map>
#include <unordered_map>
#include <list>
#include <string.h>
#include <time.h>

// 02/22/2016 ZC: to enable verbose display or not
#ifndef _SYSC_PRINT_VERBOSE_MESSAGE_ENV_VAR
#define _SYSC_PRINT_VERBOSE_MESSAGE_ENV_VAR "SYSC_PRINT_VERBOSE_MESSAGE"
#endif
namespace sc_core {

// forward declarations
class Invoker; //DM 05/16/2019

class sc_cor;
class sc_cor_pkg;
class sc_event;
class sc_event_timed;
class sc_export_registry;
class sc_module;
class sc_module_name;
class sc_module_registry;
class sc_name_gen;
class sc_object;
class sc_object_manager;
class sc_phase_callback_registry;
class sc_process_handle;
class sc_port_registry;
class sc_prim_channel_registry;
class sc_process_table;
class sc_signal_bool_deval;
class sc_trace_file;
class sc_runnable;
class sc_process_host;
class sc_method_process;
class sc_cthread_process;
class sc_thread_process;
 
template< typename > class sc_plist;
typedef sc_plist< sc_process_b* > sc_process_list;



enum sc_stop_mode {          // sc_stop modes:
    SC_STOP_FINISH_DELTA,
    SC_STOP_IMMEDIATE
};
extern void sc_set_stop_mode( sc_stop_mode mode );
extern sc_stop_mode sc_get_stop_mode();

enum sc_starvation_policy 
{
    SC_EXIT_ON_STARVATION,
    SC_RUN_TO_TIME
};
extern void sc_start();
extern void sc_start( const sc_time& duration, 
                      sc_starvation_policy p=SC_RUN_TO_TIME );
inline void sc_start( int duration, sc_time_unit unit, 
                      sc_starvation_policy p=SC_RUN_TO_TIME )
{
    sc_start( sc_time((double)duration,unit), p );
}

inline void sc_start( double duration, sc_time_unit unit, 
                      sc_starvation_policy p=SC_RUN_TO_TIME )
{
    sc_start( sc_time(duration,unit), p );
}

extern void sc_stop();

// 06/16/2016 GL: enable synchronized parallel simulation
extern bool _SYSC_SYNC_PAR_SIM;

// friend function declarations

sc_dt::uint64 sc_delta_count();
const std::vector<sc_event*>& sc_get_top_level_events(
        const   sc_simcontext* simc_p);
const std::vector<sc_object*>& sc_get_top_level_objects(
        const   sc_simcontext* simc_p);
bool    sc_is_running( const sc_simcontext* simc_p );
void    sc_pause();
bool    sc_end_of_simulation_invoked();
void    sc_start( const sc_time&, sc_starvation_policy );
bool    sc_start_of_simulation_invoked();
void    sc_set_time_resolution( double, sc_time_unit );
sc_time sc_get_time_resolution();
void    sc_set_default_time_unit( double, sc_time_unit );
sc_time sc_get_default_time_unit();
bool    sc_pending_activity_at_current_time( const sc_simcontext* );
bool    sc_pending_activity_at_future_time( const sc_simcontext* );
sc_time sc_time_to_pending_activity( const sc_simcontext* );

struct sc_invoke_method; 

/**
 *  \brief A scoped mutex for the kernel lock.
 */
// 05/22/2015 GL.
struct sc_kernel_lock {
    sc_simcontext* simc_p;
    sc_cor* m_cor_p;
    explicit sc_kernel_lock();
    ~sc_kernel_lock(); 
};

// 09/01/2019 ZC.
extern const char* _OoO_Table_File_Name;

// 09/01/2019 ZC.
// extern std::string _OoO_File_Name_;
// 02/21/2017 ZC.
extern unsigned int _OoO_Combined_Data_Conflict_Table_Size;

// 02/21/2017 ZC.
extern unsigned int _OoO_Combined_Data_Conflict_Lookup_Table_Number_Segments;

// 02/21/2017 ZC.
extern unsigned int _OoO_Combined_Data_Conflict_Lookup_Table_Max_Instances;

// 02/21/2017 ZC.
extern unsigned int _OoO_Prediction_Time_Advance_Table_Number_Steps;

// 02/21/2017 ZC.
extern unsigned int _OoO_Prediction_Time_Advance_Table_Number_Segments;

// 02/21/2017 ZC.
extern unsigned int _OoO_Prediction_Event_Notification_Table_Number_Segments;

// 02/21/2017 ZC.
extern unsigned int _OoO_Prediction_Event_Notification_Table_Max_Instances;

// 02/21/2017 ZC.
extern unsigned int _OoO_Prediction_Event_Notification_Table_No_Indirect_Number_Segments;

// 02/21/2017 ZC.
extern unsigned int _OoO_Prediction_Event_Notification_Table_No_Indirect_Max_Instances;


// 02/21/2017 ZC.
extern int _OoO_Combined_Data_Conflict_Table[];

// 02/21/2017 ZC.
extern int _OoO_Combined_Data_Conflict_Lookup_Table[];

// 02/21/2017 ZC.
extern long long _OoO_Prediction_Time_Advance_Table_Time_Units[];

// 02/21/2017 ZC.
extern int _OoO_Prediction_Time_Advance_Table_Delta[];

// 02/21/2017 ZC.
extern int _OoO_Prediction_Time_Advance_Lookup_Table[];

// 02/21/2017 ZC.
extern long long _OoO_Prediction_Event_Notification_Table_Time_Units[];

// 02/21/2017 ZC.
extern int _OoO_Prediction_Event_Notification_Table_Delta[];

// 02/21/2017 ZC.
extern long long _OoO_Prediction_Event_Notification_Table_No_Indirect_Time_Units[];

// 02/21/2017 ZC.
extern int _OoO_Prediction_Event_Notification_Table_No_Indirect_Delta[];

// 02/21/2017 ZC.
extern int _OoO_Prediction_Event_Notification_Lookup_Table[];

// 02/21/2017 ZC.
extern int _OoO_Prediction_Event_Notification_No_Indirect_Lookup_Table[];


/**
 *  \brief Data conflict table for the out-of-order simulation.
 */
// 08/17/2015 GL.
extern bool _OoO_Data_Conflict_Table[];

/**
 *  \brief Size of the data conflict table.
 */
// 08/17/2015 GL.
extern unsigned int _OoO_Data_Conflict_Table_Size;

/**
 *  \brief Event notification table for the out-of-order simulation.
 */
// 08/17/2015 GL.
extern bool _OoO_Event_Notify_Table[];

/**
 *  \brief Size of the event notification table.
 */
// 08/17/2015 GL.
extern unsigned int _OoO_Event_Notify_Table_Size;

/**
 *  \brief Index lookup table for the data conflict table and event
 *         notification table.
 */
// 09/01/2015 GL.
extern int _OoO_Conflict_Index_Lookup_Table[];

/**
 *  \brief The maximum number of instances.
 */
// 09/01/2015 GL.
extern unsigned int _OoO_Max_Number_of_Instances;

/**
 *  \brief The number of segments.
 */
// 09/01/2015 GL.
extern unsigned int _OoO_Number_of_Segments;

/**
 *  \brief Current time advance table (timed cycles) for the out-of-order 
 *         simulation.
 */
// 08/18/2015 GL.
extern long long _OoO_Curr_Time_Advance_Table_Time[];

/**
 *  \brief Current time advance table (delta cycles) for the out-of-order 
 *         simulation.
 */
// 08/18/2015 GL.
extern int _OoO_Curr_Time_Advance_Table_Delta[];

/**
 *  \brief Size of the current time advance table.
 */
// 08/18/2015 GL.
extern unsigned int _OoO_Curr_Time_Advance_Table_Size;

/**
 *  \brief Next time advance table (timed cycles) for the out-of-order 
 *         simulation.
 */
// 08/18/2015 GL.
extern long long _OoO_Next_Time_Advance_Table_Time[];

/**
 *  \brief Next time advance table (delta cycles) for the out-of-order 
 *         simulation.
 */
// 08/18/2015 GL.
extern int _OoO_Next_Time_Advance_Table_Delta[];

/**
 *  \brief Size of the next time advance table.
 */
// 08/18/2015 GL.
extern unsigned int _OoO_Next_Time_Advance_Table_Size;

/**
 *  \brief Index lookup table for the time advance table.
 */
// 09/01/2015 GL.
extern int _OoO_Time_Advance_Index_Lookup_Table[];

/**
 *  \brief Size of the index lookup table for the time advance table.
 */
// 09/01/2015 GL.
extern unsigned int _OoO_Time_Advance_Index_Lookup_Table_Size;

/**************************************************************************//**
 *  \class sc_segid
 *
 *  \brief segment id
 *         currently only used for sc_fifo::read(...) as a bug fix
 *****************************************************************************/
class sc_segid{
public:
  int seg_id;
  inline explicit sc_segid(int val):
    seg_id(val)
    {}
};
 
/**************************************************************************//**
 *  \class sc_simcontext
 *
 *  \brief The simulation context.
 *****************************************************************************/

class sc_simcontext
{
    //DM 05/16/2019
    friend class Invoker; 

    friend struct sc_invoke_method; 
    friend class sc_event;
    friend class sc_module;

    // 04/07/2015 GL: a new sc_channel class is derived from sc_module
    friend class sc_channel;

    friend class sc_object;
    friend class sc_time;
    friend class sc_clock;
    friend class sc_method_process;
    friend class sc_phase_callback_registry;
    friend class sc_process_b;
    friend class sc_process_handle;
    friend class sc_prim_channel;
    friend class sc_cthread_process;
    friend class sc_thread_process;
    friend class sc_runnable;
    friend sc_dt::uint64 sc_delta_count();
    friend const std::vector<sc_event*>& sc_get_top_level_events(
        const sc_simcontext* simc_p);
    friend const std::vector<sc_object*>& sc_get_top_level_objects(
        const sc_simcontext* simc_p);
    friend bool sc_is_running( const sc_simcontext* simc_p );
    friend void sc_pause();
    friend bool sc_end_of_simulation_invoked();
    friend void sc_start( const sc_time&, sc_starvation_policy );
    friend bool sc_start_of_simulation_invoked();
    friend void sc_thread_cor_fn(void*);
    friend void sc_method_cor_fn(void*);//DM 05/24/2019
    friend sc_time sc_time_to_pending_activity( const sc_simcontext* );
    friend bool sc_pending_activity_at_current_time( const sc_simcontext* );
    friend bool sc_pending_activity_at_future_time( const sc_simcontext* );

    // 05/25/2015 GL: a scoped mutex for the kernel lock
    friend struct sc_kernel_lock;

    // 05/26/2015 GL: get sc_cor pointer
    friend sc_cor* get_cor_pointer( sc_process_b* process_p ); 


    void init();
    void clean();
public:

    sc_simcontext();
    ~sc_simcontext();

    void initialize( bool = false );

    /**
     *  \brief This function is not supported by the out-of-order simulation in
     *         the current release.
     */
    // 08/20/2015 GL.
    void cycle( const sc_time& );

    void simulate( const sc_time& duration );
    void stop();
    void end();
    void reset();

    void print_threads_states();
    void print_events_states();
  
    // 11/05/2014 GL: thread mapping function used in parallel simulators
    //void mapper( sc_cor *cor );

    /**
     *  \fn schedule
     *
     *  \brief Scheduling function called by parallel threads.
     */
    // 05/18/2015 GL.
    //void schedule( sc_cor *cor );

    /**
     *  \brief Scheduling function in the OoO simulation.
     */
    // 08/12/2015 GL.
    void oooschedule( sc_cor *cor );

    int sim_status() const;
    bool elaboration_done() const;

    std::vector<sc_thread_handle>& get_active_invokers();
  
    sc_object_manager* get_object_manager();

    inline sc_status get_status() const;

    sc_object* active_object();

    void hierarchy_push( sc_module* );
    sc_module* hierarchy_pop();
    sc_module* hierarchy_curr() const;
    sc_object* first_object();
    sc_object* next_object();
    sc_object* find_object( const char* name );

    sc_module_registry* get_module_registry();
    sc_port_registry* get_port_registry();
    sc_export_registry* get_export_registry();
    sc_prim_channel_registry* get_prim_channel_registry();

    // to generate unique names for objects in an MT-Safe way
    const char* gen_unique_name( const char* basename_, 
                                 bool preserve_first = false 
                               );

    // process creation

    /**
     *  \brief Two new parameters segment ID and instance ID are added for the 
     *         out-of-order simulation.
     */
    // 06/12/2015 GL: modified for the OoO simulation
    // 09/02/2015 GL: set the instance id
    sc_process_handle create_cthread_process( 
    const char* name_p, bool free_host, SC_ENTRY_FUNC method_p, 
    sc_process_host* host_p, const sc_spawn_options* opt_p,
    int seg_id, int inst_id );

    /**
     *  \brief Two new parameters segment ID and instance ID are added for the 
     *         out-of-order simulation.
     */
    // 06/12/2015 GL: modified for the OoO simulation
    // 09/02/2015 GL: set the instance id
    sc_process_handle create_method_process( 
    const char* name_p, bool free_host, SC_ENTRY_FUNC method_p, 
    sc_process_host* host_p, const sc_spawn_options* opt_p,
    int seg_id, int inst_id );

    /**
     *  \brief Two new parameters segment ID and instance ID are added for the 
     *         out-of-order simulation.
     */
    // 06/12/2015 GL: modified for the OoO simulation
    // 09/02/2015 GL: set the instance id
    sc_process_handle create_thread_process( 
    const char* name_p, bool free_host, SC_ENTRY_FUNC method_p, 
    sc_process_host* host_p, const sc_spawn_options* opt_p,
    int seg_id, int inst_id );

private:
    //DM 05/15/2019 for method clustering
    sc_process_handle create_invoker_process( 
    const char* name_p, bool free_host, SC_ENTRY_FUNC method_p, 
    sc_process_host* host_p, const sc_spawn_options* opt_p,
    int invoker_id );

public:
    // 10/28/2014 GL: to be replaced by get_curr_proc
    //sc_curr_proc_handle get_curr_proc_info();

    sc_process_b* get_curr_proc() const;
    sc_object* get_current_writer() const;
    bool write_check() const;

    /**
     *  \brief This function is not supported by the out-of-order simulation in
     *         the current release.
     */
    // 10/29/2014 GL.
    void set_curr_proc( sc_process_b* );

    void reset_curr_proc();

    int next_proc_id();

    void add_trace_file( sc_trace_file* );
    void remove_trace_file( sc_trace_file* );

    friend void    sc_set_time_resolution( double, sc_time_unit );
    friend sc_time sc_get_time_resolution();
    friend void    sc_set_default_time_unit( double, sc_time_unit );
    friend sc_time sc_get_default_time_unit();

    const sc_time& max_time() const;

    // 08/19/2015 GL: obsolete, to be removed in the future
    const sc_time& time_stamp();

    sc_dt::uint64 change_stamp() const;

    // 08/19/2015 GL: obsolete, to be removed in the future
    sc_dt::uint64 delta_count() const;

    bool event_occurred( sc_dt::uint64 last_change_count ) const;
    bool evaluation_phase() const;
    bool is_running() const;
    bool update_phase() const;
    bool notify_phase() const;
    bool get_error();
    void set_error( sc_report* );

    sc_cor_pkg* cor_pkg()
        { return m_cor_pkg; }
    sc_cor* next_cor(); // 11/05/2014 GL: obsolete, to be removed in the future

    const ::std::vector<sc_object*>& get_child_objects() const;

    void elaborate();
    void prepare_to_simulate();

    /**
     *  \brief This function is partially supported by the out-of-order 
     *         simulation in the current release.
     */
    // 08/19/2015 GL.
    inline void initial_crunch( bool no_crunch );

    bool next_time( sc_time& t ) const; 
    bool pending_activity_at_current_time() const; 

    /**
     *  \brief Remove a process from the running queue.
     */
    // 11/05/2014 GL.
    void remove_running_process( sc_process_b* );

    /**
     *  \brief Check whether a process is in the running queue.
     */
    // 11/05/2014 GL.
    bool is_running_process( sc_process_b* );

    /**
     *  \brief Suspend a coroutine.
     */
    // 11/05/2014 GL.
    void suspend_cor( sc_cor* );

    /**
     *  \brief Resume a coroutine.
     */
    // 11/05/2014 GL.
    void resume_cor( sc_cor* );

    /**
     *  \brief Check whether the kernel lock is acquired.
     */
    // 04/29/2015 GL.
    bool is_locked();

    /**
     *  \brief Check whether the kernel lock is released.
     */
    // 04/29/2015 GL.
    bool is_unlocked();

    /**
     *  \brief Check whether the kernel lock is owned by the currently running 
     *         coroutine.
     */
    // 04/29/2015 GL.
    bool is_lock_owner();

    /**
     *  \brief Check whether the kernel lock is not owned by the currently 
     *         running coroutine.
     */
    // 04/29/2015 GL.
    bool is_not_owner();

    /**
     *  \brief Check whether the kernel lock is acquired and owned by the  
     *         currently running coroutine.
     */
    // 04/29/2015 GL.
    bool is_locked_and_owner();

    /**
     *  \brief Convert segment ID and instance ID to index in the data conflict
     *         table and event notification table.
     */
    // 09/02/2015 GL.
    unsigned int conflict_table_index_lookup( int, int );

    /**
     *  \brief Convert segment ID to index in the time advance table.
     */
    // 09/02/2015 GL.
    unsigned int time_adv_table_index_lookup( int );

private:
    void clean_up_old_event_notifications();
    void predict_wakeup_time_by_running_ready_threads(std::unordered_map<sc_process_b*, sc_timestamp>& wkup_t_prd_run_rdy);
    void predict_wakeup_time_by_events(std::unordered_map<sc_process_b*,
        std::map<sc_event*, sc_timestamp> >& wkup_t_evnt);
    void predict_wakeup_time_by_waiting_threads(
        std::unordered_map<sc_process_b*, sc_timestamp>& wkup_t_pred_and_evnt,
        std::unordered_map<sc_process_b*, std::map<sc_event*, sc_timestamp> >& wkup_t_evnt,
        std::unordered_map<sc_process_b*, sc_timestamp>& wkup_t_prd_run_rdy);
    bool check_and_deliver_events();


    void add_child_event( sc_event* );
    void add_child_object( sc_object* );
    void remove_child_event( sc_event* );
    void remove_child_object( sc_object* );

    //void crunch( bool once=false );

    int add_delta_event( sc_event* );
    void remove_delta_event( sc_event* );
    void add_timed_event( sc_event_timed* );

    void trace_cycle( bool delta_cycle );

    const ::std::vector<sc_event*>& get_child_events_internal() const;
    const ::std::vector<sc_object*>& get_child_objects_internal() const;

    void execute_method_next( sc_method_handle );
    void execute_thread_next( sc_thread_handle );

    sc_method_handle pop_runnable_method();
    sc_thread_handle pop_runnable_thread();

    /**
     *  \brief This function is not supported by the out-of-order simulation in
     *         the current release.
     */
    // 10/28/2014 GL.
    void preempt_with( sc_method_handle );

    /**
     *  \brief This function is not supported by the out-of-order simulation in
     *         the current release.
     */
    // 10/28/2014 GL.
    inline void preempt_with( sc_thread_handle );

    void push_runnable_method( sc_method_handle );
    void push_runnable_thread( sc_thread_handle );

    void push_runnable_method_front( sc_method_handle );
    void push_runnable_thread_front( sc_thread_handle );

    void remove_runnable_method( sc_method_handle );
    void remove_runnable_thread( sc_thread_handle );

    void requeue_current_process();
    void suspend_current_process();

    void do_sc_stop_action();
    void mark_to_collect_process( sc_process_b* zombie_p );

    /**
     *  \brief Acquire the kernel lock.
     */
    // 05/22/2015 GL.
    void acquire_sched_mutex();

    /**
     *  \brief Release the kernel lock.
     */
    // 05/22/2015 GL.
    void release_sched_mutex();

    /**
     *  \brief The dynamic conflict detection function.
     */
    // 08/17/2015 GL.
    bool has_no_conflicts( sc_process_b*, std::list<sc_method_handle>, std::list<sc_thread_handle> );

  /**
     *  \brief use index id to get segment id.
     */
    // 02/22/2017 ZC.
  //int reverse_event_prediction_index_lookup_seg(  int );
  
  /**
     *  \brief use index id to get instance id.
     */
    // 02/22/2017 ZC.
  //int reverse_event_prediction_index_lookup_inst(  int );
  
  /**
     *  \brief index lookup
     */
    // 02/22/2017 ZC. 
  int combined_data_conflict_table_index_lookup( int , int  );
  
  /**
     *  \brief index lookup
     */
    // 02/22/2017 ZC.
  int event_prediction_table_index_lookup( int , int  );
  
  /**
     *  \brief index lookup
     */
    // 02/22/2017 ZC. 
  int prediction_time_advance_table_index_lookup( int);
  
  /**
     *  \brief for recursion use in conflict_between_with_prediction()
     */
    // 02/22/2017 ZC.
  bool conflict_between_two_segs(
    int , 
    int , 
    int ,
    int ,
    sc_timestamp ,
    sc_timestamp );

  /**
     *  \brief Detect conflicts between two processes with prediction
     */
    // 02/22/2017 ZC.
  bool conflict_between_with_prediction( sc_process_b*, sc_process_b* );

  /**
     *  \brief add and remove process from wait queue
     */
    // 10:54 2017/3/10 ZC.
  void add_to_wait_queue( sc_process_b*) ;
  void remove_from_wait_queue(sc_process_b*) ;
  
    /**
     *  \brief Detect conflicts between two processes.
     */
    // 08/17/2015 GL.
    bool conflict_between( sc_process_b*, sc_process_b* );

    /**
     *  \brief Update the oldest timestamp in simulation.
     */
    // 12/22/2016 GL.
    void update_oldest_time( sc_time& curr_time );


private:

    enum execution_phases {
        phase_initialize = 0,
        phase_evaluate,
        phase_update,
        phase_notify
    };
    sc_object_manager*          m_object_manager;

    sc_module_registry*         m_module_registry;
    sc_port_registry*           m_port_registry;
    sc_export_registry*         m_export_registry;
    sc_prim_channel_registry*   m_prim_channel_registry;
    sc_phase_callback_registry* m_phase_cb_registry;
 
    sc_name_gen*                m_name_gen;

    sc_process_table*           m_process_table;

    // 10/22/2014 GL: to be removed
    //sc_curr_proc_info           m_curr_proc_info;

    std::list<sc_process_b*>    m_curr_proc_queue;

    // 10/22/2014 GL: to be replaced by m_curr_proc_queue.size()
    //int                         m_curr_proc_num;

    // 02/06/2015 GL: to be removed
    //sc_object*                  m_current_writer; 

    bool                        m_write_check;
    int                         m_next_proc_id;

    std::vector<sc_thread_handle> m_active_invokers;

    std::vector<sc_event*>      m_child_events;
    std::vector<sc_object*>     m_child_objects;

    std::vector<sc_event*>      m_delta_events;
    sc_ppq<sc_event_timed*>*    m_timed_events;
  
    std::vector<sc_trace_file*> m_trace_files;
    bool                        m_something_to_trace;
  
    sc_runnable*                m_runnable;
    sc_process_list*            m_collectable;

    sc_time_params*             m_time_params;

    // 08/17/2015 GL: no global time count
    //sc_time                     m_curr_time;

    mutable sc_time             m_max_time;
 
    sc_invoke_method*           m_method_invoker_p;
    sc_dt::uint64               m_change_stamp; // "time" change occurred.

    // 08/17/2015 GL: no global delta count
    //sc_dt::uint64               m_delta_count;

    bool                        m_forced_stop;
    bool                        m_paused;
    bool                        m_ready_to_simulate;
    bool                        m_elaboration_done;
    execution_phases            m_execution_phase;
    sc_report*                  m_error;
    bool                        m_in_simulator_control;   
    bool                        m_end_of_simulation_called;
    sc_status                   m_simulation_status;
    bool                        m_start_of_simulation_called;

    sc_cor_pkg*                 m_cor_pkg; // the simcontext's coroutine package
    sc_cor*                     m_cor;     // the simcontext's coroutine

    // 05/19/2015 GL: the following variables are used in 
    //                sc_simcontext::schedule
    bool                        m_one_delta_cycle; 
    bool                        m_one_timed_cycle;
    sc_time                     m_finish_time;
 
    
    // 12/22/2016 GL: the oldest timestamp in simulation
    
  
  long*           workload_table;
  
  long*           visits;

  long            old_sys_time;
  // 03/10/2017 ZC: wait queue
  std::list<sc_process_b*>    m_waiting_proc_queue;
  
private:
  
    // disabled
    sc_simcontext( const sc_simcontext& );
    sc_simcontext& operator = ( const sc_simcontext& );
  
public:
  //std::list<sc_thread_handle>    m_synch_thread_queue;
  std::list<sc_process_b*>    m_synch_thread_queue; //DM 05/27/2019 synch queue now includes both threads and methods

    // 12/22/2016 GL: a list of all threads and methods created in simulation
    std::list<sc_process_b*>    m_all_proc;
//4/10/2018 DM
private:
  sc_time oldest_untraced_time;

    // 18.8.6 ZC paused processes, which is for sc_start(time)
    std::vector<sc_process_b*>      m_paused_processes;
    
    
public:
  const sc_time& get_oldest_untraced_time();
    sc_time                         m_oldest_time;
    sc_timestamp                    m_simulation_duration;
    sc_timestamp                    m_simulation_time;
    sc_starvation_policy            m_starvation_policy;
    int                             last_seg_id;
    std::list<sc_process_b*>        m_waking_up_threads;

private:
//05/15/2019 DM TODO: use C++11 data structures
std::map<sc_process_b*,Invoker*> method_to_invoker_map;
std::vector<Invoker*> m_invokers;
std::set<Invoker*> ready_invokers;
std::set<Invoker*> running_invokers;
//bool check_and_deliver_events_now;
bool event_notification_update;
//DM 07/24/2019 sc_methods updates:
unsigned int running_methods;
};

// IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII

// Not MT safe.

#if 1
extern sc_simcontext* sc_curr_simcontext;
extern sc_simcontext* sc_default_global_context;

inline sc_simcontext*
sc_get_curr_simcontext()
{
    if( sc_curr_simcontext == 0 ) {
        sc_default_global_context = new sc_simcontext;
        sc_curr_simcontext = sc_default_global_context;
    }
    return sc_curr_simcontext;
}
#else
    extern sc_simcontext* sc_get_curr_simcontext();
#endif // 0
inline sc_status sc_get_status()
{
    return sc_get_curr_simcontext()->get_status();
}


// IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII

inline
bool
sc_simcontext::elaboration_done() const
{
    return m_elaboration_done;
}


inline sc_status sc_simcontext::get_status() const
{
    return m_simulation_status != SC_RUNNING ? 
                  m_simulation_status :
      (m_in_simulator_control ? SC_RUNNING : SC_PAUSED);
}

inline
int
sc_simcontext::sim_status() const
{
    if( m_error ) {
        return SC_SIM_ERROR;
    }
    if( m_forced_stop ) {
        return SC_SIM_USER_STOP;
    }
    return SC_SIM_OK;
}


inline
sc_object_manager*
sc_simcontext::get_object_manager()
{
    return m_object_manager;
}

inline
sc_module_registry*
sc_simcontext::get_module_registry()
{
    return m_module_registry;
}

inline
sc_port_registry*
sc_simcontext::get_port_registry()
{
    return m_port_registry;
}

inline
sc_export_registry*
sc_simcontext::get_export_registry()
{
    return m_export_registry;
}

inline
sc_prim_channel_registry*
sc_simcontext::get_prim_channel_registry()
{
    return m_prim_channel_registry;
}


inline
int
sc_simcontext::next_proc_id()
{
    return ( ++ m_next_proc_id );
}


inline
const sc_time&
sc_simcontext::max_time() const
{
    if ( m_max_time == SC_ZERO_TIME )
    {
//      current RISC implementation uses 'long long', not 'unsigned long long'
//      (this was a poor choice to allow for "special" values, not easy to change now)
//      m_max_time = sc_time::from_value( ~sc_dt::UINT64_ZERO );
        m_max_time = sc_time::from_value( LLONG_MAX - 0x02ff );
        // note: lowest 10 bits are lost due to 'double' conversion (08/23/19, RD)
//      printf("DEBUG: m_max_time.m_value = %lld\n", m_max_time.m_value);
    }
    return m_max_time;
}

inline
sc_dt::uint64
sc_simcontext::change_stamp() const
{
    return m_change_stamp;
}



inline 
bool
sc_simcontext::event_occurred(sc_dt::uint64 last_change_stamp) const
{
    return m_change_stamp == last_change_stamp;
}

inline
bool
sc_simcontext::evaluation_phase() const
{
    return (m_execution_phase == phase_evaluate) &&
           m_ready_to_simulate;
}

inline
bool
sc_simcontext::update_phase() const
{
    return m_execution_phase == phase_update;
}

inline
bool
sc_simcontext::notify_phase() const
{
    return m_execution_phase == phase_notify;
}

inline
void
sc_simcontext::set_error( sc_report* err )
{
    delete m_error;
    m_error = err;
}


inline
bool
sc_simcontext::get_error()
{
    return m_error != NULL;
}

inline
int
sc_simcontext::add_delta_event( sc_event* e )
{
    m_delta_events.push_back( e );
    return ( m_delta_events.size() - 1 );
}

inline
void
sc_simcontext::add_timed_event( sc_event_timed* et )
{
    m_timed_events->insert( et );
}

inline sc_object* 
sc_simcontext::get_current_writer() const
{
    // 02/06/2015 GL: m_current_writer are different in different threads
    return m_write_check ? get_curr_proc() : (sc_object*)0;
}

inline bool 
sc_simcontext::write_check() const
{
    return m_write_check;
}

inline void
sc_simcontext::remove_running_process( sc_process_b* proc )
{
    m_curr_proc_queue.remove( proc );
}

inline bool
sc_simcontext::is_running_process( sc_process_b* proc )
{
    for ( std::list<sc_process_b*>::iterator it = m_curr_proc_queue.begin(); 
          it != m_curr_proc_queue.end(); it++ )
        if ( *it == proc )
            return true;
    return false;
}


// ----------------------------------------------------------------------------

class sc_process_handle;
sc_process_handle sc_get_current_process_handle();

// Get the current object hierarchy context
//
// Returns a pointer the the sc_object (module or process) that
// would become the parent object of a newly created element
// of the SystemC object hierarchy, or NULL.
//
inline sc_object*
sc_get_current_object()
{
  return sc_get_curr_simcontext()->active_object();
}

inline
sc_process_b*
sc_get_current_process_b()
{
    return sc_get_curr_simcontext()->get_curr_proc();
}

// THE FOLLOWING FUNCTION IS DEPRECATED IN 2.1
extern sc_process_b* sc_get_curr_process_handle();

inline
sc_curr_proc_kind
sc_get_curr_process_kind()
{
    return sc_get_curr_simcontext()->get_curr_proc()->proc_kind();
}


inline int sc_get_simulator_status()
{
    return sc_get_curr_simcontext()->sim_status();
}


// Generates unique names within each module.
extern
const char*
sc_gen_unique_name( const char* basename_, bool preserve_first = false );


// Set the random seed for controlled randomization -- not yet implemented
extern
void
sc_set_random_seed( unsigned int seed_ );


extern void sc_initialize();

extern const sc_time& sc_max_time();    // Get maximum time value.
extern const sc_time& sc_time_stamp();  // Current simulation time.
extern double sc_simulation_time();     // Current time in default time units.
//4/10/2018 DM
extern const sc_time& get_current_trace_time();

inline
const std::vector<sc_event*>& sc_get_top_level_events(
    const sc_simcontext* simc_p = sc_get_curr_simcontext() )
{
    return simc_p->m_child_events;
}

inline
const std::vector<sc_object*>& sc_get_top_level_objects(
    const sc_simcontext* simc_p = sc_get_curr_simcontext() )
{
    return simc_p->m_child_objects;
}

extern sc_event* sc_find_event( const char* name );

extern sc_object* sc_find_object( const char* name );

/**
 *  \brief This function returns the local delta count of the running process.
 */
inline
sc_dt::uint64 sc_delta_count()
{
    //return sc_get_curr_simcontext()->m_delta_count;

    // 08/20/2015 GL: get the local delta count instead of the global one
    sc_process_b* proc = sc_get_current_process_b();

    if ( proc )
        return proc->get_timestamp().get_delta_count();
    else
        return 0;
}

inline 
bool sc_is_running( const sc_simcontext* simc_p = sc_get_curr_simcontext() )
{
    return simc_p->m_ready_to_simulate;
}

bool sc_is_unwinding();

inline void sc_pause()
{
    sc_get_curr_simcontext()->m_paused = true;
}

// Return indication if there are more processes to execute in this delta phase

inline bool sc_pending_activity_at_current_time
  ( const sc_simcontext* simc_p = sc_get_curr_simcontext() )
{
  return simc_p->pending_activity_at_current_time();
}

// Return indication if there are timed notifications in the future

inline bool sc_pending_activity_at_future_time
  ( const sc_simcontext* simc_p = sc_get_curr_simcontext() )
{
  sc_time ignored;
  return simc_p->next_time( ignored );
}

// Return indication if there are processes to run,
// or notifications in the future

inline bool sc_pending_activity
  ( const sc_simcontext* simc_p = sc_get_curr_simcontext() )
{
  return sc_pending_activity_at_current_time( simc_p )
      || sc_pending_activity_at_future_time( simc_p );
}

sc_time
sc_time_to_pending_activity
  ( const sc_simcontext* simc_p = sc_get_curr_simcontext() );


inline
bool
sc_end_of_simulation_invoked()
{
    return sc_get_curr_simcontext()->m_end_of_simulation_called;
}

inline bool sc_hierarchical_name_exists( const char* name )
{
    return sc_find_object(name) || sc_find_event(name);
}

inline
bool 
sc_start_of_simulation_invoked()
{
    return sc_get_curr_simcontext()->m_start_of_simulation_called;
}

// The following variable controls whether process control corners should
// be considered errors or not. See sc_simcontext.cpp for details on what
// happens if this value is set to true.

extern bool sc_allow_process_control_corners;

} // namespace sc_core

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date: Andy Goodrich, Forte Design Systems 20 May 2003
  Description of Modification: - phase callbacks
                               - sc_stop mode

      Name, Affiliation, Date: Bishnupriya Bhattacharya, Cadence Design Systems,
                               25 August, 2003
  Description of Modification: - support for dynamic process
                               - support for sc export registry
                               - new member methods elaborate(), 
                 prepare_to_simulate(), and initial_crunch()
                 that are invoked by initialize() in that order
                               - add sc_get_last_created_process_handle() for 
                 use before simulation starts
                               
      Name, Affiliation, Date: Bishnupriya Bhattacharya, Cadence Design Systems,
                               3 March, 2004
  Description of Modification: add sc_get_curr_process_kind()

      Name, Affiliation, Date: 
  Description of Modification: 
                               
 *****************************************************************************/
// $Log: sc_simcontext.h,v $
// Revision 1.26  2011/09/05 21:20:22  acg
//  Andy Goodrich: result of automake invocation.
//
// Revision 1.25  2011/09/01 15:28:10  acg
//  Andy Goodrich: the aftermath of automake.
//
// Revision 1.24  2011/08/29 18:04:32  acg
//  Philipp A. Hartmann: miscellaneous clean ups.
//
// Revision 1.23  2011/08/26 20:46:10  acg
//  Andy Goodrich: moved the modification log to the end of the file to
//  eliminate source line number skew when check-ins are done.
//
// Revision 1.22  2011/08/24 22:05:51  acg
//  Torsten Maehne: initialization changes to remove warnings.
//
// Revision 1.21  2011/05/09 04:07:49  acg
//  Philipp A. Hartmann:
//    (1) Restore hierarchy in all phase callbacks.
//    (2) Ensure calls to before_end_of_elaboration.
//
// Revision 1.20  2011/04/08 18:26:07  acg
//  Andy Goodrich: added execute_method_next() to handle method dispatch
//   for asynchronous notifications that occur outside the evaluation phase.
//
// Revision 1.19  2011/04/05 20:50:57  acg
//  Andy Goodrich:
//    (1) changes to make sure that event(), posedge() and negedge() only
//        return true if the clock has not moved.
//    (2) fixes for method self-resumes.
//    (3) added SC_PRERELEASE_VERSION
//    (4) removed kernel events from the object hierarchy, added
//        sc_hierarchical_name_exists().
//
// Revision 1.18  2011/03/20 13:43:23  acg
//  Andy Goodrich: added async_signal_is() plus suspend() as a corner case.
//
// Revision 1.17  2011/03/07 18:25:19  acg
//  Andy Goodrich: tightening of check for resume on a disabled process to
//  only produce an error if it is ready to run.
//
// Revision 1.16  2011/03/06 15:58:50  acg
//  Andy Goodrich: added escape to turn off process control corner case
//  checks.
//
// Revision 1.15  2011/03/05 04:45:16  acg
//  Andy Goodrich: moved active process calculation to the sc_simcontext class.
//
// Revision 1.14  2011/03/05 01:39:21  acg
//  Andy Goodrich: changes for named events.
//
// Revision 1.13  2011/02/18 20:27:14  acg
//  Andy Goodrich: Updated Copyrights.
//
// Revision 1.12  2011/02/13 21:47:38  acg
//  Andy Goodrich: update copyright notice.
//
// Revision 1.11  2011/02/13 21:34:35  acg
//  Andy Goodrich: added SC_UNITIALIZED enum value to process status so
//  its possible to detect throws before initialization.
//
// Revision 1.10  2011/02/11 13:25:24  acg
//  Andy Goodrich: Philipp A. Hartmann's changes:
//    (1) Removal of SC_CTHREAD method overloads.
//    (2) New exception processing code.
//
// Revision 1.9  2011/02/01 21:18:56  acg
//  Andy Goodrich: addition of new preempt_with() method used to immediately
//  throw exceptions from threads.
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
// Revision 1.5  2010/07/22 20:02:33  acg
//  Andy Goodrich: bug fixes.
//
// Revision 1.4  2009/05/22 16:06:29  acg
//  Andy Goodrich: process control updates.
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
// Revision 1.13  2006/05/08 18:00:06  acg
// Andy Goodrich: added David Long's forward declarations for friend
//   functions, methods, and operators to keep the Microsoft compiler happy.
//
// Revision 1.11  2006/04/11 23:13:21  acg
//   Andy Goodrich: Changes for reduced reset support that only includes
//   sc_cthread, but has preliminary hooks for expanding to method and thread
//   processes also.
//
// Revision 1.10  2006/03/21 00:00:34  acg
//   Andy Goodrich: changed name of sc_get_current_process_base() to be
//   sc_get_current_process_b() since its returning an sc_process_b instance.
//
// Revision 1.9  2006/01/26 21:04:54  acg
//  Andy Goodrich: deprecation message changes and additional messages.
//
// Revision 1.8  2006/01/24 20:49:05  acg
// Andy Goodrich: changes to remove the use of deprecated features within the
// simulator, and to issue warning messages when deprecated features are used.
//
// Revision 1.7  2006/01/19 00:29:52  acg
// Andy Goodrich: Yet another implementation for signal write checking. This
// one uses an environment variable SC_SIGNAL_WRITE_CHECK, that when set to
// DISABLE will disable write checking on signals.
//
// Revision 1.6  2006/01/18 21:42:37  acg
// Andy Goodrich: Changes for check writer support.
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

#endif
