/**
 * Written by Bernd Krekeler, Herne, Germany
 * 
*/
#include "stdinclude.hxx"

#ifdef _ELITE
extern uint8_t elite[65534];
extern uint8_t elite_d800[1000];
#endif 

// Y (direction of the keyboard matrix)- ROW
static const uint8_t keyboardMapRow[]={0,0,0,0,0xfd,0xf7,0xfb,0xfb,0xfd,0xfb, // 0 (4="A..F")
                                     0xf7,0xf7,0xef,0xef,0xef,0xdf,0xef,0xef,0xef,0xdf,    // 10 ("G..P")
                                     0x7f,0xfb,0xfd,0xfb,0xf7,0xf7,0xfd,0xfb,0xf7,0xfd, // 20 ("Q..Z")
                                     0x7f,0x7f,0xfd,0xfd,0xfb,0xfb,0xf7,0Xf7,0xef,0xef, // 30 ("1..0")
                                     0xfe,0x7f,0xfe,0x7f,0x7f,0xdf,0xdf,0xdf,0xbf,0xbf, // 40 (41=RS, 44=SPC)
                                     0,0xdf,0xbf,0x7f,0xdf,0xdf,0xbf,0,0xfe,0xfe, // 50
                                     0xfe,0xfe,0,0,0,0,0xbf,0xbf,0,0, // 60
                                     0xbf,0,0,0,0,0,0,0,0,0xfe, // 70
                                     0xfe,0xfe,0xfe,0,0,0,0,0,0,0, // 80
                                     0,0,0,0,0,0,0,0,0,0, // 90
                                     0,0x7f,0,0,0,0,0,0,0,0}; // 100

// X (direction of the keyboard matrix)- Columns
static const uint8_t keyboardMapCol[]={0,0,0,0,0xfb,0xef,0xef,0xfb,0xbf,0xdf, // 0
                                     0xfb,0xdf,0xfd,0xfb,0xdf,0xfb,0xef,0x7f,0xbf,0xfd,    // 10
                                     0xbf,0xfd,0xdf,0xbf,0xbf,0x7f,0xfd,0x7f,0xfd,0xef, // 20
                                     0xfe,0xf7,0xfe,0xf7,0xfe,0xf7,0xfe,0xf7,0xfe,0xf7, // 30
                                     0xfd,0x7f,0xfe,0xfb,0xef,0xfe,0xf7,0xbf,0xfd,0xdf, // 40 
                                     0,0xdf,0xfb,0xfd,0x7f,0xef,0x7f,0,0xef,0xdf, // 50
                                     0xbf,0xf7,0,0,0,0,0xbf,0xfe,0,0, // 60
                                     0xf7,0,0,0,0,0,0,0,0,0xfb, // 70
                                     0xfb,0x7f,0x7f,0,0,0,0,0,0,0, // 80
                                     0,0,0,0,0,0,0,0,0,0, // 90
                                     0,0xdf,0,0,0,0,0,0,0,0}; // 100

const char *COMPETITION_PRO="Competition Pro / Speedlink";
const char *SNES_OEM="SNES (OEM)";
const char *UNKNOWN_STICK="Unknown Stick/Pad";

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
  tuh_task();
  do {
    if (m_totalCyles%21000==0)
    {
        tuh_task();
    }

    m_pGlue->Clk(HIGH,&m_systemState,m_totalCyles);
    m_pGlue->Clk(LOW,&m_systemState,m_totalCyles);
    m_totalCyles++;
  } while (1);
  return (0);
}

int Computer::Init()
{
  board_init();
  tusb_init();  
  SIDInit();

  _pGlue=m_pGlue;
  // Create the CPU
  m_pCPU= new RP65C02(m_pLogging);
  // Create the Petra custom chip (glue logic)
  m_pGlue= new RpPetra(m_pLogging, m_pCPU);
  return 0;
}

/**
 * Keyboard processing. We do support multiple keys pressed at the same time.
*/
void process_kbd_report (hid_keyboard_report_t const* report)
{ 
  if (report!=nullptr) 
  {
    _pGlue->m_pKeyboard->OnKeyReleased(0,0);

    if (report->modifier==0x02 || report->modifier==0x20)
    {
      // pressed one of the shift keys...
      if (report->modifier==0x02) { // Left shift key
        _pGlue->m_pKeyboard->OnKeyPressed(0xfd,0x7f); 
      }
      else // right shift key
      {
        _pGlue->m_pKeyboard->OnKeyPressed(0xbf,0xef); 
      }
    }
    int i=0;
    while (report->keycode[i]!=0)
    {
      if (report->keycode[i]==0x40) // F7 => restore.
      {
        _pGlue->SignalNMI(false);
        _pGlue->SignalNMI(true);
      }
      else if (report->keycode[i]<sizeof(keyboardMapRow) && keyboardMapRow[report->keycode[i]]!=0)
      {
        _pGlue->m_pKeyboard->OnKeyPressed(keyboardMapRow[report->keycode[i]],keyboardMapCol[report->keycode[i]]); 
      }
      i++;
    }
  }
}

const char * identifyJoystick(uint8_t dev_addr, bool& supported)
{
  const char *szId="";
  uint16_t vid, pid;

  tuh_vid_pid_get(dev_addr, &vid, &pid);
  supported=true;
 
  switch (vid)
  {
    case 0x54c:
      if (pid==0x268)
      {
        szId=COMPETITION_PRO;    
      }
    break;

    case 0x79:
      if (pid==0x11)
      {
        szId=SNES_OEM;
      }
    break;

    default:
      szId=UNKNOWN_STICK;    
      supported=false;
    break;
  }
  return szId;
}

// Invoked when device with hid interface is mounted
// Report descriptor is also available for use. tuh_hid_parse_report_descriptor()
// can be used to parse common/simple enough descriptor.
// Note: if report descriptor length > CFG_TUH_ENUMERATION_BUFSIZE, it will be skipped
// therefore report_desc = NULL, desc_len = 0

void tuh_hid_mount_cb (uint8_t dev_addr, uint8_t instance,
    uint8_t const* desc_report, uint16_t desc_len)
{
  /* Ask for a report only if this is a keyboard device */
  uint8_t const itf_protocol = tuh_hid_interface_protocol (dev_addr, instance);
  if (itf_protocol == HID_ITF_PROTOCOL_KEYBOARD)
  {
    tuh_hid_receive_report (dev_addr, instance);
  }
  else
  {
    bool supported;
    const char *pId=identifyJoystick(dev_addr,supported);
    if (supported) // TODO: Create a class factory. But for now, we only have 2 types
    {
      if (strcmp(pId,COMPETITION_PRO)==0)
      {
          if (_pGlue->m_pJoystickA!=nullptr)
          {
            delete _pGlue->m_pJoystickA;
          }
          _pGlue->m_pJoystickA=new CompetitionPro(_pGlue->m_pLog);
      }
      else if (strcmp(pId,SNES_OEM)==0)
      {
          if (_pGlue->m_pJoystickA!=nullptr)
          {
            delete _pGlue->m_pJoystickA;
          }
          _pGlue->m_pJoystickA=new SNES(_pGlue->m_pLog);
      }
      if (_pGlue->m_pJoystickA!=nullptr)
      {
        tuh_hid_receive_report(dev_addr, instance);
      }
    }
  }
}

// Invoked when device with hid interface is un-mounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
{

}

/**
 * For joysticks, the report differs. I analysed the report for a simple Competition Pro
 * first, the joystick best known by C64 fellows...
*/
void handleJoystick(const char *szJoystick, uint8_t const * report, uint16_t len)
{
  _pGlue->m_pJoystickA->Convert(report,len);
}

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance,uint8_t const* report, uint16_t len)
{
  bool supported;
  const char *szId;
  switch (tuh_hid_interface_protocol (dev_addr, instance))
  {
    case HID_ITF_PROTOCOL_KEYBOARD:
      process_kbd_report ((hid_keyboard_report_t const*) report);
      tuh_hid_receive_report (dev_addr, instance);
    break;

    default:
      szId=identifyJoystick(dev_addr, supported);
      if (supported)
      {
        handleJoystick(szId, report, len);
        tuh_hid_receive_report(dev_addr, instance);
      }
    break;
  }
}  

