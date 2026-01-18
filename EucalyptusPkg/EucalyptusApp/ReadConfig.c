#include "Eucalyptus.h"

EUC_CONFIG
LoadConfig (
  IN EFI_HANDLE ImageHandle
  )
{
  EFI_STATUS                      Status;
  EFI_LOADED_IMAGE_PROTOCOL       *LoadedImage;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
  EFI_FILE_PROTOCOL               *Root;
  EFI_FILE_PROTOCOL               *CfgFile;
  CHAR8                           Buffer[256];
  UINTN                           Size = sizeof(Buffer);

  EUC_CONFIG Cfg;
  Cfg.Mode      = MODE_UNKNOWN;
  Cfg.RelocBase = 0;
  Cfg.RelocSize = 0;

  Status = gBS->HandleProtocol(
                  ImageHandle,
                  &gEfiLoadedImageProtocolGuid,
                  (VOID **)&LoadedImage
                );
  if (EFI_ERROR(Status)) {
    Print(L"-> failed to get LoadedImage: %r\n", Status);
    return Cfg;
  }

  Status = gBS->HandleProtocol(
                  LoadedImage->DeviceHandle,
                  &gEfiSimpleFileSystemProtocolGuid,
                  (VOID **)&FileSystem
                );
  if (EFI_ERROR(Status)) {
    Print(L"-> failed to get filesystem: %r\n", Status);
    return Cfg;
  }

  Status = FileSystem->OpenVolume(FileSystem, &Root);
  if (EFI_ERROR(Status)) {
    Print(L"-> failed to open volume: %r\n", Status);
    return Cfg;
  }

  Status = Root->Open(
                   Root,
                   &CfgFile,
                   L"eucalyptus.cfg",
                   EFI_FILE_MODE_READ,
                   0
                 );
  if (EFI_ERROR(Status)) {
    Print(L"-> no eucalyptus.cfg found, fallbacking\n");
    Root->Close(Root);
    return Cfg;
  }

  Status = CfgFile->Read(CfgFile, &Size, Buffer);
  CfgFile->Close(CfgFile);
  Root->Close(Root);

  if (EFI_ERROR(Status)) {
    Print(L"-> failed to read config: %r\n", Status);
    return Cfg;
  }

  if (Size >= sizeof(Buffer))
    Size = sizeof(Buffer) - 1;
  Buffer[Size] = '\0';

  CHAR8 *Line = Buffer;
  CHAR8 *End;

  while ((End = AsciiStrStr(Line, "\n")) != NULL) {
      *End = '\0';
  
      if (AsciiStrnCmp(Line, "reloc_text_base=", 16) == 0) {
          Cfg.RelocBase = AsciiStrHexToUint64(Line + 16);
      } else if (AsciiStrnCmp(Line, "reloc_text_size=", 16) == 0) {
          Cfg.RelocSize = AsciiStrHexToUint64(Line + 16);
      }
  
      Line = End + 1;
  }
  
  Print(L"-> config: base=0x%lx size=0x%lx\n",
        Cfg.RelocBase, Cfg.RelocSize);
  
  return Cfg;
}

