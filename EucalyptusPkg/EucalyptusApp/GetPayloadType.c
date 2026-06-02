#include "Eucalyptus.h"

EUC_MODE
GetPayload (
  IN EFI_HANDLE ImageHandle
  )
{
  EFI_STATUS                      Status;
  EFI_LOADED_IMAGE_PROTOCOL       *LoadedImage;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Fs;
  EFI_FILE_PROTOCOL               *Root;
  EFI_FILE_PROTOCOL               *Handle;

  Status = gBS->HandleProtocol(
                  ImageHandle,
                  &gEfiLoadedImageProtocolGuid,
                  (VOID **)&LoadedImage
                );
  if (EFI_ERROR(Status)) {
    Print(L"-> cannot get LoadedImage: %r\n", Status);
    return MODE_UNKNOWN;
  }

  Status = gBS->HandleProtocol(
                  LoadedImage->DeviceHandle,
                  &gEfiSimpleFileSystemProtocolGuid,
                  (VOID **)&Fs
                );
  if (EFI_ERROR(Status)) {
    Print(L"-> cannot get filesystem: %r\n", Status);
    return MODE_UNKNOWN;
  }

  Status = Fs->OpenVolume(Fs, &Root);
  if (EFI_ERROR(Status)) {
    Print(L"-> cannot open volume: %r\n", Status);
    return MODE_UNKNOWN;
  }

  Status = Root->Open(
                   Root,
                   &Handle,
                   L"payload.bin",
                   EFI_FILE_MODE_READ,
                   0
                 );
  if (!EFI_ERROR(Status)) {
    Handle->Close(Handle);
    Print(L"-> detected payload type: BIN\n");
    return MODE_BIN;
  }

  Status = Root->Open(
                   Root,
                   &Handle,
                   L"payload.elf",
                   EFI_FILE_MODE_READ,
                   0
                 );
  if (!EFI_ERROR(Status)) {
    Handle->Close(Handle);
    Print(L"-> detected payload type: ELF\n");
    return MODE_ELF;
  }

  Print(L"-> No payload found\n");
  return MODE_UNKNOWN;
}

