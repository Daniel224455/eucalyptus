#include "Eucalyptus.h"

EFI_STATUS
EFIAPI
LoadStage1Elf (
  IN EFI_HANDLE        ImageHandle,
  IN UINT64            Base,
  IN UINT64            Size  
  )
{
  EFI_STATUS                      Status;
  EFI_LOADED_IMAGE_PROTOCOL       *LoadedImage;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
  EFI_FILE_PROTOCOL               *Root;
  EFI_FILE_PROTOCOL               *PayloadFile;
  UINTN                           FileInfoSize;
  EFI_FILE_INFO                   *FileInfo;
  VOID                            *PayloadBuffer;
  UINTN                           PayloadSize;

  Status = gBS->HandleProtocol(
                  ImageHandle,
                  &gEfiLoadedImageProtocolGuid,
                  (VOID **)&LoadedImage
                  );
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Failed to get LoadedImage protocol: %r\n", Status);
    return Status;
  }

  Print(L"-> image handle found\n");

  Status = gBS->HandleProtocol(
                  LoadedImage->DeviceHandle,
                  &gEfiSimpleFileSystemProtocolGuid,
                  (VOID **)&FileSystem
                  );
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Failed to get filesystem protocol: %r\n", Status);
    return Status;
  }

  Print(L"-> filesystem found on boot device\n");

  Status = FileSystem->OpenVolume(FileSystem, &Root);
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Failed to open root volume: %r\n", Status);
    return Status;
  }

  Print(L"-> opened root volume\n");

  Status = Root->Open(
                   Root,
                   &PayloadFile,
                   L"stage1.elf",
                   EFI_FILE_MODE_READ,
                   0
                   );
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Failed to open stage1.elf: %r\n", Status);
    Root->Close(Root);
    return Status;
  }

  Print(L"-> stage1.elf found!\n");

  ArmSetMemoryAttributes(Base, Size, EFI_MEMORY_WB, 0);

  FileInfoSize = 0;
  Status = PayloadFile->GetInfo(
                          PayloadFile,
                          &gEfiFileInfoGuid,
                          &FileInfoSize,
                          NULL
                          );
  if (Status != EFI_BUFFER_TOO_SMALL) {
    Print(L"ERROR: Failed to get file info size: %r\n", Status);
    PayloadFile->Close(PayloadFile);
    Root->Close(Root);
    return Status;
  }

  FileInfo = AllocatePool(FileInfoSize);
  if (FileInfo == NULL) {
    Print(L"ERROR: Failed to allocate memory for file info\n");
    PayloadFile->Close(PayloadFile);
    Root->Close(Root);
    return EFI_OUT_OF_RESOURCES;
  }

  Status = PayloadFile->GetInfo(
                          PayloadFile,
                          &gEfiFileInfoGuid,
                          &FileInfoSize,
                          FileInfo
                          );
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Failed to get file info: %r\n", Status);
    FreePool(FileInfo);
    PayloadFile->Close(PayloadFile);
    Root->Close(Root);
    return Status;
  }
  
  PayloadSize = (UINTN)FileInfo->FileSize;
  Print(L"-> stage1 size: %lu bytes (%lu KB)\n", PayloadSize, PayloadSize / 1024);

  FreePool(FileInfo);

  PayloadBuffer = AllocatePool(PayloadSize);
  if (PayloadBuffer == NULL) {
    Print(L"ERROR: Failed to allocate %lu bytes for stage1\n", PayloadSize);
    PayloadFile->Close(PayloadFile);
    Root->Close(Root);
    return EFI_OUT_OF_RESOURCES;
  }

  Print(L"-> allocated buffer at 0x%p\n", PayloadBuffer);

  Status = PayloadFile->Read(PayloadFile, &PayloadSize, PayloadBuffer);
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Failed to read stage1: %r\n", Status);
    FreePool(PayloadBuffer);
    PayloadFile->Close(PayloadFile);
    Root->Close(Root);
    return Status;
  }

  Print(L"-> successfully read stage1!\n");
  Print(L"-> first 16 bytes (hex): ");
  {
    UINT8 *Bytes = (UINT8 *)PayloadBuffer;
    for (UINTN i = 0; i < 16 && i < PayloadSize; i++) {
      Print(L"%02x ", Bytes[i]);
    }
    Print(L"\n");
  }

  Print(L"-> stage1 buffer at 0x%p\n", PayloadBuffer);

  ELF64_EHDR *Eh = (ELF64_EHDR *)PayloadBuffer;
  UINT64 Entry = Eh->e_entry;

  if (Eh->e_ident[0] != 0x7F ||
      Eh->e_ident[1] != 'E'  ||
      Eh->e_ident[2] != 'L'  ||
      Eh->e_ident[3] != 'F') {
    Print(L"ERROR: Not an ELF payload\n");
    goto Fail;
  }

  if (Eh->e_machine != 183) { // EM_AARCH64
    Print(L"ERROR: Not AArch64 ELF payload (e_machine=%u)\n", Eh->e_machine);
    goto Fail;
  }

  Print(L"-> ELF entry: 0x%lx\n", Eh->e_entry);
  Print(L"-> ELF PHDRs: %u at 0x%lx\n", Eh->e_phnum, Eh->e_phoff);

  ELF64_PHDR *Ph = (ELF64_PHDR *)((UINT8 *)PayloadBuffer + Eh->e_phoff);

  for (UINTN i = 0; i < Eh->e_phnum; i++, Ph++) {
    if (Ph->p_type != PT_LOAD) {
      continue;
    }

    EFI_PHYSICAL_ADDRESS LoadAddr = (EFI_PHYSICAL_ADDRESS)Ph->p_paddr;

    Print(L"-> PT_LOAD[%u]: paddr=0x%lx vaddr=0x%lx off=0x%lx filesz=0x%lx memsz=0x%lx\n",
          i, Ph->p_paddr, Ph->p_vaddr, Ph->p_offset, Ph->p_filesz, Ph->p_memsz);

    // copy file contents
    CopyMem(
      (VOID *)(UINTN)LoadAddr,
      (UINT8 *)PayloadBuffer + Ph->p_offset,
      (UINTN)Ph->p_filesz
      );

    // zero bss tail
    if (Ph->p_memsz > Ph->p_filesz) {
      SetMem(
        (VOID *)(UINTN)(LoadAddr + Ph->p_filesz),
        (UINTN)(Ph->p_memsz - Ph->p_filesz),
        0
        );
    }

    WriteBackInvalidateDataCacheRange((VOID *)(UINTN)LoadAddr, (UINTN)Ph->p_memsz);
    InvalidateInstructionCacheRange((VOID *)(UINTN)LoadAddr, (UINTN)Ph->p_memsz);
  }

  // we are done with the file image
  PayloadFile->Close(PayloadFile);
  Root->Close(Root);
  FreePool(PayloadBuffer);

  Print(L"-> executing stage1 at 0x%lx\n", Entry);

  VOID (*Jump)(VOID) = (VOID (*)(VOID))(UINTN)Entry;
  Jump();

  Print(L"-> stage1 executed\n");
  return EFI_SUCCESS;

Fail:
  PayloadFile->Close(PayloadFile);
  Root->Close(Root);
  FreePool(PayloadBuffer);
  return EFI_LOAD_ERROR;
}


EFI_STATUS
EFIAPI
LoadStage2Elf (
  IN EFI_HANDLE        ImageHandle,
  IN UINT64            Base,
  IN UINT64            Size  
  )
{
  EFI_STATUS                      Status;
  EFI_LOADED_IMAGE_PROTOCOL       *LoadedImage;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
  EFI_FILE_PROTOCOL               *Root;
  EFI_FILE_PROTOCOL               *PayloadFile;
  UINTN                           FileInfoSize;
  EFI_FILE_INFO                   *FileInfo;
  VOID                            *PayloadBuffer;
  UINTN                           PayloadSize;

  Status = gBS->HandleProtocol(
                  ImageHandle,
                  &gEfiLoadedImageProtocolGuid,
                  (VOID **)&LoadedImage
                  );
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Failed to get LoadedImage protocol: %r\n", Status);
    return Status;
  }

  Print(L"-> image handle found\n");

  Status = gBS->HandleProtocol(
                  LoadedImage->DeviceHandle,
                  &gEfiSimpleFileSystemProtocolGuid,
                  (VOID **)&FileSystem
                  );
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Failed to get filesystem protocol: %r\n", Status);
    return Status;
  }

  Print(L"-> filesystem found on boot device\n");

  Status = FileSystem->OpenVolume(FileSystem, &Root);
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Failed to open root volume: %r\n", Status);
    return Status;
  }

  Print(L"-> opened root volume\n");

  Status = Root->Open(
                   Root,
                   &PayloadFile,
                   L"stage2.elf",
                   EFI_FILE_MODE_READ,
                   0
                   );
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Failed to open stage2.elf: %r\n", Status);
    Root->Close(Root);
    return Status;
  }

  Print(L"-> stage2.elf found!\n");

  ArmSetMemoryAttributes(Base, Size, EFI_MEMORY_WB, 0);

  FileInfoSize = 0;
  Status = PayloadFile->GetInfo(
                          PayloadFile,
                          &gEfiFileInfoGuid,
                          &FileInfoSize,
                          NULL
                          );
  if (Status != EFI_BUFFER_TOO_SMALL) {
    Print(L"ERROR: Failed to get file info size: %r\n", Status);
    PayloadFile->Close(PayloadFile);
    Root->Close(Root);
    return Status;
  }

  FileInfo = AllocatePool(FileInfoSize);
  if (FileInfo == NULL) {
    Print(L"ERROR: Failed to allocate memory for file info\n");
    PayloadFile->Close(PayloadFile);
    Root->Close(Root);
    return EFI_OUT_OF_RESOURCES;
  }

  Status = PayloadFile->GetInfo(
                          PayloadFile,
                          &gEfiFileInfoGuid,
                          &FileInfoSize,
                          FileInfo
                          );
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Failed to get file info: %r\n", Status);
    FreePool(FileInfo);
    PayloadFile->Close(PayloadFile);
    Root->Close(Root);
    return Status;
  }
  
  PayloadSize = (UINTN)FileInfo->FileSize;
  Print(L"-> stage2 size: %lu bytes (%lu KB)\n", PayloadSize, PayloadSize / 1024);

  FreePool(FileInfo);

  PayloadBuffer = AllocatePool(PayloadSize);
  if (PayloadBuffer == NULL) {
    Print(L"ERROR: Failed to allocate %lu bytes for stage2\n", PayloadSize);
    PayloadFile->Close(PayloadFile);
    Root->Close(Root);
    return EFI_OUT_OF_RESOURCES;
  }

  Print(L"-> allocated buffer at 0x%p\n", PayloadBuffer);

  Status = PayloadFile->Read(PayloadFile, &PayloadSize, PayloadBuffer);
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Failed to read stage2: %r\n", Status);
    FreePool(PayloadBuffer);
    PayloadFile->Close(PayloadFile);
    Root->Close(Root);
    return Status;
  }

  Print(L"-> successfully read stage2!\n");
  Print(L"-> first 16 bytes (hex): ");
  {
    UINT8 *Bytes = (UINT8 *)PayloadBuffer;
    for (UINTN i = 0; i < 16 && i < PayloadSize; i++) {
      Print(L"%02x ", Bytes[i]);
    }
    Print(L"\n");
  }

  Print(L"-> stage2 buffer at 0x%p\n", PayloadBuffer);

  ELF64_EHDR *Eh = (ELF64_EHDR *)PayloadBuffer;
  UINT64 Entry = Eh->e_entry;

  if (Eh->e_ident[0] != 0x7F ||
      Eh->e_ident[1] != 'E'  ||
      Eh->e_ident[2] != 'L'  ||
      Eh->e_ident[3] != 'F') {
    Print(L"ERROR: Not an ELF payload\n");
    goto Fail;
  }

  if (Eh->e_machine != 183) { // EM_AARCH64
    Print(L"ERROR: Not AArch64 ELF payload (e_machine=%u)\n", Eh->e_machine);
    goto Fail;
  }

  Print(L"-> ELF entry: 0x%lx\n", Eh->e_entry);
  Print(L"-> ELF PHDRs: %u at 0x%lx\n", Eh->e_phnum, Eh->e_phoff);

  ELF64_PHDR *Ph = (ELF64_PHDR *)((UINT8 *)PayloadBuffer + Eh->e_phoff);

  for (UINTN i = 0; i < Eh->e_phnum; i++, Ph++) {
    if (Ph->p_type != PT_LOAD) {
      continue;
    }

    EFI_PHYSICAL_ADDRESS LoadAddr = (EFI_PHYSICAL_ADDRESS)Ph->p_paddr;

    Print(L"-> PT_LOAD[%u]: paddr=0x%lx vaddr=0x%lx off=0x%lx filesz=0x%lx memsz=0x%lx\n",
          i, Ph->p_paddr, Ph->p_vaddr, Ph->p_offset, Ph->p_filesz, Ph->p_memsz);

    // copy file contents
    CopyMem(
      (VOID *)(UINTN)LoadAddr,
      (UINT8 *)PayloadBuffer + Ph->p_offset,
      (UINTN)Ph->p_filesz
      );

    // zero bss tail
    if (Ph->p_memsz > Ph->p_filesz) {
      SetMem(
        (VOID *)(UINTN)(LoadAddr + Ph->p_filesz),
        (UINTN)(Ph->p_memsz - Ph->p_filesz),
        0
        );
    }

    WriteBackInvalidateDataCacheRange((VOID *)(UINTN)LoadAddr, (UINTN)Ph->p_memsz);
    InvalidateInstructionCacheRange((VOID *)(UINTN)LoadAddr, (UINTN)Ph->p_memsz);
  }

  // we are done with the file image
  PayloadFile->Close(PayloadFile);
  Root->Close(Root);
  FreePool(PayloadBuffer);

  Print(L"-> executing stage2 at 0x%lx, good night\n", Entry);

  VOID (*Jump)(VOID) = (VOID (*)(VOID))(UINTN)Entry;
  PrepareHandoff();
  Jump();

  Print(L"-> stage2 returned\n");
  while (TRUE);

Fail:
  PayloadFile->Close(PayloadFile);
  Root->Close(Root);
  FreePool(PayloadBuffer);
  return EFI_LOAD_ERROR;
}