#include "Eucalyptus.h"

STATIC
UINTN
GetCurrentExceptionLevel (
  VOID
  )
{
  UINTN El;

  //
  // Read CurrentEL system register.
  // bits [3:2] hold the EL value, encoded as:
  //   0b00 = EL0
  //   0b01 = EL1
  //   0b10 = EL2
  //   0b11 = EL3
  //
  // so we shift right by 2 and mask with 0x3.
  //
  UINT64 CurrentElReg;

  __asm__ volatile ("mrs %0, CurrentEL" : "=r" (CurrentElReg));
  El = (UINTN)((CurrentElReg >> 2) & 0x3U);

  return El;
}

STATIC
UINTN
EscalateIntoHypervisorEL2 (
  VOID
  )
{
  __asm__ volatile (
      "ldr   w0, =0x02000121\n"   // TZ_EL2_SWITCH_SMC_ID
      "ldr   w1, =0x00000023\n"   // TZ_EL2_SWITCH_PARAM_ID
      "mov   w2, wzr\n"
      "mov   w3, wzr\n"
      "ldr   w4, =0x1\n"          // TZ_EL2_SWITCH_PARAM2_EXIT_GUNYAH
      "smc   #0"
  );

  return EFI_SUCCESS;
}

UINTN
CheckPrivileges (
  VOID
  )
{
  UINTN El = GetCurrentExceptionLevel();

  if (El == 3) {
    Print(L"-> running in EL3\n");
  } else if (El == 1) {
    // TZ IDs are under investigation.
    //Print(L"-> running in el1, switching...");
    //EscalateIntoHypervisorEL2();
    //Print(L"OK\n");
    Print(L"-> running in EL1\n"); 
  } else if (El == 2) {
    Print(L"-> running in EL2\n");  
  }

  return EFI_SUCCESS;
}