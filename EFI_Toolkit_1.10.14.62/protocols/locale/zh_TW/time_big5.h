/*
 * Copyright (c) 1999, 2000
 * Intel Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software must
 *    display the following acknowledgement:
 * 
 *    This product includes software developed by Intel Corporation and its
 *    contributors.
 * 
 * 4. Neither the name of Intel Corporation or its contributors may be used to
 *    endorse or promote products derived from this software without specific
 *    prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#ifndef _TIME_BIG5_H_
#define _TIME_BIG5_H_

#include <efi.h>

UINT8   TimeData[] =
{
0x20, 0x20, 0x31, 0x0a, 0x20, 0x20, 0x32, 0x0a, 
0x20, 0x20, 0x33, 0x0a, 0x20, 0x20, 0x34, 0x0a, 
0x20, 0x20, 0x35, 0x0a, 0x20, 0x20, 0x36, 0x0a, 
0x20, 0x20, 0x37, 0x0a, 0x20, 0x20, 0x38, 0x0a, 
0x20, 0x20, 0x39, 0x0a, 0x20, 0x31, 0x30, 0x0a, 
0x20, 0x31, 0x31, 0x0a, 0x20, 0x31, 0x32, 0x0a, 
0x31, 0xa4, 0xeb, 0x0a, 0x32, 0xa4, 0xeb, 0x0a, 
0x33, 0xa4, 0xeb, 0x0a, 0x34, 0xa4, 0xeb, 0x0a, 
0x35, 0xa4, 0xeb, 0x0a, 0x36, 0xa4, 0xeb, 0x0a, 
0x37, 0xa4, 0xeb, 0x0a, 0x38, 0xa4, 0xeb, 0x0a, 
0x39, 0xa4, 0xeb, 0x0a, 0x31, 0x30, 0xa4, 0xeb, 
0x0a, 0x31, 0x31, 0xa4, 0xeb, 0x0a, 0x31, 0x32, 
0xa4, 0xeb, 0x0a, 0xa4, 0xe9, 0x20, 0x0a, 0xa4, 
0x40, 0x20, 0x0a, 0xa4, 0x47, 0x20, 0x0a, 0xa4, 
0x54, 0x20, 0x0a, 0xa5, 0x7c, 0x20, 0x0a, 0xa4, 
0xad, 0x20, 0x0a, 0xa4, 0xbb, 0x20, 0x0a, 0xa9, 
0x50, 0xa4, 0xe9, 0x0a, 0xa9, 0x50, 0xa4, 0x40, 
0x0a, 0xa9, 0x50, 0xa4, 0x47, 0x0a, 0xa9, 0x50, 
0xa4, 0x54, 0x0a, 0xa9, 0x50, 0xa5, 0x7c, 0x0a, 
0xa9, 0x50, 0xa4, 0xad, 0x0a, 0xa9, 0x50, 0xa4, 
0xbb, 0x0a, 0x25, 0x48, 0xae, 0xc9, 0x25, 0x4d, 
0xa4, 0xc0, 0x25, 0x53, 0xac, 0xed, 0x0a, 0x25, 
0x79, 0x2f, 0x25, 0x6d, 0x2f, 0x25, 0x64, 0x0a, 
0x25, 0x61, 0x20, 0x25, 0x62, 0x2f, 0x25, 0x65, 
0x20, 0x25, 0x54, 0x20, 0x25, 0x59, 0x0a, 0xa4, 
0x57, 0xa4, 0xc8, 0x0a, 0xa4, 0x55, 0xa4, 0xc8, 
0x0a, 0x25, 0x59, 0xa6, 0x7e, 0x25, 0x62, 0xa4, 
0xeb, 0x25, 0x65, 0xa4, 0xe9, 0x20, 0x25, 0x41, 
0x20, 0x25, 0x58, 0x20, 0x25, 0x5a, 0x0a
//Total: 231 bytes
};

int     TimeDataSize = 231;

#endif // _TIME_BIG5_H_
