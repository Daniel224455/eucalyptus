#include "Eucalyptus.h"

EUC_MODE
GetPayloadStage1 (
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
                   L"stage1.bin",
                   EFI_FILE_MODE_READ,
                   0
                 );
  if (!EFI_ERROR(Status)) {
    Handle->Close(Handle);
    Print(L"-> stage1 mode is BIN\n");
    return MODE_BIN;
  }

  Status = Root->Open(
                   Root,
                   &Handle,
                   L"stage1.elf",
                   EFI_FILE_MODE_READ,
                   0
                 );
  if (!EFI_ERROR(Status)) {
    Handle->Close(Handle);
    Print(L"-> stage1 mode is ELF\n");
    return MODE_ELF;
  }

  Print(L"-> No payload found\n");
  return MODE_UNKNOWN;
}

EUC_MODE
GetPayloadStage2 (
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
                   L"stage2.bin",
                   EFI_FILE_MODE_READ,
                   0
                 );
  if (!EFI_ERROR(Status)) {
    Handle->Close(Handle);
    Print(L"-> stage2 mode is BIN\n");
    return MODE_BIN;
  }

  Status = Root->Open(
                   Root,
                   &Handle,
                   L"stage2.elf",
                   EFI_FILE_MODE_READ,
                   0
                 );
  if (!EFI_ERROR(Status)) {
    Handle->Close(Handle);
    Print(L"-> stage2 mode is ELF\n");
    return MODE_ELF;
  }

  Print(L"-> No payload found\n");
  return MODE_UNKNOWN;
}

