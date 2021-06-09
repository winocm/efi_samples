/*-----------------------------------------------------------------------
        File:   oidsattr.h
  
Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

  -----------------------------------------------------------------------
*/
/* 
 * WARNING: EXPORT RESTRICTED. 
 * This software is subject to the U.S. Export Administration Regulations 
 * and other U.S. law, and may not be exported or re-exported to certain 
 * countries (currently Afghanistan (Taliban-controlled areas), Cuba, Iran, 
 * Iraq, Libya, North Korea, Serbia (except Kosovo), Sudan and Syria) or to 
 * persons or entities prohibited from receiving U.S. exports (including Denied 
 * Parties, Specially Designated Nationals, and entities on the Bureau of 
 * Export Administration Entity List or involved with missile technology or 
 * nuclear, chemical or biological weapons).
 */ 

#ifndef _OIDSATTR_H
#define _OIDSATTR_H

#include "cssm.h"
#include "oidsbase.h"

/* From X.520 */
static uint8    
OID_ObjectClass[]           = { OID_ATTR_TYPE, 0 },
OID_AliasedEntryName[]      = { OID_ATTR_TYPE, 1 },
OID_KnowledgeInformation[]  = { OID_ATTR_TYPE, 2 }, 
OID_CommonName[]            = { OID_ATTR_TYPE, 3 },
OID_Surname[]               = { OID_ATTR_TYPE, 4 },
OID_SerialNumber[]          = { OID_ATTR_TYPE, 5 },
OID_CountryName[]           = { OID_ATTR_TYPE, 6 },
OID_LocalityName[]          = { OID_ATTR_TYPE, 7 },
OID_StateProvinceName[]     = { OID_ATTR_TYPE, 8 },
OID_CollectiveStateProvinceName[] = { OID_ATTR_TYPE, 8, 1 },
OID_StreetAddress[]         = { OID_ATTR_TYPE, 9 },
OID_CollectiveStreetAddress[]     = { OID_ATTR_TYPE, 9, 1 },
OID_OrganizationName[]      = { OID_ATTR_TYPE, 10 },
OID_CollectiveOrganizationName[]  = { OID_ATTR_TYPE, 10, 1 },
OID_OrganizationalUnitName[]= { OID_ATTR_TYPE, 11 }, 
OID_CollectiveOrganizationalUnitName[] = { OID_ATTR_TYPE, 11, 1 }, 
OID_Title[]                 = { OID_ATTR_TYPE, 12 },
OID_Description[]           = { OID_ATTR_TYPE, 13 },
OID_SearchGuide[]           = { OID_ATTR_TYPE, 14 },
OID_BusinessCategory[]      = { OID_ATTR_TYPE, 15 },
OID_PostalAddress[]         = { OID_ATTR_TYPE, 16 },
OID_CollectivePostalAddress[]     = { OID_ATTR_TYPE, 16, 1 },
OID_PostalCode[]            = { OID_ATTR_TYPE, 17 },
OID_CollectivePostalCode[]  = { OID_ATTR_TYPE, 17, 1 },
OID_PostOfficeBox[]         = { OID_ATTR_TYPE, 18 },
OID_CollectivePostOfficeBox[]     = { OID_ATTR_TYPE, 18, 1 },
OID_PhysicalDeliveryOfficeName[]  = { OID_ATTR_TYPE, 19 }, 
OID_CollectivePhysicalDeliveryOfficeName[] = { OID_ATTR_TYPE, 19, 1 }, 
OID_TelephoneNumber[]       = { OID_ATTR_TYPE, 20 },
OID_CollectiveTelephoneNumber[]   = { OID_ATTR_TYPE, 20, 1 },
OID_TelexNumber[]           = { OID_ATTR_TYPE, 21 },
OID_CollectiveTelexNumber[] = { OID_ATTR_TYPE, 21, 1 },
OID_TelexTerminalIdentifier[]     = { OID_ATTR_TYPE, 22 }, 
OID_CollectiveTelexTerminalIdentifier[] = { OID_ATTR_TYPE, 22, 1 }, 
OID_FacsimileTelephoneNumber[]    = { OID_ATTR_TYPE, 23 }, 
OID_CollectiveFacsimileTelephoneNumber[] = { OID_ATTR_TYPE, 23, 1 }, 
OID_X_121Address[]          = { OID_ATTR_TYPE, 24 },
OID_InternationalISDNNumber[]     = { OID_ATTR_TYPE, 25 }, 
OID_CollectiveInternationalISDNNumber[] = { OID_ATTR_TYPE, 25, 1 }, 
OID_RegisteredAddress[]     = { OID_ATTR_TYPE, 26 },
OID_DestinationIndicator[]  = { OID_ATTR_TYPE, 27 }, 
OID_PreferredDeliveryMethod[] = { OID_ATTR_TYPE, 28 }, 
OID_PresentationAddress[]   = { OID_ATTR_TYPE, 29 }, 
OID_SupportedApplicationContext[] = { OID_ATTR_TYPE, 30 }, 
OID_Member[]                = { OID_ATTR_TYPE, 31 },
OID_Owner[]                 = { OID_ATTR_TYPE, 32 },
OID_RoleOccupant[]          = { OID_ATTR_TYPE, 33 },
OID_SeeAlso[]               = { OID_ATTR_TYPE, 34 },
OID_UserPassword[]          = { OID_ATTR_TYPE, 35 },
OID_UserCertificate[]       = { OID_ATTR_TYPE, 36 },
OID_CACertificate[]         = { OID_ATTR_TYPE, 37 },
OID_AuthorityRevocationList[] = { OID_ATTR_TYPE, 38 }, 
OID_CertificateRevocationList[] = { OID_ATTR_TYPE, 39 }, 
OID_CrossCertificatePair[]  = { OID_ATTR_TYPE, 40 },
OID_Name[]                  = { OID_ATTR_TYPE, 41 },
OID_GivenName[]             = { OID_ATTR_TYPE, 42 },
OID_Initials[]              = { OID_ATTR_TYPE, 43 },
OID_GenerationQualifier[]   = { OID_ATTR_TYPE, 44 },
OID_UniqueIdentifier[]      = { OID_ATTR_TYPE, 45 },
OID_DNQualifier[]           = { OID_ATTR_TYPE, 46 },
OID_EnhancedSearchGuide[]   = { OID_ATTR_TYPE, 47 },
OID_ProtocolInformation[]   = { OID_ATTR_TYPE, 48 },
OID_DistinguishedName[]     = { OID_ATTR_TYPE, 49 },
OID_UniqueMember[]          = { OID_ATTR_TYPE, 50 },
OID_HouseIdentifier[]       = { OID_ATTR_TYPE, 51 };

/* From PKCS 9 */
static uint8    
OID_EmailAddress[]          = { OID_PKCS_9, 1 },
OID_UnstructuredName[]      = { OID_PKCS_9, 2 }, 
OID_ContentType[]           = { OID_PKCS_9, 3 },
OID_MessageDigest[]         = { OID_PKCS_9, 4 },
OID_SigningTime[]           = { OID_PKCS_9, 5 },
OID_CounterSignature[]      = { OID_PKCS_9, 6 }, 
OID_ChallengePassword[]     = { OID_PKCS_9, 7 }, 
OID_UnstructuredAddress[]   = { OID_PKCS_9, 8 }, 
OID_ExtendedCertificateAttributes[] = { OID_PKCS_9, 9 };

/* From PKIX 1 */
/* Standard Extensions */
static uint8
OID_SubjectDirectoryAttributes[] = { OID_EXTENSION, 9 },
OID_SubjectKeyIdentifier[]  = { OID_EXTENSION, 14 },
OID_KeyUsage[]              = { OID_EXTENSION, 15 },
OID_PrivateKeyUsagePeriod[] = { OID_EXTENSION, 16 },
OID_SubjectAltName[]        = { OID_EXTENSION, 17 },
OID_IssuerAltName[]         = { OID_EXTENSION, 18 },
OID_BasicConstraints[]      = { OID_EXTENSION, 19 },
OID_CrlNumber[]             = { OID_EXTENSION, 20 },
OID_CrlReason[]             = { OID_EXTENSION, 21 },
OID_HoldInstructionCode[]   = { OID_EXTENSION, 23 },
OID_InvalidityDate[]        = { OID_EXTENSION, 24 },
OID_DeltaCrlIndicator[]     = { OID_EXTENSION, 27 },
OID_IssuingDistributionPoints[]     = { OID_EXTENSION, 28 },
OID_NameConstraints[]       = { OID_EXTENSION, 30 },
OID_CrlDistributionPoints[] = { OID_EXTENSION, 31 },
OID_CertificatePolicies[]   = { OID_EXTENSION, 32 },
OID_PolicyMappings[]        = { OID_EXTENSION, 33 },
OID_PolicyConstraints[]     = { OID_EXTENSION, 34 },
OID_AuthorityKeyIdentifier[]= { OID_EXTENSION, 35 };

/* From PKIX 1 */
/* Private Internet Extensions */
static uint8
OID_FtpID[]  = { OID_APPL_TCP_PROTO, 21},
OID_HttpID[] = { OID_APPL_TCP_PROTO, 80},
OID_SmtpID[] = { OID_APPL_TCP_PROTO, 25},
OID_LdapID[] = { OID_APPL_TCP_PROTO, 131, 5};

static uint8
OID_SubjectInfoAccess[]     = { OID_PKIX, 1},
OID_AuthorityInfoAccess[]   = { OID_PKIX, 2},
OID_CaInfoAccess[]          = { OID_PKIX, 3};

static uint8
OID_HoldInstructionNone[]   = { OID_X9CM_INSTRUCTION, 1},
OID_HoldInstructionCallIssuer[]  = { OID_X9CM_INSTRUCTION, 2},
OID_HoldInstructionReject[] = { OID_X9CM_INSTRUCTION, 3};


/* per Microsoft CAPI 2.0 API reference */
/* Extension OIDs */
/*
static uint8
OID_AUTHORITY_KEY_IDENTIFIER[]  = { OID_EXTENSION, 1 },
OID_KEY_ATTRIBUTES[]            = { OID_EXTENSION, 2 },
OID_KEY_USAGE_RESTRICTION[]     = { OID_EXTENSION, 4 },
OID_SUBJECT_ALT_NAME[]          = { OID_EXTENSION, 7 },
OID_BASIC_CONSTRAINTS[]         = { OID_EXTENSION, 10 };
*/

/* From X.520 */
static CSSM_OID
CSSMOID_ObjectClass         = {OID_ATTR_TYPE_LENGTH+1, OID_ObjectClass},
CSSMOID_AliasedEntryName    = {OID_ATTR_TYPE_LENGTH+1, OID_AliasedEntryName},
CSSMOID_KnowledgeInformation= {OID_ATTR_TYPE_LENGTH+1, OID_KnowledgeInformation},
CSSMOID_CommonName          = {OID_ATTR_TYPE_LENGTH+1, OID_CommonName},
CSSMOID_Surname             = {OID_ATTR_TYPE_LENGTH+1, OID_Surname},
CSSMOID_SerialNumber        = {OID_ATTR_TYPE_LENGTH+1, OID_SerialNumber},
CSSMOID_CountryName         = {OID_ATTR_TYPE_LENGTH+1, OID_CountryName},
CSSMOID_LocalityName        = {OID_ATTR_TYPE_LENGTH+1, OID_LocalityName},
CSSMOID_StateProvinceName   = {OID_ATTR_TYPE_LENGTH+1, OID_StateProvinceName},
CSSMOID_CollectiveStateProvinceName = {OID_ATTR_TYPE_LENGTH+2, OID_CollectiveStateProvinceName},
CSSMOID_StreetAddress       = {OID_ATTR_TYPE_LENGTH+1, OID_StreetAddress},
CSSMOID_CollectiveStreetAddress     = {OID_ATTR_TYPE_LENGTH+2, OID_CollectiveStreetAddress},
CSSMOID_OrganizationName    = {OID_ATTR_TYPE_LENGTH+1, OID_OrganizationName},
CSSMOID_CollectiveOrganizationName  = {OID_ATTR_TYPE_LENGTH+2, OID_CollectiveOrganizationName},
CSSMOID_OrganizationalUnitName      = {OID_ATTR_TYPE_LENGTH+1, OID_OrganizationalUnitName},
CSSMOID_CollectiveOrganizationalUnitName= {OID_ATTR_TYPE_LENGTH+2, OID_CollectiveOrganizationalUnitName},
CSSMOID_Title               = {OID_ATTR_TYPE_LENGTH+1, OID_Title},
CSSMOID_Description         = {OID_ATTR_TYPE_LENGTH+1, OID_Description},
CSSMOID_SearchGuide         = {OID_ATTR_TYPE_LENGTH+1, OID_SearchGuide},
CSSMOID_BusinessCategory    = {OID_ATTR_TYPE_LENGTH+1, OID_BusinessCategory},
CSSMOID_PostalAddress       = {OID_ATTR_TYPE_LENGTH+1, OID_PostalAddress},
CSSMOID_CollectivePostalAddress     = {OID_ATTR_TYPE_LENGTH+2, OID_CollectivePostalAddress},
CSSMOID_PostalCode          = {OID_ATTR_TYPE_LENGTH+1, OID_PostalCode},
CSSMOID_CollectivePostalCode= {OID_ATTR_TYPE_LENGTH+2, OID_CollectivePostalCode},
CSSMOID_PostOfficeBox       = {OID_ATTR_TYPE_LENGTH+1, OID_PostOfficeBox},
CSSMOID_CollectivePostOfficeBox     = {OID_ATTR_TYPE_LENGTH+2, OID_CollectivePostOfficeBox},
CSSMOID_PhysicalDeliveryOfficeName  = {OID_ATTR_TYPE_LENGTH+1, OID_PhysicalDeliveryOfficeName},
CSSMOID_CollectivePhysicalDeliveryOfficeName = {OID_ATTR_TYPE_LENGTH+2, OID_CollectivePhysicalDeliveryOfficeName},
CSSMOID_TelephoneNumber     = {OID_ATTR_TYPE_LENGTH+1, OID_TelephoneNumber},
CSSMOID_CollectiveTelephoneNumber   = {OID_ATTR_TYPE_LENGTH+2, OID_CollectiveTelephoneNumber},
CSSMOID_TelexNumber         = {OID_ATTR_TYPE_LENGTH+1, OID_TelexNumber},
CSSMOID_CollectiveTelexNumber       = {OID_ATTR_TYPE_LENGTH+2, OID_CollectiveTelexNumber},
CSSMOID_TelexTerminalIdentifier     = {OID_ATTR_TYPE_LENGTH+1, OID_TelexTerminalIdentifier},
CSSMOID_CollectiveTelexTerminalIdentifier = {OID_ATTR_TYPE_LENGTH+2, OID_CollectiveTelexTerminalIdentifier},
CSSMOID_FacsimileTelephoneNumber    = {OID_ATTR_TYPE_LENGTH+1, OID_FacsimileTelephoneNumber},
CSSMOID_CollectiveFacsimileTelephoneNumber= {OID_ATTR_TYPE_LENGTH+2, },
CSSMOID_X_121Address        = {OID_ATTR_TYPE_LENGTH+1, OID_X_121Address},
CSSMOID_InternationalISDNNumber     = {OID_ATTR_TYPE_LENGTH+1, OID_InternationalISDNNumber},
CSSMOID_CollectiveInternationalISDNNumber = {OID_ATTR_TYPE_LENGTH+2, OID_CollectiveInternationalISDNNumber},
CSSMOID_RegisteredAddress   = {OID_ATTR_TYPE_LENGTH+1, OID_RegisteredAddress},
CSSMOID_DestinationIndicator= {OID_ATTR_TYPE_LENGTH+1, OID_DestinationIndicator},
CSSMOID_PreferredDeliveryMethod     = {OID_ATTR_TYPE_LENGTH+1, OID_PreferredDeliveryMethod},
CSSMOID_PresentationAddress = {OID_ATTR_TYPE_LENGTH+1, OID_PresentationAddress},
CSSMOID_SupportedApplicationContext = {OID_ATTR_TYPE_LENGTH+1, OID_SupportedApplicationContext},
CSSMOID_Member              = {OID_ATTR_TYPE_LENGTH+1, OID_Member},
CSSMOID_Owner               = {OID_ATTR_TYPE_LENGTH+1, OID_Owner},
CSSMOID_RoleOccupant        = {OID_ATTR_TYPE_LENGTH+1, OID_RoleOccupant},
CSSMOID_SeeAlso             = {OID_ATTR_TYPE_LENGTH+1, OID_SeeAlso},
CSSMOID_UserPassword        = {OID_ATTR_TYPE_LENGTH+1, OID_UserPassword},
CSSMOID_UserCertificate     = {OID_ATTR_TYPE_LENGTH+1, OID_UserCertificate},
CSSMOID_CACertificate       = {OID_ATTR_TYPE_LENGTH+1, OID_CACertificate},
CSSMOID_AuthorityRevocationList     = {OID_ATTR_TYPE_LENGTH+1, OID_AuthorityRevocationList},
CSSMOID_CertificateRevocationList   = {OID_ATTR_TYPE_LENGTH+1, OID_CertificateRevocationList},
CSSMOID_CrossCertificatePair= {OID_ATTR_TYPE_LENGTH+1, OID_CrossCertificatePair},
CSSMOID_Name                = {OID_ATTR_TYPE_LENGTH+1, OID_Name},
CSSMOID_GivenName           = {OID_ATTR_TYPE_LENGTH+1, OID_GivenName},
CSSMOID_Initials            = {OID_ATTR_TYPE_LENGTH+1, OID_Initials},
CSSMOID_GenerationQualifier = {OID_ATTR_TYPE_LENGTH+1, OID_GenerationQualifier},
CSSMOID_UniqueIdentifier    = {OID_ATTR_TYPE_LENGTH+1, OID_UniqueIdentifier},
CSSMOID_DNQualifier         = {OID_ATTR_TYPE_LENGTH+1, OID_DNQualifier},
CSSMOID_EnhancedSearchGuide = {OID_ATTR_TYPE_LENGTH+1, OID_EnhancedSearchGuide},
CSSMOID_ProtocolInformation = {OID_ATTR_TYPE_LENGTH+1, OID_ProtocolInformation},
CSSMOID_DistinguishedName   = {OID_ATTR_TYPE_LENGTH+1, OID_DistinguishedName},
CSSMOID_UniqueMember        = {OID_ATTR_TYPE_LENGTH+1, OID_UniqueMember},
CSSMOID_HouseIdentifier     = {OID_ATTR_TYPE_LENGTH+1, OID_HouseIdentifier};

/* From PKCS 9 */
static CSSM_OID
CSSMOID_EmailAddress        = {OID_PKCS_9_LENGTH+1, OID_EmailAddress},
CSSMOID_UnstructuredName    = {OID_PKCS_9_LENGTH+1, OID_UnstructuredName},
CSSMOID_ContentType         = {OID_PKCS_9_LENGTH+1, OID_ContentType},
CSSMOID_MessageDigest       = {OID_PKCS_9_LENGTH+1, OID_MessageDigest},
CSSMOID_SigningTime         = {OID_PKCS_9_LENGTH+1, OID_SigningTime},
CSSMOID_CounterSignature    = {OID_PKCS_9_LENGTH+1, OID_CounterSignature},
CSSMOID_ChallengePassword   = {OID_PKCS_9_LENGTH+1, OID_ChallengePassword},
CSSMOID_UnstructuredAddress = {OID_PKCS_9_LENGTH+1, OID_UnstructuredAddress},
CSSMOID_ExtendedCertificateAttributes = {OID_PKCS_9_LENGTH+1, OID_ExtendedCertificateAttributes};


/* The following OIDs (defined above) are commonly used in 
   the issuer and subject name fields of certificates.
OID_CommonName[]            = { OID_ATTR_TYPE, 3 },
OID_CountryName[]           = { OID_ATTR_TYPE, 6 },
OID_LocalityName[]          = { OID_ATTR_TYPE, 7 },
OID_StateProvinceName[]     = { OID_ATTR_TYPE, 8 },
OID_OrganizationName[]      = { OID_ATTR_TYPE, 10 },
OID_OrganizationalUnitName[]= { OID_ATTR_TYPE, 11 }, 
*/
#endif
