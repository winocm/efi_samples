/*-----------------------------------------------------------------------
 *      File:   TRC_UTIL.H
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1996, 1997
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *-----------------------------------------------------------------------
 */
/* 
 * WARNING: EXPORT RESTRICTED. 
 * This software listing contains cryptographic methods and technology. 
 * It is export restricted by the Office of Defense Trade Controls, United 
 * States Department of State and cannot be downloaded or otherwise 
 * exported or re-exported (i) into (or to a national or resident of) Cuba, 
 * Iraq, Libya, Yugoslavia, North Korea, Iran, Syria or any other country 
 * to which the US has embargoed goods; or (ii) to anyone on the US 
 * Treasury Department's list of Specially Designated Nationals or the US 
 * Commerce Department's Table of Denial Orders. By downloading or using 
 * this product, you are agreeing to the foregoing and you are representing 
 * and warranting that you are not located in, under the control of, or a 
 * national or resident of any such country or on any such list. 
 */ 
#ifdef DO_TRACE
#ifdef BINARY_TRACE

#define RETURN(x)\
{\
BinWrite(ThisFilter, Level, 1, ThisFunction, (uint32)x);\ return(x);\
}

#define RETURN_EXPRESSION(x)\
{\
BinWrite(ThisFilter, Level, 2, ThisFunction, 0);\ return(x);\
}

#define VOID_RETURN\
{\
BinWrite(ThisFilter, Level, 3, ThisFunction, 0);\ return;\
}

#else /* if not BINARY_TRACE */

#define RETURN(x)\
{\
Dprintf(ThisFilter, Level, "%s <- %s (%d)\n", DecreaseStackDepth(), ThisFunction, x);\
return(x);\
}

#define RETURN_EXPRESSION(x)\
{\
Dprintf(ThisFilter, Level, "%s <- %s (?)\n",
DecreaseStackDepth(), ThisFunction);\
return(x);\
}

#define VOID_RETURN\
{\
Dprintf(ThisFilter, Level, "%s <- %s\n", DecreaseStackDepth(),
ThisFunction);\
return;\
}
#endif /* BINARY_TRACE */

#else /* if not TRACE */
#define RETURN(x) return(x);
#define RETURN_EXPRESSION(x) return(x); 
#define VOID_RETURN  return;
#endif /* TRACE */
