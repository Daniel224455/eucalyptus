#ifndef _EUCALYPTUS_H_
#define _EUCALYPTUS_H_

#include <Uefi.h>
#include <Library/ArmLib.h>
#include <Library/ArmMmuLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/IoLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h> 
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DevicePathLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>

//
// ELF structs and typedefs
//
typedef struct {
  UINT8   e_ident[16];
  UINT16  e_type;
  UINT16  e_machine;
  UINT32  e_version;
  UINT64  e_entry;
  UINT64  e_phoff;
  UINT64  e_shoff;
  UINT32  e_flags;
  UINT16  e_ehsize;
  UINT16  e_phentsize;
  UINT16  e_phnum;
  UINT16  e_shentsize;
  UINT16  e_shnum;
  UINT16  e_shstrndx;
} ELF64_EHDR;

typedef struct {
  UINT32  p_type;
  UINT32  p_flags;
  UINT64  p_offset;
  UINT64  p_vaddr;
  UINT64  p_paddr;
  UINT64  p_filesz;
  UINT64  p_memsz;
  UINT64  p_align;
} ELF64_PHDR;

#define PT_LOAD 1

typedef enum {
  MODE_ELF,
  MODE_BIN,
  MODE_UNKNOWN
} EUC_MODE;

typedef struct {
  EUC_MODE  Mode;
  UINT64    RelocBase;
  UINT64    RelocSize;
} EUC_CONFIG;

//
// Function prototypes
//
UINTN 
CheckPrivileges (
  VOID
  );

EUC_CONFIG
LoadConfig (
  IN EFI_HANDLE        ImageHandle
  );

EUC_MODE
GetPayloadStage1 (
  IN EFI_HANDLE ImageHandle
  );

EUC_MODE
GetPayloadStage2 (
  IN EFI_HANDLE ImageHandle
  );  

EFI_STATUS
EFIAPI
Stage1ParseAndLoad (
  IN EFI_HANDLE        ImageHandle,
  IN UINT64            Base,
  IN UINT64            Size
  );

EFI_STATUS
EFIAPI
Stage2ParseAndLoad (
  IN EFI_HANDLE        ImageHandle,
  IN UINT64            Base,
  IN UINT64            Size
  );

EFI_STATUS
EFIAPI
LoadStage1Elf (
  IN EFI_HANDLE        ImageHandle,
  IN UINT64            Base,
  IN UINT64            Size  
  );

EFI_STATUS
EFIAPI
LoadStage2Elf (
  IN EFI_HANDLE        ImageHandle,
  IN UINT64            Base,
  IN UINT64            Size  
  );

#endif // _EUCALYPTUS_H_