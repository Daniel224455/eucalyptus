#include "Eucalyptus.h"

EFI_STATUS
EFIAPI
PayloadBinParseAndLoad (
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

  // get the loadedimage protocol to find our boot device
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

  // get the filesystem protocol from our boot device
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

  // open the root dir
  Status = FileSystem->OpenVolume(FileSystem, &Root);
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Failed to open root volume: %r\n", Status);
    return Status;
  }

  Print(L"-> opened root volume\n");

  // try to open payload.bin from root
  Status = Root->Open(
                   Root,
                   &PayloadFile,
                   L"payload.bin",
                   EFI_FILE_MODE_READ,
                   0
                   );
  
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Failed to open payload.bin: %r\n", Status);
    Print(L"Make sure payload.bin was included in the fs\n");
    Root->Close(Root);
    return Status;
  }

  Print(L"-> payload found!\n");

  ArmSetMemoryAttributes(Base, Size, EFI_MEMORY_WB, 0);

  // get file info to determine size
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
  Print(L"-> payload size: %lu bytes (%lu KB)\n", PayloadSize, PayloadSize / 1024);

  FreePool(FileInfo);

  // alloc buffer for payload
  PayloadBuffer = AllocatePool(PayloadSize);
  if (PayloadBuffer == NULL) {
    Print(L"ERROR: Failed to allocate %lu bytes for payload\n", PayloadSize);
    PayloadFile->Close(PayloadFile);
    Root->Close(Root);
    return EFI_OUT_OF_RESOURCES;
  }

  Print(L"-> allocated buffer at 0x%p\n", PayloadBuffer);

  // read the payload
  Status = PayloadFile->Read(PayloadFile, &PayloadSize, PayloadBuffer);
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Failed to read payload: %r\n", Status);
    FreePool(PayloadBuffer);
    PayloadFile->Close(PayloadFile);
    Root->Close(Root);
    return Status;
  }

  Print(L"-> successfully read payload!\n");
  Print(L"-> first 16 bytes (hex): ");
  
  // show first 16 bytes
  UINT8 *Bytes = (UINT8 *)PayloadBuffer;
  for (UINTN i = 0; i < 16 && i < PayloadSize; i++) {
    Print(L"%02x ", Bytes[i]);
  }

  Print(L"\n-> payload loaded successfully at 0x%p\n", PayloadBuffer);

  SetMem((VOID *)Base, Size, 0x00);
  CopyMem((VOID *)Base, PayloadBuffer, Size);
  WriteBackInvalidateDataCacheRange((VOID *)Base, Size);
  InvalidateInstructionCacheRange((VOID *)Base, Size);

  Print(L"-> payload relocated to 0x%p\n", Base);

  // cleanup
  FreePool(PayloadBuffer);
  PayloadFile->Close(PayloadFile);
  Root->Close(Root);

  VOID (*Entry)(VOID) = (VOID (*)(VOID))Base;
  Print(L"-> executing payload at 0x%p, good night\n", Base);
  PrepareHandoff();
  Entry();

  while (TRUE);
}