

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Sun Jun 06 15:39:38 2010
 */
/* Compiler settings for PartCover.CorDriver.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, LIBID_PartCover,0x7D0E6AAB,0xC5FC,0x4103,0xAA,0xD4,0x8B,0xF3,0x11,0x2A,0x56,0xC4);


MIDL_DEFINE_GUID(IID, IID_IReportReceiver,0x4BAD004E,0x1EF9,0x43d2,0x8D,0x3A,0x09,0x59,0x63,0xE3,0x24,0xEF);


MIDL_DEFINE_GUID(IID, IID_IConnectorActionCallback,0x64845E73,0x9471,0x401d,0xAE,0xB8,0xB6,0xB2,0x4C,0xF0,0xE8,0x94);


MIDL_DEFINE_GUID(IID, IID_IPartCoverConnector2,0x64D5D652,0x8BF4,0x4E16,0xB1,0x92,0x80,0xB6,0xCE,0x91,0x47,0xAD);


MIDL_DEFINE_GUID(CLSID, CLSID_CorProfiler,0x717FF691,0x2ADF,0x4AC0,0x98,0x5F,0x1D,0xD3,0xC4,0x2F,0xDF,0x90);


MIDL_DEFINE_GUID(CLSID, CLSID_PartCoverConnector2,0xFB20430E,0xCDC9,0x45D7,0x84,0x53,0x27,0x22,0x68,0x00,0x2E,0x08);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



