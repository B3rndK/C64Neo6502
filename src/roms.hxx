#ifndef _ROMS
#define _ROMS

#include "roms/chargen.hxx"
#include "roms/basic.hxx"
#include "roms/kernal.hxx"

/* Current development playground (not yet working 100%) */
#ifdef _FLASHDANCE
#include "roms/flashdance.hxx"
#endif
#ifdef _ELITE
#include "roms/elite.hxx" 
#endif
#ifdef _MERCENARY
#include "roms/mercenary.hxx" 
#endif
#ifdef _WIZBALL
#include "roms/wizball.hxx"
#endif
#ifdef _SYNTH_SAMPLE
#include "roms/synth_sample.hxx"
#endif
#ifdef _HOBBIT 
#include "roms/hobbit.hxx"
#endif
#ifdef _CMASTER 
#include "roms/cmaster.hxx"
#endif

/* Working/Tested (Excerpt) */
#ifdef _MONITOR_CARTRIDGE
#include "roms/monitor.hxx"
#endif
#ifdef _SIMONS_BASIC
#include "roms/simons_basic.hxx"
#include "roms/apfel_0801.hxx"
#endif
#ifdef _TRAPDOOR
#include "roms/trapdoor.hxx"
#endif
#ifdef _PULSAR7
#include "roms/pulsar7.hxx"
#endif
#ifdef _FAIRLIGHT
#include "roms/fairlight.hxx"
#endif
#ifdef _NIGHTSHADE
#include "roms/nightshade.hxx"
#endif
#ifdef _LOM // Lords of Midnight
#include "roms/lom.hxx"
#endif
#ifdef _LOMII // Doomdark's revenge
#include "roms/lomii.hxx"
#endif
#ifdef _COLOSSUS // Colossus Chess 4.0
#include "roms/colossus.hxx"
#endif

#endif