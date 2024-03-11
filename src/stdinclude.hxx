#ifndef _STD_INCLUDE
#define _STD_INCLUDE

#include <stdio.h>
#include <vector>
#include <memory.h>
#include <hardware/adc.h>
#include <hardware/gpio.h>
#include <hardware/regs/resets.h>
#include <hardware/resets.h>
#include <hardware/vreg.h>
#include <hardware/structs/bus_ctrl.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/time.h>
#include <pico/bootrom.h>
#include <dvi.h>
#include <dvi_serialiser.h>
#include <bsp/board_api.h>
#include <tusb.h>
#include "terminalBase.hxx"
#include "ansiTerminal.hxx"
#include "logging.hxx"
#include "rp65c02.hxx"
#include "cia6526.hxx"
#include "vic6569.hxx"
#include "videoOut.hxx"
#include "keyboard.hxx"
#include "joysticks.hxx"
#include "competitionPro.hxx"
#include "snes.hxx"
#include "rpPetra.hxx"
#include "computer.hxx"

extern uint8_t chargen_rom[];
extern uint8_t basic_rom[];
extern uint8_t kernal_rom[];

#ifdef _MONITOR_CARTRIDGE
extern uint8_t mon_c000[];
#endif
#ifdef _SIMONS_BASIC
extern uint8_t simons_basic[];
extern uint8_t apfel_0801[];
#endif
#ifdef _TRAPDOOR
extern uint8_t trapdoor[];
#endif

#endif