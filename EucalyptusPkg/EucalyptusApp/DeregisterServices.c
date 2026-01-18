#include "Eucalyptus.h"

EFI_STATUS 
EFIAPI
ShutdownUefiBootServices (
  VOID
  )
{
  EFI_STATUS Status;
  UINTN MemoryMapSize;
  EFI_MEMORY_DESCRIPTOR *MemoryMap;
  UINTN MapKey;
  UINTN DescriptorSize;
  UINT32 DescriptorVersion;
  UINTN Pages;

  MemoryMap = NULL;
  MemoryMapSize = 0;
  Pages = 0;

  do {
    Status = gBS->GetMemoryMap (&MemoryMapSize, MemoryMap, &MapKey,
                                &DescriptorSize, &DescriptorVersion);
    if (Status == EFI_BUFFER_TOO_SMALL) {

      Pages = EFI_SIZE_TO_PAGES (MemoryMapSize) + 1;
      MemoryMap = AllocatePages (Pages);
      if (!MemoryMap) {
        Print(L"Failed to allocate pages for memory map");
        return EFI_OUT_OF_RESOURCES;
      }

      Status = gBS->GetMemoryMap (&MemoryMapSize, MemoryMap, &MapKey,
                                  &DescriptorSize, &DescriptorVersion);
    }

    if (!EFI_ERROR (Status)) {
      Status = gBS->ExitBootServices (gImageHandle, MapKey);
      if (EFI_ERROR (Status)) {
        FreePages (MemoryMap, Pages);
        MemoryMap = NULL;
        MemoryMapSize = 0;
      }
    }
  } while (EFI_ERROR (Status));

  return Status;
}

EFI_STATUS 
EFIAPI
ArmPrepareHw (
  VOID
  )
{
  ArmDisableBranchPrediction ();

  ArmDisableAllExceptions ();
  ArmDisableInterrupts ();
  ArmDisableAsynchronousAbort ();

  // clean, invalidate, disable data cache
  WriteBackInvalidateDataCache ();
  InvalidateInstructionCache ();

  ArmDisableMmu ();

  return EFI_SUCCESS;
}

EFI_STATUS 
EFIAPI
PrepareHandoff (
  VOID
  )
{
  EFI_STATUS Status;
    
  Status = ShutdownUefiBootServices ();
  if (EFI_ERROR (Status)) {
    Print(L"Failed to shutdown UEFI boot services. Status=0x%X\n", Status);
    while (TRUE);
  }

  ArmPrepareHw();

  return EFI_SUCCESS;
}
