#ifndef RP65C02_HXX_
#define RP65C02_HXX_

//extern class RP65C02 *g_p65c02;

// This bitmask describes which PIO pins will be written to by "gpio_put_masked" api call
constexpr uint32_t pioMask_8_9_10 =    0b0000000000000000000011100000000; // Modify PIO PINs 8,9 and 10 (74LVC245APW U5, U6 and U7) only
constexpr uint32_t pioMask_8_9_10_11 = 0b0000000000000000000111100000000; // Modify PIO PINs 8, 9,10 and 11 (U7, OE and DIR) only

constexpr uint32_t pioMask_11 = 0b0000000000000000000100000000000; // Modify PIO PINs 11 (DIR) only
constexpr uint32_t directionOutput = 0b0000000000000000000000000000000; // Modify PIO PINs 11 (DIR) only

// These bitmasks describe the values for each PIO pin masked by pioMask using the "gpio_put_masked" api call
constexpr uint32_t maskTransceiver= 0b00000000000000000000000011111111; // U5, U6 and U7 are connected to GPIO0-7 (multiplexed)

constexpr uint32_t transceiverDirectionOutput= 0b00000000000000000000000011111111; // direction of GPIO0-7 shall be output
constexpr uint32_t transceiverDirectionInput=  0b00000000000000000000000000000000; // direction of GPIO0-7 shall be input


typedef enum {
  RESET=26,   // RESB(40) <-- UEXT pin 9
  IRQ=25,     // IRQB <-- UEXT pin 10
  NMI=27,     // NMI <-- UEXT pin 8
  RW=11,      // RW
  CLK=21      // PHI2 (CLK)
} PINS;

typedef enum {
  LOW=false,
  HIGH=true
} CLOCK;

#define HIGH true
#define LOW false

#define DELAY_FACTOR_SHORT() asm volatile("nop\nnop\nnop\n");

// OK #define DELAY_FACTOR_TRANSCEIVER() asm volatile("nop\nnop\nnop\nnop\n");
#define DELAY_FACTOR_TRANSCEIVER() asm volatile("nop\nnop\nnop\nnop\n");


typedef enum
{
  ToCpu=0,
  FromCpu=1

} Direction;

class RP65C02 {

  private:
    class Logging *m_pLogging;

  public:
    RP65C02(Logging *pLogging);
    virtual ~RP65C02();
};

#endif