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

  sc_spawn_options.cpp -- Process spawning options implementation.

  Original Authors: Andy Goodrich, Forte Design Systems, 17 June 2003
                    Stuart Swan, Cadence,
                    Bishnupriya Bhattacharya, Cadence Design Systems,
                    25 August, 2003

  CHANGE LOG AT THE END OF THE FILE
 *****************************************************************************/

#include "sysc/kernel/sc_spawn_options.h"
#include "sysc/kernel/sc_reset.h"
//-------------------------------------------------Farah is working here

sc_core::sc_spawn_options::sc_spawn_options() :
        m_dont_initialize(false), m_resets(), m_sensitive_events(),
        m_sensitive_event_finders(), m_sensitive_interfaces(),
        m_sensitive_port_bases(), m_spawn_method(false), m_stack_size(0)
        { }


void sc_core::sc_spawn_options::dont_initialize()   { m_dont_initialize = true; }

bool sc_core::sc_spawn_options::is_method() const   { return m_spawn_method; }

void sc_core::sc_spawn_options::set_stack_size(int stack_size) { m_stack_size = stack_size; }

void sc_core::sc_spawn_options::set_sensitivity(const sc_event* event)
    { m_sensitive_events.push_back(event); }

void sc_core::sc_spawn_options::set_sensitivity(sc_port_base* port_base)
    { m_sensitive_port_bases.push_back(port_base); }

void sc_core::sc_spawn_options::set_sensitivity(sc_interface* interface_p)
    { m_sensitive_interfaces.push_back(interface_p); }

void sc_core::sc_spawn_options::set_sensitivity(sc_export_base* export_base)
    { m_sensitive_interfaces.push_back(export_base->get_interface()); }

void sc_core::sc_spawn_options::set_sensitivity(sc_event_finder* event_finder)
    { m_sensitive_event_finders.push_back(event_finder); }

void sc_core::sc_spawn_options::spawn_method() { m_spawn_method = true; }




//-------------------------------------------------Farah is done working here
namespace sc_core {

// +======================================================================
// | CLASS sc_spawn_reset_base - Class to do a generic access to an
// |                             sc_spawn_rest object instance
// +======================================================================
class sc_spawn_reset_base
{
  public:
    sc_spawn_reset_base( bool async, bool level )
      : m_async( async ), m_level(level)
    {}
    virtual ~sc_spawn_reset_base() {}
    virtual void specify_reset() = 0;

  protected:
    bool m_async;   // = true if async reset.
    bool m_level;   // level indicating reset.
};

// +======================================================================
// | CLASS sc_spawn_reset<SOURCE>
// |  - Reset specification for sc_spawn_options.
// +======================================================================
template<typename SOURCE>
class sc_spawn_reset : public sc_spawn_reset_base
{
  public:
    sc_spawn_reset( bool async, const SOURCE& source, bool level )
      : sc_spawn_reset_base(async, level), m_source(source)
    {}
    virtual ~sc_spawn_reset() {}
    virtual void specify_reset()
    {
        sc_reset::reset_signal_is( m_async, m_source, m_level );
    }

  protected:
    const SOURCE& m_source; // source of reset signal.
};

// +======================================================================
// | CLASS sc_spawn_options (implementation)
// |
// +======================================================================

sc_spawn_options::~sc_spawn_options()
{
    std::vector<sc_spawn_reset_base*>::size_type resets_n = m_resets.size();
    for ( std::vector<sc_spawn_reset_base*>::size_type reset_i = 0; reset_i < resets_n; reset_i++ )
        delete m_resets[reset_i];
}

#define SC_DEFINE_RESET_SIGNALS( Port )                       \
  /* asynchronous reset */                                    \
  void                                                        \
  sc_spawn_options::                                          \
    async_reset_signal_is ( const Port & port, bool level )   \
    {                                                         \
        m_resets.push_back(                                   \
            new sc_spawn_reset< Port >(true, port, level) );  \
    }                                                         \
  /* sync reset */                                            \
  void                                                        \
  sc_spawn_options::                                          \
    reset_signal_is ( const Port & port, bool level )         \
    {                                                         \
        m_resets.push_back(                                   \
            new sc_spawn_reset< Port >(false, port, level) ); \
    }

SC_DEFINE_RESET_SIGNALS( sc_in<bool> )
SC_DEFINE_RESET_SIGNALS( sc_inout<bool> )
SC_DEFINE_RESET_SIGNALS( sc_out<bool> )
SC_DEFINE_RESET_SIGNALS( sc_signal_in_if<bool> )

#undef SC_DEFINE_RESET_SIGNALS

void
sc_spawn_options::specify_resets() const
{
    std::vector<sc_spawn_reset_base*>::size_type resets_n; // number of reset specifications to process.
    resets_n = m_resets.size();
    for ( std::vector<sc_spawn_reset_base*>::size_type reset_i = 0; reset_i < resets_n; reset_i++ )
        m_resets[reset_i]->specify_reset();
}

} // namespace sc_core

// Taf!
