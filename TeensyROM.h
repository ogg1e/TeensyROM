

#define PHI2_PIN            1  
const uint8_t InputPins[] = {
   19,18,14,15,40,41,17,16,22,23,20,21,38,39,26,27,  //address bus
   2, 3, 4, 5, PHI2_PIN, 0,         // IO1n, IO2n, ROML, ROMH, PHI2_PIN, R_Wn
   25,     //BA
   };

const uint8_t OutputPins[] = {
   35, 9, 32, 6, // DataCEn, ExROM, Game, Reset
   34,33,    //debug
   };

#define ReadGPIO6          (*(volatile uint32_t *)IMXRT_GPIO6_ADDRESS)
#define GP6_R_Wn(r)        (r & CORE_PIN0_BITMASK)
#define GP6_Address(r)     ((r >> CORE_PIN19_BIT) & 0xFFFF)  // bits 16-31 contain address bus, in order 

#define ReadGPIO9          (*(volatile uint32_t *)IMXRT_GPIO9_ADDRESS)
#define GP9_IO1n(r)        (r & CORE_PIN2_BITMASK)
#define GP9_IO2n(r)        (r & CORE_PIN3_BITMASK)
#define GP9_ROML(r)        (r & CORE_PIN4_BITMASK)
#define GP9_ROMH(r)        (r & CORE_PIN5_BITMASK)
#define GP9_BA(r)          (r & CORE_PIN29_BITMASK)

#define ReadGPIO7          (*(volatile uint32_t *)IMXRT_GPIO7_ADDRESS)
#define GP7_DataMask       0xF000F   //CORE_PIN10,12,11,13,8,7,36,37_BITMASK
#define SetDataPortDirOut  CORE_PIN10_DDRREG |= GP7_DataMask
#define SetDataPortDirIn   CORE_PIN10_DDRREG &= ~GP7_DataMask

#define DataBufDisable     CORE_PIN35_PORTSET = CORE_PIN35_BITMASK
#define DataBufEnable      CORE_PIN35_PORTCLEAR = CORE_PIN35_BITMASK
#define SetResetAssert     CORE_PIN6_PORTSET = CORE_PIN6_BITMASK
#define SetResetDeassert   CORE_PIN6_PORTCLEAR = CORE_PIN6_BITMASK 
#define SetExROMAssert     CORE_PIN9_PORTSET = CORE_PIN9_BITMASK
#define SetExROMDeassert   CORE_PIN9_PORTCLEAR = CORE_PIN9_BITMASK 
#define SetGameAssert      CORE_PIN32_PORTSET = CORE_PIN32_BITMASK
#define SetGameDeassert    CORE_PIN32_PORTCLEAR = CORE_PIN32_BITMASK 
#define SetDebugAssert     CORE_PIN34_PORTSET = CORE_PIN34_BITMASK
#define SetDebugDeassert   CORE_PIN34_PORTCLEAR = CORE_PIN34_BITMASK 
#define SetDebug2Assert    CORE_PIN33_PORTSET = CORE_PIN33_BITMASK
#define SetDebug2Deassert  CORE_PIN33_PORTCLEAR = CORE_PIN33_BITMASK 

#define RESET_CYCLECOUNT   { ARM_DEMCR |= ARM_DEMCR_TRCENA; ARM_DWT_CTRL |= ARM_DWT_CTRL_CYCCNTENA; ARM_DWT_CYCCNT = 0; }
#define WaitUntil_nS(N)      while(ARM_DWT_CYCCNT < N * 0.816) 
   //was ARM_DWT_CYCCNT < ((F_CPU_ACTUAL>>16) * N) / (1000000000UL>>16)     F_CPU_ACTUAL=816000000  /1000000000 = 0.816
// all times starting from Phi2 rising (interrupt). 
#define nS_RWnReady        20  // Phi2 rise to RWn valid
#define nS_PLAprop         100 //100 //delay through PLA to decode address (IO1/2, ROML/H), have measured >100nS from Phi2 to IO1 (delayed through PLA, etc)
#define nS_DataSetup       325 //On a write, when to latch data bus. spec calls for 150-200nS min to Data valid for write opperation (TMDS)
#define nS_DataHold        375 //375  //On a read, when to stop driving the data bus, spec calls for >430

__attribute__((always_inline)) inline void DataPortWriteWait(uint8_t Data)
{
   DataBufEnable; 
   register uint32_t RegBits = (Data & 0x0F) | ((Data & 0xF0) << 12);
   CORE_PIN7_PORTSET = RegBits;
   CORE_PIN7_PORTCLEAR = ~RegBits & GP7_DataMask;
   WaitUntil_nS(nS_DataHold);  //could poll Phi2 for falling edge...  only 30nS typ hold time
   DataBufDisable;
}

__attribute__(( always_inline )) inline uint8_t DataPortWaitRead()
{
   SetDataPortDirIn; //set data ports to inputs         //data port set to read previously
   DataBufEnable; //enable external buffer
   WaitUntil_nS(nS_DataSetup);  //could poll Phi2 for falling edge...  only 30nS typ hold time
   register uint32_t DataIn = ReadGPIO7;
   DataBufDisable;
   SetDataPortDirOut; //set data ports to outputs (default)
   return ((DataIn & 0x0F) | ((DataIn >> 12) & 0xF0));
}

