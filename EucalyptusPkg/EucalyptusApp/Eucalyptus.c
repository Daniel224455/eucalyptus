/** @file
  Copyright (c) 2020 - 2026, Daniel224455. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Eucalyptus.h"
#include <Library/UefiApplicationEntryPoint.h>

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

/**
  The Entry Point for Eucalyptus.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{  
  Print(L"============================\n");
  Print(L"Welcome to Eucalyptus\n");
  Print(L"Universal BIN and ELF loader\n");
  Print(L"============================\n");  
  Print(L"Binary build date: \n%a on %a\n", __TIME__, __DATE__);
  Print(L"============================\n");
  Print(L"github.com/Daniel224455\n");
  Print(L"============================\n");

  UINTN El = GetCurrentExceptionLevel();

  if (El == 3) {
    // uefi cannot run in el3, how did you get here
    Print(L"-> running in EL3, how did you get here?\n");
  } else if (El == 1) {
    Print(L"-> running in EL1\n"); 
  } else if (El == 2) {
    Print(L"-> running in EL2\n");  
  }

  EUC_CONFIG Cfg = LoadConfig(ImageHandle);

  EUC_MODE Type = GetPayload(ImageHandle);

  switch (Type) {
    case MODE_BIN:
      PayloadBinParseAndLoad(ImageHandle, Cfg.RelocBase, Cfg.RelocSize);
      break;
  
    case MODE_ELF:
      PayloadElfParseAndLoad(ImageHandle, Cfg.RelocBase, Cfg.RelocSize);
      break;
  
    default:
      Print(L"-> No payload detected\n");
      return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}
