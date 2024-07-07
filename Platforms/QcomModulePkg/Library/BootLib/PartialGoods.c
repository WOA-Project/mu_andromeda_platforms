/* Copyright (c) 2017,2019, 2020 The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * * Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided
 *  with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
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

#include "libfdt.h"
#include <Library/DebugLib.h>
#include <Library/LinuxLoaderLib.h>
#include <Library/PartialGoods.h>
#include <Protocol/EFIChipInfo.h>
#include <Protocol/EFIChipInfoTypes.h>
#include <Uefi/UefiBaseType.h>
#include <Library/FdtRw.h>

/* Look up table for cpu partial goods
 *
 * NOTE: Array size of PartialGoodsCpuType0 and
 *       PartialGoodsCpuType1 have to be same
 */
static struct PartialGoods PartialGoodsCpuType0[] = {
    {0x1, "/cpus", {"cpu@0", "enable-method", "psci", "none"}},
    {0x2, "/cpus", {"cpu@100", "enable-method", "psci", "none"}},
    {0x4, "/cpus", {"cpu@200", "enable-method", "psci", "none"}},
    {0x8, "/cpus", {"cpu@300", "enable-method", "psci", "none"}},
    {0x10, "/cpus", {"cpu@400", "enable-method", "psci", "none"}},
    {0x20, "/cpus", {"cpu@500", "enable-method", "psci", "none"}},
    {0x40, "/cpus", {"cpu@600", "enable-method", "psci", "none"}},
    {0x80, "/cpus", {"cpu@700", "enable-method", "psci", "none"}},
};

/* Look up table for cpu partial goods 
 *
 * NOTE: Array size of PartialGoodsCpuType0 and
 *       PartialGoodsCpuType1 have to be same
 */
static struct PartialGoods PartialGoodsCpuType1[] = {
    {0x1, "/cpus", {"cpu@101", "enable-method", "psci", "none"}},
    {0x2, "/cpus", {"cpu@102", "enable-method", "psci", "none"}},
    {0x4, "/cpus", {"cpu@103", "enable-method", "psci", "none"}},
    {0x8, "/cpus", {"cpu@104", "enable-method", "psci", "none"}},
    {0x10, "/cpus", {"cpu@105", "enable-method", "psci", "none"}},
    {0x20, "/cpus", {"cpu@106", "enable-method", "psci", "none"}},
    {0x30, "/cpus", {"cpu@107", "enable-method", "psci", "none"}},
    {0x40, "/cpus", {"cpu@108", "enable-method", "psci", "none"}},
};

#define NUM_OF_CPUS (ARRAY_SIZE(PartialGoodsCpuType0))

STATIC struct PartialGoods *PartialGoodsCpuType[MAX_CPU_CLUSTER] = {
    PartialGoodsCpuType0, PartialGoodsCpuType1};

/* Look up table for multimedia partial goods */
static struct PartialGoods PartialGoodsMmType[] = {
    {BIT (EFICHIPINFO_PART_GPU),
     "/soc",
     {"qcom,kgsl-3d0", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_GPU),
     "/soc",
     {"qcom,gmu", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_GPU),
     "/soc",
     {"kgsl-smmu", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_GPU),
     "/soc",
     {"qcom,gpucc", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_VIDEO),
     "/soc",
     {"qcom,vidc", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_VIDEO),
     "/soc",
     {"qcom,vidc0", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_VIDEO),
     "/soc",
     {"qcom,vidc1", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_VIDEO),
     "/soc",
     {"qcom,videocc", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,cam-req-mgr", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,msm-cam", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,csiphy", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,csiphy0", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,csiphy1", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,csiphy2", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,csiphy3", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,csiphy4", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,csiphy5", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,csid0", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,csid1", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,csid-lite0", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,csid-lite1", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,cam_smmu", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,cam-cpas", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,cam-cdm-intf", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,cpas-cdm0", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,cpas-cdm1", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,cpas-cdm2", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,cam-cpas", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,cam-fd", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,fd", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,ife0", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,ife1", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,ife-lite0", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,ife-lite1", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,cci", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,cci0", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,cci1", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,jpegenc", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,jpegdma", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,camera-flash0", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,camera-flash1", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,cam-icp", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,cam-isp", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,a5", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,ipe0", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,bps", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,cam-jpeg", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,lrme", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,ipe1", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,vfe0", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,vfe1", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_CAMERA),
     "/soc",
     {"qcom,camcc", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_DISPLAY),
     "/soc",
     {"qcom,mdss_mdp", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_DISPLAY),
     "/soc",
     {"qcom,mdss_rotator", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_DISPLAY),
     "/soc",
     {"qcom,mdss_dsi0_ctrl", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_DISPLAY),
     "/soc",
     {"qcom,mdss_dsi1_ctrl", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_DISPLAY),
     "/soc",
     {"qcom,mdss_dsi_phy0", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_DISPLAY),
     "/soc",
     {"qcom,mdss_dsi_phy1", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_DISPLAY),
     "/soc",
     {"qcom,mdss_dsi0_pll", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_DISPLAY),
     "/soc",
     {"qcom,mdss_dsi1_pll", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_DISPLAY),
     "/soc",
     {"qcom,mdss_dsi_pll", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_DISPLAY),
     "/soc",
     {"qcom,mdss_dp_pll", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_DISPLAY),
     "/soc",
     {"qcom,msm-ext-disp", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_DISPLAY),
     "/soc",
     {"qcom,sde_rscc", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_DISPLAY),
     "/soc",
     {"qcom,dp_display", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_DISPLAY),
     "/soc",
     {"qcom,dispcc", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_AUDIO),
     "/soc",
     {"qcom,msm-adsp-loader", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_AUDIO),
     "/soc",
     {"qcom,lpass", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_AUDIO),
     "/soc",
     {"qcom,msm-adsprpc-mem", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_AUDIO),
     "/soc",
     {"qcom,msm_fastrpc", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_MODEM),
     "/soc",
     {"qcom,mss", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_WLAN),
     "/soc",
     {"qcom,mss", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_COMP),
     "/soc",
     {"qcom,turing", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_COMP),
     "/soc",
     {"qcom,msm-adsprpc-mem", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_COMP),
     "/soc",
     {"qcom,msm_fastrpc", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_SENSORS),
     "/soc",
     {"qcom,msm-adsprpc-mem", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_SENSORS),
     "/soc",
     {"qcom,msm_fastrpc", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_SENSORS),
     "/soc",
     {"qcom,ssc", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_NPU),
     "/soc",
     {"qcom,npucc", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_NPU),
     "/soc",
     {"qcom,npu", "status", "ok", "no"}},
    {BIT (EFICHIPINFO_PART_NAV),
     "/soc",
     {"qcom,mss", "status", "ok", "no"}},
};

STATIC VOID
FindNodeAndUpdateProperty (VOID *fdt,
                           UINT32 TableSz,
                           struct PartialGoods *Table,
                           UINT32 Value)
{
  struct SubNodeListNew *SNode = NULL;
  INT32 SubNodeOffset = 0;
  INT32 ParentOffset = 0;
  INT32 Ret = 0;
  UINT32 i;

  for (i = 0; i < TableSz; i++, Table++) {
    if (!(Value & Table->Val))
      continue;

    /* Find the parent node */
    ParentOffset = FdtPathOffset (fdt, Table->ParentNode);
    if (ParentOffset < 0) {
      DEBUG ((EFI_D_ERROR, "Failed to Get parent node: %a\terror: %d\n",
              Table->ParentNode, ParentOffset));
      continue;
    }

    /* Find the subnode */
    SNode = &(Table->SubNode);
    SubNodeOffset = fdt_subnode_offset (fdt, ParentOffset,
                                      SNode->SubNodeName);
    if (SubNodeOffset < 0) {
      DEBUG ((EFI_D_INFO, "Subnode: %a is not present, ignore\n",
              SNode->SubNodeName));
      continue;
    }

     /* Add/Replace the property with Replace string value */
    Ret = FdtSetProp (fdt, SubNodeOffset, SNode->PropertyName,
                      (CONST VOID *)SNode->ReplaceStr,
                      AsciiStrLen (SNode->ReplaceStr) + 1);
    if (!Ret) {
      DEBUG ((EFI_D_INFO, "Partial goods (%a) status property disabled\n",
              SNode->SubNodeName));
    } else {
      DEBUG ((EFI_D_ERROR, "Failed to update property: %a, ret =%d \n",
              SNode->PropertyName, Ret));
    }
  }
}

STATIC EFI_STATUS
ReadCpuPartialGoods (EFI_CHIPINFO_PROTOCOL *pChipInfoProtocol, UINT32 *Value)
{
  UINT32 i;
  EFI_STATUS Status = EFI_SUCCESS;
  UINT32 DefectVal;

  for (i = 0; i < MAX_CPU_CLUSTER; i++) {
    /* Ensure to reset the Value before checking CPU part for defect */
    DefectVal = 0;
    Value[i] = 0;

    Status =
        pChipInfoProtocol->GetDefectiveCPUs (pChipInfoProtocol, i, &DefectVal);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_VERBOSE, "Failed to get CPU defective[%d] part. %r\n", i,
              Status));
      continue;
    }

    Value[i] = DefectVal;
  }

  if (Status == EFI_NOT_FOUND)
    Status = EFI_SUCCESS;

  return Status;
}

STATIC EFI_STATUS
ReadMMPartialGoods (EFI_CHIPINFO_PROTOCOL *pChipInfoProtocol, UINT32 *Value)
{
  UINT32 i;
  EFI_STATUS Status = EFI_SUCCESS;
  UINT32 DefectVal;

  *Value = 0;
  for (i = 1; i < EFICHIPINFO_NUM_PARTS; i++) {
    /* Ensure to reset the Value before checking for defect Part*/
    DefectVal = 0;

    Status =
        pChipInfoProtocol->GetDefectivePart (pChipInfoProtocol, i, &DefectVal);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_VERBOSE, "Failed to get MM defective[%d] part. %r\n", i,
              Status));
      continue;
    }

    *Value |= (DefectVal << i);
  }

  if (Status == EFI_NOT_FOUND)
    Status = EFI_SUCCESS;

  return Status;
}

EFI_STATUS
UpdatePartialGoodsNode (VOID *fdt)
{
  UINT32 i;
  UINT32 PartialGoodsMMValue = 0;
  UINT32 PartialGoodsCpuValue[MAX_CPU_CLUSTER];
  EFI_CHIPINFO_PROTOCOL *pChipInfoProtocol;
  EFI_STATUS Status = EFI_SUCCESS;

  Status = gBS->LocateProtocol (&gEfiChipInfoProtocolGuid, NULL,
                                (VOID **)&pChipInfoProtocol);
  if (EFI_ERROR (Status))
    return Status;

  if (pChipInfoProtocol->Revision < EFI_CHIPINFO_PROTOCOL_REVISION)
    return Status;

  /* Read and update Multimedia Partial Goods Nodes */
  Status = ReadMMPartialGoods (pChipInfoProtocol, &PartialGoodsMMValue);
  if (Status != EFI_SUCCESS) {
    DEBUG ((EFI_D_INFO, "No mm partial goods found.\n"));
  }

  if (PartialGoodsMMValue) {
    DEBUG ((EFI_D_INFO, "PartialGoods for Multimedia: 0x%x\n",
            PartialGoodsMMValue));

    FindNodeAndUpdateProperty (fdt, ARRAY_SIZE (PartialGoodsMmType),
                               &PartialGoodsMmType[0], PartialGoodsMMValue);
  }

  /* Read and update CPU Partial Goods nodes */
  Status = ReadCpuPartialGoods (pChipInfoProtocol, PartialGoodsCpuValue);
  if (Status != EFI_SUCCESS) {
    DEBUG ((EFI_D_INFO, "No partial goods for cpu ss found.\n"));
  }

  for (i = 0; i < MAX_CPU_CLUSTER; i++) {
    if (PartialGoodsCpuValue[i]) {
      DEBUG ((EFI_D_INFO, "PartialGoods for Cluster[%d]: 0x%x\n", i,
              PartialGoodsCpuValue[i]));
      FindNodeAndUpdateProperty (fdt, NUM_OF_CPUS,
                                 &PartialGoodsCpuType[i][0],
                                 PartialGoodsCpuValue[i]);
    }
  }

  return EFI_SUCCESS;
}
