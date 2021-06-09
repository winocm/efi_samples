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

/*
 *  INTEL CONFIDENTIAL
 *  This file, software, or program is supplied under the terms
 *  of a licence agreement or nondisclosure agreement with
 *  Intel Corporation and may not be copied or disclosed except
 *  in accordance with the terms of that agreement. This file,
 *  software, or program contains copyrighted material and/or
 *  trade secret information of Intel Corporation, and must be
 *  treated as such. Intel reserves all rights in this material,
 *  except as the licence agreement or nondisclosure agreement
 *  specifically indicate.
*/

/* 
 * Module name: verifymd.c
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"
#include "../../include/rsakg.h"
#include "../../include/iclproc.h"
#include "../../include/misc.h"

#ifdef ICL_DSA
/***************************************************************************/

#if defined(_MSC_VER)
#pragma warning(disable:4035)
#endif

/*  Verification of DSS signatures with precomputed digest value
    The function returns
    0        for verified signatures
    -1        for rejected signatures
*/

int ICL_DSSVerifyDigest
        (ICLData            *Digest,
        ICLData             *Signature_r,
        ICLData             *Signature_s,
        ICLDSSPublicKey     *PublicKey)
{
    ICLWord    sval[MODULUS], rval[MODULUS], qval[MODULUS],
        pval[MODULUS], wval[MODULUS], gval[MODULUS],
        u1val[MODULUS], u2val[MODULUS], yval[MODULUS],
        vval[MODULUS];
    RSAData    s, r, p, q, w, g, u1, u2, y, v;

    int Index;

    /* array value assignments */
    s.value = sval;                         
    ICL_MoveByte2Word(Signature_s, &s);     
                                                
    r.value = rval;                         
    ICL_MoveByte2Word(Signature_r, &r);     
                                            
    p.value = pval;                         
    ICL_MoveByte2Word(&PublicKey->PrimeModulus, &p);      
                                            
    q.value = qval;                         
    ICL_MoveByte2Word(&PublicKey->PrimeDivisor, &q);      
    
    g.value = gval;                         
    ICL_MoveByte2Word(&PublicKey->OrderQ, &g);            
    
    y.value = yval;                         
    ICL_MoveByte2Word(                      
        &PublicKey->PublicKey, &y);         
    
    w.value = wval;                         
    u1.value = u1val;                       
    u2.value = u2val;                       
    v.value = vval;                         

    // Check to make sure 0<r<q and 0<s<q
    if (s.length == 1 && s.value[0] == 0)   
        return -1;                          
                                            
    if (r.length == 1 || r.value[0] == 0)   
        return -1;                          
                                            
    // if s>=q return error
    if (ICL_Compare(&s, &q) >= 0)           
        return -1;                          
                                            
    // if r>=q return error
    if (ICL_Compare(&r, &q) >= 0)           
        return -1;                          
                                            
    // OK, 0<r<q and 0<s<q, do the computations ... 


    // w = s^{-1} mod q 
    ICL_ModularInverse (                    
                Signature_s,                
                &PublicKey->PrimeDivisor,   
                (ICLData *)&w);             
                                            
    // pad the high order bytes of q with zeros for dword alignment 
    ICL_PadBytes0 (&w, w.length);           

    w.length = ICL_WordCount (w.length);    
    
    // u1 = digest*w mod q 

    // clear the u1 value                       
    for (Index = 0; Index < MODULUS; Index++)   
        u1.value[Index] = 0;                    
    
    u1.length = ICL_WordCount(Digest->length);  
            
    u1.value[4] = bswap ( ((ICLWord *) Digest->value)[0] );
    u1.value[3] = bswap ( ((ICLWord *) Digest->value)[1] );
    u1.value[2] = bswap ( ((ICLWord *) Digest->value)[2] );
    u1.value[1] = bswap ( ((ICLWord *) Digest->value)[3] );
    u1.value[0] = bswap ( ((ICLWord *) Digest->value)[4] );

    // u1 = u1 mod q                        
    ICL_Rem(&u1,        // a                
            &q,         // b                
            &u1);       // remainder        
                                            
    // u1 = u1*w mod q (t = a * b mod n)    
    ICL_ModMul (                            
        &u1,    // a                        
        &w,     // b                        
        &q,     // n                        
        &u1);   // t                        
 
    // u2 = r*w mod q (t = a * b mod n) 
    ICL_ModMul (                        
        &r,     // a                    
        &w,     // b                    
        &q,     // n                    
        &u2);   // t                    
 
    //u1 = g^u1 mod p           
    ICL_ModExp (                
        &g,     // a            
        &u1,    // e,           
        &p,     // n,           
        &u1);   // t            
 
    // u2 = y^u2 mod p          
    ICL_ModExp (                
        &y,     // a            
        &u2,    // e,           
        &p,     // n,           
        &u2);   // t            
                                
    // u1 = u1*u2 mod p  (t = a * b mod n)  
    ICL_ModMul (                            
        &u1,    // a                        
        &u2,    // b                        
        &p,     // n                        
        &u1);   // t                        
 
    // v = (g^u1 * y^u2 mod p) mod q    
    ICL_Rem(&u1,    // a                
            &q,     // b                
            &v);    // remainder        
                                        
    // the signature comparison         
    if (ICL_Compare(&r, &v) != 0)       
        return -1;                      
                                        
    return 0;                           
}

/***************************************************************************/
#endif
