
/* Copyright (c) 2012, The Linux Foundation. All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _PLATFORM_MSM_SHARED_QTMR_MMAP_H_
#define _PLATFORM_MSM_SHARED_QTMR_MMAP_H_

#define QTMR_BASE ((UINTN)PcdGet64(PcdQTimerBase))
#define QTMR_V1_CNTVCT_LO (0x00000000 + QTMR_BASE)
#define QTMR_V1_CNTVCT_HI (0x00000004 + QTMR_BASE)
#define QTMR_V1_CNTFRQ (0x00000010 + QTMR_BASE)
#define QTMR_V1_CNTV_CVAL_LO (0x00000030 + QTMR_BASE)
#define QTMR_V1_CNTV_CVAL_HI (0x00000034 + QTMR_BASE)
#define QTMR_V1_CNTV_TVAL (0x00000038 + QTMR_BASE)
#define QTMR_V1_CNTV_CTL (0x0000003C + QTMR_BASE)

#endif
