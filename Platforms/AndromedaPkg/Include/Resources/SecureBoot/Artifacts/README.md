# Aarch64 Secure Boot Defaults

This external dependency contains the default values suggested by microsoft the KEK, DB, and DBX UEFI variables.

Additionally, it contains an optional shared PK certificate that may be used as the root of trust for the system.
The shared PK certificate is an offering from Microsoft. Instead of a original equipment manufacturer (OEM)
managed PK, an OEM may choose to use the shared PK certificate managed by Microsoft. Partically, this may be
useful as default on non production code provided to an OEM by an indenpendent vendor (IV).

1. The PK (Platform Key) is a single certificate that is the root of trust for the system. This certificate is used
    to verify the KEK.
2. The KEK (Key Exchange Key) is a list of certificates that verify the signature of other keys attempting to update
   the DB and DBX.
3. The DB (Signature Database) is a list of certificates that verify the signature of a binary attempting to execute
   on the system.
4. The DBX (Forbidden Signature Database) is a list of signatures that are forbidden from executing on the system.

Please review [Microsoft's documentation](https://learn.microsoft.com/en-us/windows-hardware/manufacture/desktop/windows-secure-boot-key-creation-and-management-guidance?view=windows-11#15-keys-required-for-secure-boot-on-all-pcs)
for more information on key requirements if appending to the defaults provided in this external dependency.

## Folder Layout

### Artifacts
This folder contains the defaults in a EFI Signature List Format broken up by architecture. This format is used by the UEFI firmware to
initialize the secure boot variables. These files are in the format described by [EFI_SIGNATURE_DATA](https://uefi.org/specs/UEFI/2.10/32_Secure_Boot_and_Driver_Signing.html?highlight=authenticated%20variable#efi-signature-data)

## Artifacts/Imaging Folder
This folder contains the defaults in a format that may be used by imaging tools during imagine (such as tools that call
SetFirmwareVariableEx(..) like [WinPE](https://learn.microsoft.com/en-us/windows-hardware/manufacture/desktop/winpe-intro?view=windows-11))
to initialize the secure boot variables. These files have a authenticated variable header prepended to the EFI Signature List. However the
signature is not included. These variables are not signed but may be used to initialize the secure on systems that support this feature.

The additional data appended is a empty [EFI_VARIABLE_AUTHENTICATION_2](https://uefi.org/specs/UEFI/2.10/08_Services_Runtime_Services.html?highlight=efi_time#using-the-efi-variable-authentication-2-descriptor)
descriptor and is as follows:
[EFI_TIME](https://uefi.org/sites/default/files/resources/UEFI_Spec_2_8_final.pdf#page=158) +
[WIN_CERTIFICATE_UEFI_GUID](https://uefi.org/specs/UEFI/2.10/32_Secure_Boot_and_Driver_Signing.html?highlight=authenticated%20variable#win-certificate-uefi-guid) +
[PKCS7](https://tools.ietf.org/html/rfc2315#section-9.1) +
Data

Where the PKCS7 is a empty signature with the following ASN.1 structure:
```text
ContentInfo SEQUENCE (4 elem)
    contentType ContentType [?] INTEGER 1
    content [0] [?] SET (1 elem)
        ANY SEQUENCE (2 elem)
            OBJECT IDENTIFIER 2.16.840.1.101.3.4.2.1 sha-256 (NIST Algorithm)
            NULL
    SEQUENCE (1 elem)
        OBJECT IDENTIFIER 1.2.840.113549.1.7.1 data (PKCS #7)
    SET (0 elem)
```

For some firmware implementations, the PK is required to be at-least self signed during the imaging process.
However [Project Mu has a relaxed implementation](https://github.com/microsoft/mu_tiano_plus/blob/5c96768c404d1e4e32b1fea6bfd83e588c0f5d67/SecurityPkg/Library/AuthVariableLib/AuthService.c#L656C13-L656C52)
that allows for the PK to use an empty signature.


## DefaultPk

Contains the OEMA0 PK to enable signature database updates and binary execution.

Files Included:


## DefaultKek

Contains the Microsoft KEKs and OEMA0 KEK to enable signature database updates and binary execution.

Files Included:

* <https://go.microsoft.com/fwlink/?LinkId=321185>
* <https://go.microsoft.com/fwlink/?linkid=2239775>
* <https://github.com/WOA-Project/mu_andromeda_platforms>

## DefaultDb

Contains only Microsoft certificates to verify binaries before execution. More secure than Default3PDb.

Files Included:

* <https://go.microsoft.com/fwlink/p/?linkid=321192>
* <https://go.microsoft.com/fwlink/?linkid=2239776>

## Default3PDb

Contains Microsoft and UEFI third party certificates to verify binaries before execution. More compatible than
DefaultDb.

Files Included:

* <https://go.microsoft.com/fwlink/p/?linkid=321192>
* <https://go.microsoft.com/fwlink/?linkid=2239776>
* <https://go.microsoft.com/fwlink/p/?linkid=321194>
* <https://go.microsoft.com/fwlink/?linkid=2239872>
* <http://www.microsoft.com/pkiops/certs/microsoft%20option%20rom%20uefi%20ca%202023.crt>

## DefaultDbx

Contains a list of revoked certificates that will not execute on this system. Filtered per Architecture (ARM, Intel).

Files Included:

* <https://uefi.org/sites/default/files/resources/dbx_info.csv>

---

## License

Terms of Use for Microsoft Secure Boot Objects ("Secure Boot Objects")

By downloading the Secure Boot Objects, you agree to the following terms.
If you do not accept them, do not download or use the Secure Boot Objects.

These terms do not provide you with any legal rights to any intellectual
property in any Microsoft product.

You may copy and use the Secure Boot Objects for your internal, reference
purposes and to design, develop, and test your software, firmware or hardware,
as applicable; and you may distribute the Secure Boot Objects to end users
solely as part of the distribution of an operating system software product, or
as part of the distribution of updates to an operating system software product;
and you may distribute the Secure Boot Objects to end users or through your
distribution channels solely as embodied in a firmware product or hardware
product that embodies nontrivial additional functionality. Without limiting the
foregoing, copying or reproduction of the Secure Boot Objects to any other
server or location for further reproduction or redistribution on a standalone
basis is expressly prohibited.

If you are engaged in the business of developing and commercializing hardware
products that include the UEFI standard
(available at https://uefi.org/specifications), you may copy and use the Secure
Boot Objects for your internal, reference purposes and to design, develop, and
test your software; and you may distribute the Secure Boot Objects end users
solely as part of the distribution of an operating system software product, or
as part of the distribution of updates to an operating system software product.
Without limiting the foregoing, copying or reproduction of the Secure Boot
Objects to any other server or location for further reproduction or
redistribution on a standalone basis is expressly prohibited.
The Secure Boot Objects are provided “as-is.” The information contained in the
Secure Boot Objects may change without notice.  Microsoft does not represent
that the Secure Boot Objects is error free and you bear the entire risk of
using it.  NEITHER MICROSOFT NOR UEFI MAKES ANY WARRANTIES, EXPRESS OR IMPLIED,
WITH RESPECT TO THE SECURE BOOT OBJECTS, AND MICROSOFT AND UEFI EACH EXPRESSLY
DISCLAIMS ALL OTHER EXPRESS, IMPLIED, OR STATUTORY WARRANTIES.  THIS INCLUDES
THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND
NON-INFRINGEMENT.
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, IN NO EVENT SHALL MICROSOFT
OR UEFI BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY
DAMAGES WHATSOEVER ARISING OUT OF OR IN CONNECTION WITH THE USE OR DISTRIBUTION
OF THE SECURE BOOT OBJECTS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION.

YOU AGREE TO RELEASE MICROSOFT (INCLUDING ITS AFFLIATES, CONTRACTORS, AGENTS,
EMPLOYEES, LICENSEES AND ASSIGNEES) AND UEFI (INCLUDING ITS AFFILIATES,
CONTRACTORS, AGENTS, EMPLOYEES, LICENSEES AND SUCCESSORS) FROM ANY AND ALL
CLAIMS OR LIABILITY ARISING OUT OF YOUR USE OR DISTRIBUTION OF THE SECURE
BOOT OBJECTS AND ANY RELATED INFORMATION.
