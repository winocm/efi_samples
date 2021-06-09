/*

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

*/

#ifndef CSSM_BIS

static CSSM_CSSMINFO CssmInfo =
{
    {CSSM_MAJOR,CSSM_MINOR}, 
    "",
    "Intel",
    CSSM_FALSE,
    "",
    {intel_preos_cssm_guid_def},
    {intel_preos_cssm_guid_def}
};

#define NUM_MODULES_LIST 4

static CSSM_LIST_ITEM ModuleListItems[NUM_MODULES_LIST] =
{
    {
        {
            {intel_preos_cssm_guid_def},
            {CSSM_MAJOR,CSSM_MINOR},
            0,
            0
        },
        "Intel CSSM"
    },
    {
        {
            {intel_preos_isr_csm_guid_def},
            {INTEL_CSM_MAJOR_VER,INTEL_CSM_MINOR_VER},
            0,
            0
        },
        "Intel ISR Cryptographic Services Module"
    },
    {
        {
            {intel_preos_x509v3_clm_guid_def},
            {INTEL_X509V3_CLM_MAJOR_VER,INTEL_X509V3_CLM_MINOR_VER},
            0,
            0
        },
        "Intel X509v3 Certificate Library Module"
    },
    {
        {
            {intel_preos_nsi_dsm_guid_def},
            {INTEL_NSI_DSM_MAJOR_VER,INTEL_NSI_DSM_MINOR_VER},
            0,
            0
        },
        "Intel NVS Data Storage Module"
    },
};

static CSSM_LIST      ModuleList = {NUM_MODULES_LIST, ModuleListItems};
static CSSM_MODULE_INFO ModuleInfo[NUM_MODULES_LIST] =
                        {
                            {   /* Core CSSM */
                                {CSSM_MAJOR,CSSM_MINOR},/* Module version */
                                {0,0},                  /* Module written for CSSM version */
	                            NULL,                   /* opt GUID defining supported interface */
                                "",                     /* Module description */
                                "Intel",                /* Vendor name, etc */
                                CSSM_MODULE_EXPORTABLE, /* Flags to describe and control module use */
/* ASK DENISE */                NULL,                   /* Module-specific keys to authenticate apps */
/* ASK DENISE */                0,                      /* Number of module-specific root keys */
                                0,                      /* Bit mask of supported services */
                                0,                      /* Num of services in ServiceList */
                                NULL,	                /* Pointer to list of service infos */
                                0                       /* Reserved */
                            },
                            {   /* CSP */
                                {INTEL_CSM_MAJOR_VER,INTEL_CSM_MINOR_VER},/* Module version */
                                {0,0},                  /* Module written for CSSM version */
	                            NULL,                   /* opt GUID defining supported interface */
                                "",                     /* Module description */
                                "Intel",                /* Vendor name, etc */
                                CSSM_MODULE_EXPORTABLE, /* Flags to describe and control module use */
	                            NULL,                   /* Module-specific keys to authenticate apps */
                                0,                      /* Number of module-specific root keys */
                                CSSM_SERVICE_CSP,       /* Bit mask of supported services */
                                1,                      /* Num of services in ServiceList */
                                NULL,	                /* Pointer to list of service infos */
                                0                       /* Reserved */
                            },
                            {   /* CL */
                                {INTEL_X509V3_CLM_MAJOR_VER,INTEL_X509V3_CLM_MINOR_VER},              /* Module version */
                                {0,0},              /* Module written for CSSM version */
	                            NULL,               /* opt GUID defining supported interface */
                                "",               /* Module description */
                                "Intel",            /* Vendor name, etc */
                                CSSM_MODULE_EXPORTABLE,/* Flags to describe and control module use */                            NULL,               /* Module-specific keys to authenticate apps */
                                0,                  /* Number of module-specific root keys */
                                CSSM_SERVICE_CL,    /* Bit mask of supported services */
                                1,                  /* Num of services in ServiceList */
                                NULL,	            /* Pointer to list of service infos */
                                0                   /* Reserved */
                            },
                            {   /* DL */
                                {INTEL_NSI_DSM_MAJOR_VER,INTEL_NSI_DSM_MINOR_VER},              /* Module version */
                                {0,0},              /* Module written for CSSM version */
	                            NULL,               /* opt GUID defining supported interface */
                                "",               /* Module description */
                                "Intel",            /* Vendor name, etc */
                                CSSM_MODULE_EXPORTABLE,/* Flags to describe and control module use */
	                            NULL,               /* Module-specific keys to authenticate apps */
                                0,                  /* Number of module-specific root keys */
                                CSSM_SERVICE_DL,    /* Bit mask of supported services */
                                1,                  /* Num of services in ServiceList */
                                NULL,	            /* Pointer to list of service infos */
                                0                   /* Reserved */
                            },
                        };

#endif /* #ifndef CSSM_BIS */
