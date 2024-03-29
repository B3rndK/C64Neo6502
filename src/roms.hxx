#ifndef _ROMS
#define _ROMS

#include "roms/chargen.hxx"
#include "roms/basic.hxx"
#include "roms/kernal.hxx"
#ifdef _MONITOR_CARTRIDGE
#include "roms/monitor.hxx"
#endif
#ifdef _FLASHDANCE
#include "roms/flashdance.hxx"
#endif
#ifdef _SIMONS_BASIC
#include "roms/simons_basic.hxx"
#include "roms/apfel_0801.hxx"
#endif
#ifdef _TRAPDOOR
#include "roms/trapdoor.hxx"
#endif
#ifdef _ELITE
#include "roms/elite.hxx" 
#endif
#ifdef _WIZBALL
#include "roms/wizball.hxx"
#endif
#ifdef _SYNTH_SAMPLE
#include "roms/synth_sample.hxx"
#endif
#endif