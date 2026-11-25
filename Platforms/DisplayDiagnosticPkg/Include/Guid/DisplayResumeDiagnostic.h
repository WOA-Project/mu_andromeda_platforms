/** @file
  Early display state captured without changing the hardware.

  SPDX-License-Identifier: MIT
**/
#ifndef DISPLAY_RESUME_DIAGNOSTIC_H_
#define DISPLAY_RESUME_DIAGNOSTIC_H_

#include <Uefi.h>

#define DISPLAY_DIAGNOSTIC_SIGNATURE      SIGNATURE_32 ('D', 'S', 'R', 'D')
#define DISPLAY_DIAGNOSTIC_VERSION        6
#define DISPLAY_DIAGNOSTIC_MAX_REGISTERS  512
#define DISPLAY_FRAMEBUFFER_SAMPLES       512
#define DISPLAY_XBL_CHUNKS                9
#define DISPLAY_XBL_CHUNK_SIZE            0x00400000

typedef struct {
  UINT64 Address;
  UINT32 Value;
  UINT32 Read;
} DISPLAY_DIAGNOSTIC_REGISTER;

typedef struct {
  UINT32 SampleCount;
  UINT32 NonzeroSamples;
  UINT32 Transitions;
  UINT32 Checksum;
  UINT32 FirstValue;
  UINT32 LastValue;
} DISPLAY_MEMORY_SAMPLES;

typedef struct {
  UINT32                      Signature;
  UINT32                      Version;
  UINT32                      RegisterCount;
  UINT32                      MdssAccessible;
  EFI_STATUS                  CaptureStatus;
  EFI_STATUS                  FramebufferStatus;
  UINT64                      FramebufferAddress;
  UINT64                      FramebufferRegionSize;
  UINT32                      FramebufferWidth;
  UINT32                      FramebufferHeight;
  UINT32                      FramebufferBitsPerPixel;
  DISPLAY_MEMORY_SAMPLES      FramebufferSamples;
  UINT64                      XblDisplayAddress;
  DISPLAY_MEMORY_SAMPLES      XblDisplaySamples[DISPLAY_XBL_CHUNKS];
  DISPLAY_DIAGNOSTIC_REGISTER Registers[DISPLAY_DIAGNOSTIC_MAX_REGISTERS];
} DISPLAY_RESUME_DIAGNOSTIC;

extern EFI_GUID gDisplayResumeDiagnosticHobGuid;

#endif
