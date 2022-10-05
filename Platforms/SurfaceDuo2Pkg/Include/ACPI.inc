FILE FREEFORM = 7E374E25-8E01-4FEE-87F2-390C23C606CD {
!if $(PREFER_MPPARK_OVER_SMC_PSCI) == 1
	SECTION RAW = SurfaceDuo2Pkg/AcpiTables/8350-min/builtin/MADT.aml
!else
	SECTION RAW = SurfaceDuo2Pkg/AcpiTables/8350/builtin/APIC.aml
!endif
# 	SECTION RAW = SurfaceDuo2Pkg/AcpiTables/8350/builtin/BGRT.aml
	SECTION RAW = SurfaceDuo2Pkg/AcpiTables/8350/builtin/CSRT.aml
	SECTION RAW = SurfaceDuo2Pkg/AcpiTables/8350/builtin/DBG2.aml
	SECTION RAW = SurfaceDuo2Pkg/AcpiTables/8350/builtin/DSDT.aml
!if $(PREFER_MPPARK_OVER_SMC_PSCI) == 1
	SECTION RAW = SurfaceDuo2Pkg/AcpiTables/8350-min/builtin/FADT.aml
!else
	SECTION RAW = SurfaceDuo2Pkg/AcpiTables/8350/builtin/FACP.aml
!endif
	SECTION RAW = SurfaceDuo2Pkg/AcpiTables/8350/builtin/GTDT.aml
	SECTION RAW = SurfaceDuo2Pkg/AcpiTables/8350/builtin/IORT.aml
	SECTION RAW = SurfaceDuo2Pkg/AcpiTables/8350/builtin/MCFG.aml
	SECTION RAW = SurfaceDuo2Pkg/AcpiTables/8350/builtin/PPTT.aml
	SECTION RAW = SurfaceDuo2Pkg/AcpiTables/8350/builtin/SPCR.aml
	SECTION RAW = SurfaceDuo2Pkg/AcpiTables/8350/builtin/TPM2.aml
	SECTION UI = "AcpiTables"
}