/** @file
  Copyright (c) 2020 - 2026, Daniel224455. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Eucalyptus.h"
#include <Library/UefiApplicationEntryPoint.h>

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

  CheckPrivileges();  

  EUC_CONFIG Cfg = LoadConfig(ImageHandle);
  EUC_MODE Type = GetPayloadStage1(ImageHandle);
  
  switch (Type) {
    case MODE_BIN:
      Stage1ParseAndLoad(ImageHandle, Cfg.RelocBase, Cfg.RelocSize);
      break;
  
    case MODE_ELF:
      LoadStage1Elf(ImageHandle, Cfg.RelocBase, Cfg.RelocSize);
      break;
  
    default:
      Print(L"-> No payload detected\n");
      return EFI_NOT_FOUND;
  }

  EUC_MODE Type2 = GetPayloadStage2(ImageHandle);

  switch (Type2) {
    case MODE_BIN:
      Stage2ParseAndLoad(ImageHandle, Cfg.RelocBase, Cfg.RelocSize);
      break;
  
    case MODE_ELF:
      LoadStage2Elf(ImageHandle, Cfg.RelocBase, Cfg.RelocSize);
      break;
  
    default:
      Print(L"-> No payload detected\n");
      return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}
