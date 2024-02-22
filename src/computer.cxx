#include <hardware/gpio.h>
#include <hardware/adc.h>
#include <pico/stdlib.h>
#include <bsp/board_api.h>
#include <pico/bootrom.h>
#include <tusb.h>
#include <dvi.h>
#include <dvi_serialiser.h>
#include "logging.hxx"
#include "rp65c02.hxx"
#include "cia6526.hxx"
#include "vic6569.hxx"
#include "videoOut.hxx"
#include "rpPetra.hxx"
#include "computer.hxx"

/*
    Keyboard mappings are incomplete/wrong and have been implemented to at least
    be able to test some basic code. Code will later be replaced by real CIA#1 emulation.

*/

static const u_int8_t _keyboard_mapping[]={0,0,0,0,65,66,67,68,69,70, //0 
                                     71,72,73,74,75,76,77,78,79,80, // 10
                                     81,82,83,84,85,86,87,88,89,90, // 20  (Z)
                                     49,50,51,52,53,54,55,56,57,48, // 30 (1)
                                     13,0x83,20,58,32,0,19 /* HOME */,41,0,61, //40, 43==CTRL(58)
                                     0,58,59,0,44,0,0,0,0,0, // 50
                                     0,0,0,0,0,0,0,0,0,0, // 60
                                     0,0,0,0,0,0,0,0,0, 29/* CRSR right */, // 70
                                     157 /* CRSR left */,17 /* CRSR down */,145 /* CRSR up */,0,0,0,0,0,0,0x31, // 80 
                                     0x32,59,8,11,16,19,24,27,32,35, // 90
                                     0,0,0,0,0,0,0,0,0,0, // 100
                                     0,0,0,0,0,0,0,0,0,0, // 110
                                     0,0,0,0,0,0,0};

static const u_int8_t _keyboard_mapping_shift[]={0,0,0,0,65,66,67,68,69,70, //0 
                                     71,72,73,74,75,76,77,78,79,80, // 10
                                     81,82,83,84,85,86,87,88,89,90, // 20  (Z)
                                     33,34,35,36,37,38,39,40,41,61, // 30, !
                                     34,0x83,20,0,32,0,147 /*(CLR/HOME)*/,41,0,0, //40
                                     0,58,0,0,44,0,0,0,0,0, // 50
                                     0,0,0,0,0,0,0,0,0,0, // 60
                                     0,0,0,0,0,0,0,0,0,0, // 70
                                     0,0,0,0,0,0,0,0,0,0x31, // 80 
                                     0x32,59,8,11,16,19,24,27,32,35, // 90
                                     0,0,0,0,0,0,0,0,0,0, // 100
                                     0,0,0,0,0,0,0,0,0,0, // 110
                                     0,0,0,0,0,0,0};


static const u_int8_t _keyboard_mapping_ctrl[]={0,0,0,0,0,0,0,0,0,0, // 0
                                                0,0,0,0,0,0,0,0,0,0, // 10
                                                0,0,0,0,0,0,0,0,0,0, // 20 
                                                0x1c,0x05,28,0x9f,0x9c,30,158,0x0a,0x12,0x92, // 30
                                                0,0x13,0,0,0,0,0,0,0,0, // 0x13=Home
                                                0,0,0,0,0,0,0,0,0,0,
                                                0,0,0,0,0,0,0,0,0,0,
                                                0,0,0,0,0,0,0,0,0,0,
                                                0,0,0,0,0,0,0,0,0,0,
                                                0,0,0,0,0,0,0,0,0,0,
                                                0,0,0,0,0,0,0,0,0,0,
                                                0,0,0,0,0,0,0,0,0,0,
                                                0,0,0,0,0,0,0,0,0,0,
                                                0,0,0,0,0,0,0,0};

static const u_int8_t _keyboard_mapping_commodore[]={0,0,0,0,0,0,0,0,0,0, // 0
                                                0,0,0,0,0,0,0,0,0,0, // 10
                                                0,0,0,0,0,0,0,0,0,0, // 20 
                                                0x1c,0x05,28,0x9f,0x9c,30,158,0x0a,0x12,0x92, // 30
                                                0,0x13,0,0,0,0,0,0,0,0, // 0x13=Home
                                                0,0,0,0,0,0,0,0,0,0,
                                                0,0,0,0,0,0,0,0,0,0,
                                                0,0,0,0,0,0,0,0,0,0,
                                                0,0,0,0,0,0,0,0,0,0,
                                                0,0,0,0,0,0,0,0,0,0,
                                                0,0,0,0,0,0,0,0,0,0,
                                                0,0,0,0,0,0,0,0,0,0,
                                                0,0,0,0,0,0,0,0,0,0,
                                                0,0,0,0,0,0,0,0};                                                

Computer::Computer(Logging *pLogging)
{
    m_pLogging=pLogging;
    m_systemState.cpuState.isA0A15SetToOutput=true;
    m_totalCyles=0;
}

Computer::~Computer()
{
}

int Computer::Run()
{
  Init();

  do {
    if (m_totalCyles%30000==0)
    {
        tuh_task();
    }
    m_pGlue->Clk(HIGH,&m_systemState);
    m_pGlue->Clk(LOW,&m_systemState);
    m_totalCyles++;
  } while (1);

  return (0);
}

int Computer::Init()
{
  board_init();
  tusb_init();  

  // Create the CPU
  m_pCPU= new RP65C02(m_pLogging);
  // Create the Petra custom chip (glue logic)
  m_pGlue= new RpPetra(m_pLogging, m_pCPU);
  _pGlue=m_pGlue;

  return 0;
}

/**
 * Very basic keyboard processing here just to be able to play around with basic
 * and to get used to tinyUSB.
 * 
*/
void process_kbd_report (hid_keyboard_report_t const* report)
{ 
  if (report!=nullptr) 
  {
    if (report->keycode[0]==0 && report->modifier==0) // No key pressed
    {
      _pGlue->m_pRAM[653]=0x00;
    }
    else
    {
      if (report->modifier)
      {
        if (report->modifier & 1) // Ctrl key will become Commodore key
        {
          _pGlue->m_pRAM[653]|=0x02;  
          if (_keyboard_mapping_commodore[report->keycode[1]]!=0) 
          {
              _pGlue->m_pRAM[631]=_keyboard_mapping_commodore[report->keycode[1]];
              _pGlue->m_pRAM[198]=1;
          }
        }
        if (report->modifier & 2) // Shift key
        {
          _pGlue->m_pRAM[653]|=0x01;
          if (_keyboard_mapping_shift[report->keycode[0]]!=0)
          {
            if (report->keycode[0]!=0) {
              _pGlue->m_pRAM[631]=_keyboard_mapping_shift[report->keycode[0]];
              _pGlue->m_pRAM[198]=1;
            }
          }
        }
      }
      else 
      {
        if (report->keycode[0]==43) { // CTRL
          _pGlue->m_pRAM[653]|=0x04;
          if (_keyboard_mapping_ctrl[report->keycode[1]]!=0) 
          {
            _pGlue->m_pRAM[631]=_keyboard_mapping_ctrl[report->keycode[1]];
            _pGlue->m_pRAM[198]=1;
          }
        }
        else if (report->keycode[0]==41) // Run/Stop simulation
        {
            _pGlue->m_pRAM[631]=0x03;
            _pGlue->m_pRAM[198]=1;
            _pGlue->m_pRAM[0x91]=0x7f;
        }
        else
        {
          if (_keyboard_mapping[report->keycode[0]]!=0)
          {
            _pGlue->m_pRAM[631]=_keyboard_mapping[report->keycode[0]];
            _pGlue->m_pRAM[198]=1;
          }
        }
      }
    }
  }
}

void tuh_hid_mount_cb (uint8_t dev_addr, uint8_t instance,
    uint8_t const* desc_report, uint16_t desc_len)
{
  /* Ask for a report only if this is a keyboard device */
  uint8_t const itf_protocol = tuh_hid_interface_protocol (dev_addr, instance);
  if (itf_protocol == HID_ITF_PROTOCOL_KEYBOARD)
  {
    tuh_hid_receive_report (dev_addr, instance);
  }
}

void tuh_hid_report_received_cb  (uint8_t dev_addr, uint8_t instance,uint8_t const* report, uint16_t len)
{
  switch (tuh_hid_interface_protocol (dev_addr, instance))
  {
    case HID_ITF_PROTOCOL_KEYBOARD:
      process_kbd_report ((hid_keyboard_report_t const*) report);
      tuh_hid_receive_report (dev_addr, instance);
    break;
  }
}

