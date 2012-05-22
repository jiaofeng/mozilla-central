/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* Platform specific code to invoke XPCOM methods on native objects */

#include "xptcprivate.h"

/* solaris defines __sparc for workshop compilers and 
   linux defines __sparc__ */

#if !defined(__sparc) && !defined(__sparc__)
#error "This code is for Sparc only"
#endif

typedef unsigned nsXPCVariant;

extern "C" PRUint32
invoke_count_words(PRUint32 paramCount, nsXPTCVariant* s)
{
    PRUint32 result = 0;
    for(PRUint32 i = 0; i < paramCount; i++, s++)
    {
        if(s->IsPtrData())
        {
            result++;
            continue;
        }
        switch(s->type)
        {
        case nsXPTType::T_I8     :
        case nsXPTType::T_I16    :
        case nsXPTType::T_I32    :
            result++;
            break;
        case nsXPTType::T_I64    :
            result+=2;
            break;
        case nsXPTType::T_U8     :
        case nsXPTType::T_U16    :
        case nsXPTType::T_U32    :
            result++;
            break;
        case nsXPTType::T_U64    :
            result+=2;
            break;
        case nsXPTType::T_FLOAT  :
            result++;
            break;
        case nsXPTType::T_DOUBLE :
            result+=2;
            break;
        case nsXPTType::T_BOOL   :
        case nsXPTType::T_CHAR   :
        case nsXPTType::T_WCHAR  :
            result++;
            break;
        default:
            // all the others are plain pointer types
            result++;
            break;
        }
    }
    // nuts, I know there's a cooler way of doing this, but it's late
    // now and it'll probably come to me in the morning.
    if (result & 0x3) result += 4 - (result & 0x3);     // ensure q-word alignment
    return result;
}

extern "C" PRUint32
invoke_copy_to_stack(PRUint32* d, PRUint32 paramCount, nsXPTCVariant* s)
{
/*
    We need to copy the parameters for this function to locals and use them
    from there since the parameters occupy the same stack space as the stack
    we're trying to populate.
*/
    uint32 *l_d = d;
    nsXPTCVariant *l_s = s;
    uint32 l_paramCount = paramCount;
    uint32 regCount = 0;	// return the number of registers to load from the stack

    typedef struct {
        uint32 hi;
        uint32 lo;
    } DU;               // have to move 64 bit entities as 32 bit halves since
                        // stack slots are not guaranteed 16 byte aligned

    for(uint32 i = 0; i < l_paramCount; i++, l_d++, l_s++)
    {
	if (regCount < 5) regCount++;
        if(l_s->IsPtrData())
        {
            *((void**)l_d) = l_s->ptr;
            continue;
        }
        switch(l_s->type)
        {
        case nsXPTType::T_I8     : *((int32*)   l_d) = l_s->val.i8;          break;
        case nsXPTType::T_I16    : *((int32*)  l_d) = l_s->val.i16;         break;
        case nsXPTType::T_I32    : *((int32*)  l_d) = l_s->val.i32;         break;
        case nsXPTType::T_I64    : 
        case nsXPTType::T_U64    : 
        case nsXPTType::T_DOUBLE : *((uint32*) l_d++) = ((DU *)l_s)->hi;
				   if (regCount < 5) regCount++;
                                   *((uint32*) l_d) = ((DU *)l_s)->lo;
                                   break;
        case nsXPTType::T_U8     : *((uint32*) l_d) = l_s->val.u8;          break;
        case nsXPTType::T_U16    : *((uint32*) l_d) = l_s->val.u16;         break;
        case nsXPTType::T_U32    : *((uint32*) l_d) = l_s->val.u32;         break;
        case nsXPTType::T_FLOAT  : *((float*)  l_d) = l_s->val.f;           break;
        case nsXPTType::T_BOOL   : *((uint32*) l_d) = l_s->val.b;           break;
        case nsXPTType::T_CHAR   : *((uint32*) l_d) = l_s->val.c;           break;
        case nsXPTType::T_WCHAR  : *((int32*)  l_d) = l_s->val.wc;          break;
        default:
            // all the others are plain pointer types
            *((void**)l_d) = l_s->val.p;
            break;
        }
    }
    return regCount;
}

