
//  Microsoft (R) .NET Framework IL Disassembler.  Version 4.0.30319.1
//  Copyright (c) Microsoft Corporation.  All rights reserved.



// Metadata version: v4.0.30319
.assembly extern mscorlib
{
  .publickeytoken = (B7 7A 5C 56 19 34 E0 89 )                         // .z\V.4..
  .ver 2:0:0:0
}
.assembly Interop.PartCover
{
  .custom instance void [mscorlib]System.Runtime.InteropServices.TypeLibVersionAttribute::.ctor(int32,
                                                                                                int32) = ( 01 00 04 00 00 00 00 00 00 00 00 00 ) 
  .custom instance void [mscorlib]System.Runtime.InteropServices.GuidAttribute::.ctor(string) = ( 01 00 24 37 44 30 45 36 41 41 42 2D 43 35 46 43   // ..$7D0E6AAB-C5FC
                                                                                                  2D 34 31 30 33 2D 41 41 44 34 2D 38 42 46 33 31   // -4103-AAD4-8BF31
                                                                                                  31 32 41 35 36 43 34 00 00 )                      // 12A56C4..
  .custom instance void [mscorlib]System.Runtime.InteropServices.ImportedFromTypeLibAttribute::.ctor(string) = ( 01 00 09 50 61 72 74 43 6F 76 65 72 00 00 )       // ...PartCover..
  .hash algorithm 0x00008004
  .ver 4:0:0:0
}
.module Interop.PartCover.dll
// MVID: {BC4F1F71-0042-4B4E-B458-C0C973A64896}
.imagebase 0x00400000
.file alignment 0x00000200
.stackreserve 0x00100000
.subsystem 0x0003       // WINDOWS_CUI
.corflags 0x00000001    //  ILONLY
// Image base: 0x00230000


// =============== CLASS MEMBERS DECLARATION ===================

.class interface public abstract auto ansi import PartCover.Framework.CorProfiler
{
  .custom instance void [mscorlib]System.Runtime.InteropServices.GuidAttribute::.ctor(string) = ( 01 00 24 30 30 30 30 30 30 30 30 2D 30 30 30 30   // ..$00000000-0000
                                                                                                  2D 30 30 30 30 2D 43 30 30 30 2D 30 30 30 30 30   // -0000-C000-00000
                                                                                                  30 30 30 30 30 34 36 00 00 )                      // 0000046..
  .custom instance void [mscorlib]System.Runtime.InteropServices.CoClassAttribute::.ctor(class [mscorlib]System.Type) = ( 01 00 24 50 61 72 74 43 6F 76 65 72 2E 46 72 61   // ..$PartCover.Fra
                                                                                                                          6D 65 77 6F 72 6B 2E 43 6F 72 50 72 6F 66 69 6C   // mework.CorProfil
                                                                                                                          65 72 43 6C 61 73 73 00 00 )                      // erClass..
} // end of class PartCover.Framework.CorProfiler

.class public auto ansi import PartCover.Framework.CorProfilerClass
       extends [mscorlib]System.Object
       implements PartCover.Framework.CorProfiler
{
  .custom instance void [mscorlib]System.Runtime.InteropServices.ClassInterfaceAttribute::.ctor(valuetype [mscorlib]System.Runtime.InteropServices.ClassInterfaceType) = ( 01 00 00 00 00 00 00 00 ) 
  .custom instance void [mscorlib]System.Runtime.InteropServices.TypeLibTypeAttribute::.ctor(valuetype [mscorlib]System.Runtime.InteropServices.TypeLibTypeFlags) = ( 01 00 02 00 00 00 00 00 ) 
  .custom instance void [mscorlib]System.Runtime.InteropServices.GuidAttribute::.ctor(string) = ( 01 00 24 37 31 37 46 46 36 39 31 2D 32 41 44 46   // ..$717FF691-2ADF
                                                                                                  2D 34 41 43 30 2D 39 38 35 46 2D 31 44 44 33 43   // -4AC0-985F-1DD3C
                                                                                                  34 32 46 44 46 39 30 00 00 )                      // 42FDF90..
  .method public specialname rtspecialname 
          instance void  .ctor() runtime managed internalcall
  {
  } // end of method CorProfilerClass::.ctor

} // end of class PartCover.Framework.CorProfilerClass

.class interface public abstract auto ansi import PartCover.Framework.IPartCoverConnector2
{
  .custom instance void [mscorlib]System.Runtime.InteropServices.GuidAttribute::.ctor(string) = ( 01 00 24 36 34 44 35 44 36 35 32 2D 38 42 46 34   // ..$64D5D652-8BF4
                                                                                                  2D 34 45 31 36 2D 42 31 39 32 2D 38 30 42 36 43   // -4E16-B192-80B6C
                                                                                                  45 39 31 34 37 41 44 00 00 )                      // E9147AD..
  .custom instance void [mscorlib]System.Runtime.InteropServices.InterfaceTypeAttribute::.ctor(valuetype [mscorlib]System.Runtime.InteropServices.ComInterfaceType) = ( 01 00 01 00 00 00 00 00 ) 
  .method public hidebysig newslot abstract virtual 
          instance void  StartTarget([in] string  marshal( bstr) targetPath,
                                     [in] string  marshal( bstr) targetWorkingDir,
                                     [in] string  marshal( bstr) targetArguments,
                                     [in] bool redirectOutput) runtime managed internalcall
  {
  } // end of method IPartCoverConnector2::StartTarget

  .method public hidebysig newslot specialname abstract virtual 
          instance void  set_LoggingLevel([in] int32 A_1) runtime managed internalcall
  {
  } // end of method IPartCoverConnector2::set_LoggingLevel

  .method public hidebysig newslot specialname abstract virtual 
          instance void  set_FileLoggingEnable([in] bool A_1) runtime managed internalcall
  {
  } // end of method IPartCoverConnector2::set_FileLoggingEnable

  .method public hidebysig newslot specialname abstract virtual 
          instance void  set_PipeLoggingEnable([in] bool A_1) runtime managed internalcall
  {
  } // end of method IPartCoverConnector2::set_PipeLoggingEnable

  .method public hidebysig newslot specialname abstract virtual 
          instance void  set_StatusCallback([in] class PartCover.Framework.IConnectorActionCallback  marshal( interface ) callback) runtime managed internalcall
  {
  } // end of method IPartCoverConnector2::set_StatusCallback

  .method public hidebysig newslot abstract virtual 
          instance void  EnableOption([in] valuetype PartCover.Framework.ProfilerMode mode) runtime managed internalcall
  {
  } // end of method IPartCoverConnector2::EnableOption

  .method public hidebysig newslot abstract virtual 
          instance void  WaitForResults([in] bool delayClose) runtime managed internalcall
  {
  } // end of method IPartCoverConnector2::WaitForResults

  .method public hidebysig newslot abstract virtual 
          instance void  GetReport([in] class PartCover.Framework.IReportReceiver  marshal( interface ) receiver) runtime managed internalcall
  {
  } // end of method IPartCoverConnector2::GetReport

  .method public hidebysig newslot abstract virtual 
          instance void  IncludeItem([in] string  marshal( bstr) item) runtime managed internalcall
  {
  } // end of method IPartCoverConnector2::IncludeItem

  .method public hidebysig newslot abstract virtual 
          instance void  ExcludeItem([in] string  marshal( bstr) item) runtime managed internalcall
  {
  } // end of method IPartCoverConnector2::ExcludeItem

  .method public hidebysig newslot specialname abstract virtual 
          instance bool  get_HasTargetExitCode() runtime managed internalcall
  {
  } // end of method IPartCoverConnector2::get_HasTargetExitCode

  .method public hidebysig newslot specialname abstract virtual 
          instance int32  get_TargetExitCode() runtime managed internalcall
  {
  } // end of method IPartCoverConnector2::get_TargetExitCode

  .method public hidebysig newslot specialname abstract virtual 
          instance string 
          marshal( bstr) 
          get_LogFilePath() runtime managed internalcall
  {
  } // end of method IPartCoverConnector2::get_LogFilePath

  .method public hidebysig newslot specialname abstract virtual 
          instance int32  get_ProcessId() runtime managed internalcall
  {
  } // end of method IPartCoverConnector2::get_ProcessId

  .method public hidebysig newslot specialname abstract virtual 
          instance class PartCover.Framework.IConnectorActionCallback 
          marshal( interface ) 
          get_StatusCallback() runtime managed internalcall
  {
  } // end of method IPartCoverConnector2::get_StatusCallback

  .property int32 LoggingLevel()
  {
    .custom instance void [mscorlib]System.Runtime.InteropServices.DispIdAttribute::.ctor(int32) = ( 01 00 01 00 01 60 00 00 )                         // .....`..
    .set instance void PartCover.Framework.IPartCoverConnector2::set_LoggingLevel(int32)
  } // end of property IPartCoverConnector2::LoggingLevel
  .property bool FileLoggingEnable()
  {
    .custom instance void [mscorlib]System.Runtime.InteropServices.DispIdAttribute::.ctor(int32) = ( 01 00 02 00 01 60 00 00 )                         // .....`..
    .set instance void PartCover.Framework.IPartCoverConnector2::set_FileLoggingEnable(bool)
  } // end of property IPartCoverConnector2::FileLoggingEnable
  .property bool PipeLoggingEnable()
  {
    .custom instance void [mscorlib]System.Runtime.InteropServices.DispIdAttribute::.ctor(int32) = ( 01 00 03 00 01 60 00 00 )                         // .....`..
    .set instance void PartCover.Framework.IPartCoverConnector2::set_PipeLoggingEnable(bool)
  } // end of property IPartCoverConnector2::PipeLoggingEnable
  .property class PartCover.Framework.IConnectorActionCallback
          StatusCallback()
  {
    .custom instance void [mscorlib]System.Runtime.InteropServices.DispIdAttribute::.ctor(int32) = ( 01 00 04 00 01 60 00 00 )                         // .....`..
    .get instance class PartCover.Framework.IConnectorActionCallback PartCover.Framework.IPartCoverConnector2::get_StatusCallback()
    .set instance void PartCover.Framework.IPartCoverConnector2::set_StatusCallback(class PartCover.Framework.IConnectorActionCallback)
  } // end of property IPartCoverConnector2::StatusCallback
  .property bool HasTargetExitCode()
  {
    .custom instance void [mscorlib]System.Runtime.InteropServices.DispIdAttribute::.ctor(int32) = ( 01 00 0A 00 01 60 00 00 )                         // .....`..
    .get instance bool PartCover.Framework.IPartCoverConnector2::get_HasTargetExitCode()
  } // end of property IPartCoverConnector2::HasTargetExitCode
  .property int32 TargetExitCode()
  {
    .custom instance void [mscorlib]System.Runtime.InteropServices.DispIdAttribute::.ctor(int32) = ( 01 00 0B 00 01 60 00 00 )                         // .....`..
    .get instance int32 PartCover.Framework.IPartCoverConnector2::get_TargetExitCode()
  } // end of property IPartCoverConnector2::TargetExitCode
  .property string LogFilePath()
  {
    .custom instance void [mscorlib]System.Runtime.InteropServices.DispIdAttribute::.ctor(int32) = ( 01 00 0C 00 01 60 00 00 )                         // .....`..
    .get instance string PartCover.Framework.IPartCoverConnector2::get_LogFilePath()
  } // end of property IPartCoverConnector2::LogFilePath
  .property int32 ProcessId()
  {
    .custom instance void [mscorlib]System.Runtime.InteropServices.DispIdAttribute::.ctor(int32) = ( 01 00 0D 00 01 60 00 00 )                         // .....`..
    .get instance int32 PartCover.Framework.IPartCoverConnector2::get_ProcessId()
  } // end of property IPartCoverConnector2::ProcessId
} // end of class PartCover.Framework.IPartCoverConnector2

.class interface public abstract auto ansi import PartCover.Framework.PartCoverConnector2
       implements PartCover.Framework.IPartCoverConnector2
{
  .custom instance void [mscorlib]System.Runtime.InteropServices.CoClassAttribute::.ctor(class [mscorlib]System.Type) = ( 01 00 2C 50 61 72 74 43 6F 76 65 72 2E 46 72 61   // ..,PartCover.Fra
                                                                                                                          6D 65 77 6F 72 6B 2E 50 61 72 74 43 6F 76 65 72   // mework.PartCover
                                                                                                                          43 6F 6E 6E 65 63 74 6F 72 32 43 6C 61 73 73 00   // Connector2Class.
                                                                                                                          00 ) 
  .custom instance void [mscorlib]System.Runtime.InteropServices.GuidAttribute::.ctor(string) = ( 01 00 24 36 34 44 35 44 36 35 32 2D 38 42 46 34   // ..$64D5D652-8BF4
                                                                                                  2D 34 45 31 36 2D 42 31 39 32 2D 38 30 42 36 43   // -4E16-B192-80B6C
                                                                                                  45 39 31 34 37 41 44 00 00 )                      // E9147AD..
} // end of class PartCover.Framework.PartCoverConnector2

.class public auto ansi import PartCover.Framework.PartCoverConnector2Class
       extends [mscorlib]System.Object
       implements PartCover.Framework.IPartCoverConnector2,
                  PartCover.Framework.PartCoverConnector2
{
  .custom instance void [mscorlib]System.Runtime.InteropServices.GuidAttribute::.ctor(string) = ( 01 00 24 46 42 32 30 34 33 30 45 2D 43 44 43 39   // ..$FB20430E-CDC9
                                                                                                  2D 34 35 44 37 2D 38 34 35 33 2D 32 37 32 32 36   // -45D7-8453-27226
                                                                                                  38 30 30 32 45 30 38 00 00 )                      // 8002E08..
  .custom instance void [mscorlib]System.Runtime.InteropServices.ClassInterfaceAttribute::.ctor(valuetype [mscorlib]System.Runtime.InteropServices.ClassInterfaceType) = ( 01 00 00 00 00 00 00 00 ) 
  .custom instance void [mscorlib]System.Runtime.InteropServices.TypeLibTypeAttribute::.ctor(valuetype [mscorlib]System.Runtime.InteropServices.TypeLibTypeFlags) = ( 01 00 02 00 00 00 00 00 ) 
  .method public specialname rtspecialname 
          instance void  .ctor() runtime managed internalcall
  {
  } // end of method PartCoverConnector2Class::.ctor

  .method public hidebysig newslot virtual 
          instance void  StartTarget([in] string  marshal( bstr) targetPath,
                                     [in] string  marshal( bstr) targetWorkingDir,
                                     [in] string  marshal( bstr) targetArguments,
                                     [in] bool redirectOutput) runtime managed internalcall
  {
    .override PartCover.Framework.IPartCoverConnector2::StartTarget
  } // end of method PartCoverConnector2Class::StartTarget

  .method public hidebysig newslot specialname virtual 
          instance void  set_LoggingLevel([in] int32 A_1) runtime managed internalcall
  {
    .override PartCover.Framework.IPartCoverConnector2::set_LoggingLevel
  } // end of method PartCoverConnector2Class::set_LoggingLevel

  .method public hidebysig newslot specialname virtual 
          instance void  set_FileLoggingEnable([in] bool A_1) runtime managed internalcall
  {
    .override PartCover.Framework.IPartCoverConnector2::set_FileLoggingEnable
  } // end of method PartCoverConnector2Class::set_FileLoggingEnable

  .method public hidebysig newslot specialname virtual 
          instance void  set_PipeLoggingEnable([in] bool A_1) runtime managed internalcall
  {
    .override PartCover.Framework.IPartCoverConnector2::set_PipeLoggingEnable
  } // end of method PartCoverConnector2Class::set_PipeLoggingEnable

  .method public hidebysig newslot specialname virtual 
          instance void  set_StatusCallback([in] class PartCover.Framework.IConnectorActionCallback  marshal( interface ) callback) runtime managed internalcall
  {
    .override PartCover.Framework.IPartCoverConnector2::set_StatusCallback
  } // end of method PartCoverConnector2Class::set_StatusCallback

  .method public hidebysig newslot virtual 
          instance void  EnableOption([in] valuetype PartCover.Framework.ProfilerMode mode) runtime managed internalcall
  {
    .override PartCover.Framework.IPartCoverConnector2::EnableOption
  } // end of method PartCoverConnector2Class::EnableOption

  .method public hidebysig newslot virtual 
          instance void  WaitForResults([in] bool delayClose) runtime managed internalcall
  {
    .override PartCover.Framework.IPartCoverConnector2::WaitForResults
  } // end of method PartCoverConnector2Class::WaitForResults

  .method public hidebysig newslot virtual 
          instance void  GetReport([in] class PartCover.Framework.IReportReceiver  marshal( interface ) receiver) runtime managed internalcall
  {
    .override PartCover.Framework.IPartCoverConnector2::GetReport
  } // end of method PartCoverConnector2Class::GetReport

  .method public hidebysig newslot virtual 
          instance void  IncludeItem([in] string  marshal( bstr) item) runtime managed internalcall
  {
    .override PartCover.Framework.IPartCoverConnector2::IncludeItem
  } // end of method PartCoverConnector2Class::IncludeItem

  .method public hidebysig newslot virtual 
          instance void  ExcludeItem([in] string  marshal( bstr) item) runtime managed internalcall
  {
    .override PartCover.Framework.IPartCoverConnector2::ExcludeItem
  } // end of method PartCoverConnector2Class::ExcludeItem

  .method public hidebysig newslot specialname virtual 
          instance bool  get_HasTargetExitCode() runtime managed internalcall
  {
    .override PartCover.Framework.IPartCoverConnector2::get_HasTargetExitCode
  } // end of method PartCoverConnector2Class::get_HasTargetExitCode

  .method public hidebysig newslot specialname virtual 
          instance int32  get_TargetExitCode() runtime managed internalcall
  {
    .override PartCover.Framework.IPartCoverConnector2::get_TargetExitCode
  } // end of method PartCoverConnector2Class::get_TargetExitCode

  .method public hidebysig newslot specialname virtual 
          instance string 
          marshal( bstr) 
          get_LogFilePath() runtime managed internalcall
  {
    .override PartCover.Framework.IPartCoverConnector2::get_LogFilePath
  } // end of method PartCoverConnector2Class::get_LogFilePath

  .method public hidebysig newslot specialname virtual 
          instance int32  get_ProcessId() runtime managed internalcall
  {
    .override PartCover.Framework.IPartCoverConnector2::get_ProcessId
  } // end of method PartCoverConnector2Class::get_ProcessId

  .method public hidebysig newslot specialname virtual 
          instance class PartCover.Framework.IConnectorActionCallback 
          marshal( interface ) 
          get_StatusCallback() runtime managed internalcall
  {
    .override PartCover.Framework.IPartCoverConnector2::get_StatusCallback
  } // end of method PartCoverConnector2Class::get_StatusCallback

  .property int32 LoggingLevel()
  {
    .custom instance void [mscorlib]System.Runtime.InteropServices.DispIdAttribute::.ctor(int32) = ( 01 00 01 00 01 60 00 00 )                         // .....`..
    .set instance void PartCover.Framework.PartCoverConnector2Class::set_LoggingLevel(int32)
  } // end of property PartCoverConnector2Class::LoggingLevel
  .property bool FileLoggingEnable()
  {
    .custom instance void [mscorlib]System.Runtime.InteropServices.DispIdAttribute::.ctor(int32) = ( 01 00 02 00 01 60 00 00 )                         // .....`..
    .set instance void PartCover.Framework.PartCoverConnector2Class::set_FileLoggingEnable(bool)
  } // end of property PartCoverConnector2Class::FileLoggingEnable
  .property bool PipeLoggingEnable()
  {
    .custom instance void [mscorlib]System.Runtime.InteropServices.DispIdAttribute::.ctor(int32) = ( 01 00 03 00 01 60 00 00 )                         // .....`..
    .set instance void PartCover.Framework.PartCoverConnector2Class::set_PipeLoggingEnable(bool)
  } // end of property PartCoverConnector2Class::PipeLoggingEnable
  .property class PartCover.Framework.IConnectorActionCallback
          StatusCallback()
  {
    .custom instance void [mscorlib]System.Runtime.InteropServices.DispIdAttribute::.ctor(int32) = ( 01 00 04 00 01 60 00 00 )                         // .....`..
    .set instance void PartCover.Framework.PartCoverConnector2Class::set_StatusCallback(class PartCover.Framework.IConnectorActionCallback)
    .get instance class PartCover.Framework.IConnectorActionCallback PartCover.Framework.PartCoverConnector2Class::get_StatusCallback()
  } // end of property PartCoverConnector2Class::StatusCallback
  .property bool HasTargetExitCode()
  {
    .custom instance void [mscorlib]System.Runtime.InteropServices.DispIdAttribute::.ctor(int32) = ( 01 00 0A 00 01 60 00 00 )                         // .....`..
    .get instance bool PartCover.Framework.PartCoverConnector2Class::get_HasTargetExitCode()
  } // end of property PartCoverConnector2Class::HasTargetExitCode
  .property int32 TargetExitCode()
  {
    .custom instance void [mscorlib]System.Runtime.InteropServices.DispIdAttribute::.ctor(int32) = ( 01 00 0B 00 01 60 00 00 )                         // .....`..
    .get instance int32 PartCover.Framework.PartCoverConnector2Class::get_TargetExitCode()
  } // end of property PartCoverConnector2Class::TargetExitCode
  .property string LogFilePath()
  {
    .custom instance void [mscorlib]System.Runtime.InteropServices.DispIdAttribute::.ctor(int32) = ( 01 00 0C 00 01 60 00 00 )                         // .....`..
    .get instance string PartCover.Framework.PartCoverConnector2Class::get_LogFilePath()
  } // end of property PartCoverConnector2Class::LogFilePath
  .property int32 ProcessId()
  {
    .custom instance void [mscorlib]System.Runtime.InteropServices.DispIdAttribute::.ctor(int32) = ( 01 00 0D 00 01 60 00 00 )                         // .....`..
    .get instance int32 PartCover.Framework.PartCoverConnector2Class::get_ProcessId()
  } // end of property PartCoverConnector2Class::ProcessId
} // end of class PartCover.Framework.PartCoverConnector2Class

.class interface public abstract auto ansi import PartCover.Framework.IConnectorActionCallback
{
  .custom instance void [mscorlib]System.Runtime.InteropServices.InterfaceTypeAttribute::.ctor(valuetype [mscorlib]System.Runtime.InteropServices.ComInterfaceType) = ( 01 00 01 00 00 00 00 00 ) 
  .custom instance void [mscorlib]System.Runtime.InteropServices.GuidAttribute::.ctor(string) = ( 01 00 24 36 34 38 34 35 45 37 33 2D 39 34 37 31   // ..$64845E73-9471
                                                                                                  2D 34 30 31 44 2D 41 45 42 38 2D 42 36 42 32 34   // -401D-AEB8-B6B24
                                                                                                  43 46 30 45 38 39 34 00 00 )                      // CF0E894..
  .method public hidebysig newslot abstract virtual 
          instance void  SetConnected([in] bool connected) runtime managed internalcall
  {
  } // end of method IConnectorActionCallback::SetConnected

  .method public hidebysig newslot abstract virtual 
          instance void  ShowTargetMemory([in] valuetype PartCover.Framework.MEMORY_COUNTERS counters) runtime managed internalcall
  {
  } // end of method IConnectorActionCallback::ShowTargetMemory

  .method public hidebysig newslot abstract virtual 
          instance void  MethodsReceiveBegin() runtime managed internalcall
  {
  } // end of method IConnectorActionCallback::MethodsReceiveBegin

  .method public hidebysig newslot abstract virtual 
          instance void  MethodsReceiveStatus() runtime managed internalcall
  {
  } // end of method IConnectorActionCallback::MethodsReceiveStatus

  .method public hidebysig newslot abstract virtual 
          instance void  MethodsReceiveEnd() runtime managed internalcall
  {
  } // end of method IConnectorActionCallback::MethodsReceiveEnd

  .method public hidebysig newslot abstract virtual 
          instance void  InstrumentDataReceiveBegin() runtime managed internalcall
  {
  } // end of method IConnectorActionCallback::InstrumentDataReceiveBegin

  .method public hidebysig newslot abstract virtual 
          instance void  InstrumentDataReceiveStatus() runtime managed internalcall
  {
  } // end of method IConnectorActionCallback::InstrumentDataReceiveStatus

  .method public hidebysig newslot abstract virtual 
          instance void  InstrumentDataReceiveEnd() runtime managed internalcall
  {
  } // end of method IConnectorActionCallback::InstrumentDataReceiveEnd

  .method public hidebysig newslot abstract virtual 
          instance void  InstrumentDataReceiveFilesBegin() runtime managed internalcall
  {
  } // end of method IConnectorActionCallback::InstrumentDataReceiveFilesBegin

  .method public hidebysig newslot abstract virtual 
          instance void  InstrumentDataReceiveFilesCount([in] uint32 fileCount) runtime managed internalcall
  {
    .param [1]
    .custom instance void [mscorlib]System.Runtime.InteropServices.ComAliasNameAttribute::.ctor(string) = ( 01 00 1D 50 61 72 74 43 6F 76 65 72 2E 46 72 61   // ...PartCover.Fra
                                                                                                            6D 65 77 6F 72 6B 2E 55 4C 4F 4E 47 5F 50 54 52   // mework.ULONG_PTR
                                                                                                            00 00 ) 
  } // end of method IConnectorActionCallback::InstrumentDataReceiveFilesCount

  .method public hidebysig newslot abstract virtual 
          instance void  InstrumentDataReceiveFilesStat([in] uint32 index) runtime managed internalcall
  {
    .param [1]
    .custom instance void [mscorlib]System.Runtime.InteropServices.ComAliasNameAttribute::.ctor(string) = ( 01 00 1D 50 61 72 74 43 6F 76 65 72 2E 46 72 61   // ...PartCover.Fra
                                                                                                            6D 65 77 6F 72 6B 2E 55 4C 4F 4E 47 5F 50 54 52   // mework.ULONG_PTR
                                                                                                            00 00 ) 
  } // end of method IConnectorActionCallback::InstrumentDataReceiveFilesStat

  .method public hidebysig newslot abstract virtual 
          instance void  InstrumentDataReceiveFilesEnd() runtime managed internalcall
  {
  } // end of method IConnectorActionCallback::InstrumentDataReceiveFilesEnd

  .method public hidebysig newslot abstract virtual 
          instance void  InstrumentDataReceiveSkippedBegin() runtime managed internalcall
  {
  } // end of method IConnectorActionCallback::InstrumentDataReceiveSkippedBegin

  .method public hidebysig newslot abstract virtual 
          instance void  InstrumentDataReceiveSkippedCount([in] uint32 itemCount) runtime managed internalcall
  {
    .param [1]
    .custom instance void [mscorlib]System.Runtime.InteropServices.ComAliasNameAttribute::.ctor(string) = ( 01 00 1D 50 61 72 74 43 6F 76 65 72 2E 46 72 61   // ...PartCover.Fra
                                                                                                            6D 65 77 6F 72 6B 2E 55 4C 4F 4E 47 5F 50 54 52   // mework.ULONG_PTR
                                                                                                            00 00 ) 
  } // end of method IConnectorActionCallback::InstrumentDataReceiveSkippedCount

  .method public hidebysig newslot abstract virtual 
          instance void  InstrumentDataReceiveSkippedStat([in] uint32 index) runtime managed internalcall
  {
    .param [1]
    .custom instance void [mscorlib]System.Runtime.InteropServices.ComAliasNameAttribute::.ctor(string) = ( 01 00 1D 50 61 72 74 43 6F 76 65 72 2E 46 72 61   // ...PartCover.Fra
                                                                                                            6D 65 77 6F 72 6B 2E 55 4C 4F 4E 47 5F 50 54 52   // mework.ULONG_PTR
                                                                                                            00 00 ) 
  } // end of method IConnectorActionCallback::InstrumentDataReceiveSkippedStat

  .method public hidebysig newslot abstract virtual 
          instance void  InstrumentDataReceiveSkippedEnd() runtime managed internalcall
  {
  } // end of method IConnectorActionCallback::InstrumentDataReceiveSkippedEnd

  .method public hidebysig newslot abstract virtual 
          instance void  InstrumentDataReceiveCountersBegin() runtime managed internalcall
  {
  } // end of method IConnectorActionCallback::InstrumentDataReceiveCountersBegin

  .method public hidebysig newslot abstract virtual 
          instance void  InstrumentDataReceiveCountersAsmCount([in] uint32 asmCount) runtime managed internalcall
  {
    .param [1]
    .custom instance void [mscorlib]System.Runtime.InteropServices.ComAliasNameAttribute::.ctor(string) = ( 01 00 1D 50 61 72 74 43 6F 76 65 72 2E 46 72 61   // ...PartCover.Fra
                                                                                                            6D 65 77 6F 72 6B 2E 55 4C 4F 4E 47 5F 50 54 52   // mework.ULONG_PTR
                                                                                                            00 00 ) 
  } // end of method IConnectorActionCallback::InstrumentDataReceiveCountersAsmCount

  .method public hidebysig newslot abstract virtual 
          instance void  InstrumentDataReceiveCountersAsm([in] string  marshal( bstr) name,
                                                          [in] string  marshal( bstr) mod,
                                                          [in] uint32 typeDefCount) runtime managed internalcall
  {
    .param [3]
    .custom instance void [mscorlib]System.Runtime.InteropServices.ComAliasNameAttribute::.ctor(string) = ( 01 00 1D 50 61 72 74 43 6F 76 65 72 2E 46 72 61   // ...PartCover.Fra
                                                                                                            6D 65 77 6F 72 6B 2E 55 4C 4F 4E 47 5F 50 54 52   // mework.ULONG_PTR
                                                                                                            00 00 ) 
  } // end of method IConnectorActionCallback::InstrumentDataReceiveCountersAsm

  .method public hidebysig newslot abstract virtual 
          instance void  InstrumentDataReceiveCountersEnd() runtime managed internalcall
  {
  } // end of method IConnectorActionCallback::InstrumentDataReceiveCountersEnd

  .method public hidebysig newslot abstract virtual 
          instance void  OpenMessagePipe() runtime managed internalcall
  {
  } // end of method IConnectorActionCallback::OpenMessagePipe

  .method public hidebysig newslot abstract virtual 
          instance void  TargetSetEnvironmentVars() runtime managed internalcall
  {
  } // end of method IConnectorActionCallback::TargetSetEnvironmentVars

  .method public hidebysig newslot abstract virtual 
          instance void  TargetCreateProcess() runtime managed internalcall
  {
  } // end of method IConnectorActionCallback::TargetCreateProcess

  .method public hidebysig newslot abstract virtual 
          instance void  TargetWaitDriver() runtime managed internalcall
  {
  } // end of method IConnectorActionCallback::TargetWaitDriver

  .method public hidebysig newslot abstract virtual 
          instance void  TargetRequestShutdown() runtime managed internalcall
  {
  } // end of method IConnectorActionCallback::TargetRequestShutdown

  .method public hidebysig newslot abstract virtual 
          instance void  DriverConnected() runtime managed internalcall
  {
  } // end of method IConnectorActionCallback::DriverConnected

  .method public hidebysig newslot abstract virtual 
          instance void  DriverSendRules() runtime managed internalcall
  {
  } // end of method IConnectorActionCallback::DriverSendRules

  .method public hidebysig newslot abstract virtual 
          instance void  DriverWaitEoIConfirm() runtime managed internalcall
  {
  } // end of method IConnectorActionCallback::DriverWaitEoIConfirm

  .method public hidebysig newslot abstract virtual 
          instance void  FunctionsReceiveBegin() runtime managed internalcall
  {
  } // end of method IConnectorActionCallback::FunctionsReceiveBegin

  .method public hidebysig newslot abstract virtual 
          instance void  FunctionsCount([in] uint32 count) runtime managed internalcall
  {
    .param [1]
    .custom instance void [mscorlib]System.Runtime.InteropServices.ComAliasNameAttribute::.ctor(string) = ( 01 00 1D 50 61 72 74 43 6F 76 65 72 2E 46 72 61   // ...PartCover.Fra
                                                                                                            6D 65 77 6F 72 6B 2E 55 4C 4F 4E 47 5F 50 54 52   // mework.ULONG_PTR
                                                                                                            00 00 ) 
  } // end of method IConnectorActionCallback::FunctionsCount

  .method public hidebysig newslot abstract virtual 
          instance void  FunctionsReceiveStat([in] uint32 index) runtime managed internalcall
  {
    .param [1]
    .custom instance void [mscorlib]System.Runtime.InteropServices.ComAliasNameAttribute::.ctor(string) = ( 01 00 1D 50 61 72 74 43 6F 76 65 72 2E 46 72 61   // ...PartCover.Fra
                                                                                                            6D 65 77 6F 72 6B 2E 55 4C 4F 4E 47 5F 50 54 52   // mework.ULONG_PTR
                                                                                                            00 00 ) 
  } // end of method IConnectorActionCallback::FunctionsReceiveStat

  .method public hidebysig newslot abstract virtual 
          instance void  FunctionsReceiveEnd() runtime managed internalcall
  {
  } // end of method IConnectorActionCallback::FunctionsReceiveEnd

  .method public hidebysig newslot abstract virtual 
          instance void  LogMessage([in] int32 threadId,
                                    [in] int32 tick,
                                    [in] string  marshal( bstr) message) runtime managed internalcall
  {
  } // end of method IConnectorActionCallback::LogMessage

} // end of class PartCover.Framework.IConnectorActionCallback

.class public auto ansi sealed PartCover.Framework.ProfilerMode
       extends [mscorlib]System.Enum
{
  .custom instance void [mscorlib]System.Runtime.InteropServices.GuidAttribute::.ctor(string) = ( 01 00 24 39 42 43 32 33 44 32 30 2D 30 34 44 45   // ..$9BC23D20-04DE
                                                                                                  2D 34 45 45 37 2D 41 42 32 34 2D 31 45 38 39 30   // -4EE7-AB24-1E890
                                                                                                  43 37 34 31 46 37 38 00 00 )                      // C741F78..
  .field public specialname rtspecialname int32 value__
  .field public static literal valuetype PartCover.Framework.ProfilerMode COUNT_COVERAGE = int32(0x00000001)
  .field public static literal valuetype PartCover.Framework.ProfilerMode COVERAGE_USE_CLASS_LEVEL = int32(0x00000004)
  .field public static literal valuetype PartCover.Framework.ProfilerMode COVERAGE_USE_ASSEMBLY_LEVEL = int32(0x00000008)
} // end of class PartCover.Framework.ProfilerMode

.class interface public abstract auto ansi import PartCover.Framework.IReportReceiver
{
  .custom instance void [mscorlib]System.Runtime.InteropServices.GuidAttribute::.ctor(string) = ( 01 00 24 34 42 41 44 30 30 34 45 2D 31 45 46 39   // ..$4BAD004E-1EF9
                                                                                                  2D 34 33 44 32 2D 38 44 33 41 2D 30 39 35 39 36   // -43D2-8D3A-09596
                                                                                                  33 45 33 32 34 45 46 00 00 )                      // 3E324EF..
  .custom instance void [mscorlib]System.Runtime.InteropServices.InterfaceTypeAttribute::.ctor(valuetype [mscorlib]System.Runtime.InteropServices.ComInterfaceType) = ( 01 00 01 00 00 00 00 00 ) 
  .method public hidebysig newslot abstract virtual 
          instance void  RegisterSkippedItem([in] string  marshal( bstr) assemblyName,
                                             [in] string  marshal( bstr) typedefName) runtime managed internalcall
  {
  } // end of method IReportReceiver::RegisterSkippedItem

  .method public hidebysig newslot abstract virtual 
          instance void  EnterAssembly([in] int32 domain,
                                       [in] string  marshal( bstr) domainName,
                                       [in] string  marshal( bstr) assemblyName,
                                       [in] string  marshal( bstr) moduleName) runtime managed internalcall
  {
  } // end of method IReportReceiver::EnterAssembly

  .method public hidebysig newslot abstract virtual 
          instance void  EnterTypedef([in] string  marshal( bstr) typedefName,
                                      [in] uint32 'flags') runtime managed internalcall
  {
  } // end of method IReportReceiver::EnterTypedef

  .method public hidebysig newslot abstract virtual 
          instance void  EnterMethod([in] string  marshal( bstr) methodName,
                                     [in] string  marshal( bstr) methodSig,
                                     [in] int32 bodySize,
                                     [in] uint32 'flags',
                                     [in] uint32 implFlags,
                                     [in] int32 symbolFileId,
                                     [in] int32 methodDef) runtime managed internalcall
  {
  } // end of method IReportReceiver::EnterMethod

  .method public hidebysig newslot abstract virtual 
          instance void  AddCoverageBlock([in] valuetype PartCover.Framework.BLOCK_DATA blockData) runtime managed internalcall
  {
  } // end of method IReportReceiver::AddCoverageBlock

  .method public hidebysig newslot abstract virtual 
          instance void  LeaveMethod() runtime managed internalcall
  {
  } // end of method IReportReceiver::LeaveMethod

  .method public hidebysig newslot abstract virtual 
          instance void  LeaveTypedef() runtime managed internalcall
  {
  } // end of method IReportReceiver::LeaveTypedef

  .method public hidebysig newslot abstract virtual 
          instance void  LeaveAssembly() runtime managed internalcall
  {
  } // end of method IReportReceiver::LeaveAssembly

} // end of class PartCover.Framework.IReportReceiver

.class public sequential ansi sealed beforefieldinit PartCover.Framework.BLOCK_DATA
       extends [mscorlib]System.ValueType
{
  .pack 4
  .size 0
  .custom instance void [mscorlib]System.Runtime.InteropServices.GuidAttribute::.ctor(string) = ( 01 00 24 30 34 30 37 39 30 45 36 2D 44 34 41 31   // ..$040790E6-D4A1
                                                                                                  2D 34 35 37 39 2D 41 34 42 39 2D 42 46 39 42 33   // -4579-A4B9-BF9B3
                                                                                                  31 45 38 46 39 45 38 00 00 )                      // 1E8F9E8..
  .field public int32 position
  .field public int32 blockLen
  .field public int32 visitCount
} // end of class PartCover.Framework.BLOCK_DATA

.class public sequential ansi sealed beforefieldinit PartCover.Framework.MEMORY_COUNTERS
       extends [mscorlib]System.ValueType
{
  .pack 4
  .size 0
  .custom instance void [mscorlib]System.Runtime.InteropServices.GuidAttribute::.ctor(string) = ( 01 00 24 33 37 44 45 30 45 35 42 2D 43 41 38 42   // ..$37DE0E5B-CA8B
                                                                                                  2D 34 41 38 39 2D 39 30 37 31 2D 31 30 34 31 37   // -4A89-9071-10417
                                                                                                  41 41 34 42 43 46 34 00 00 )                      // AA4BCF4..
  .field public uint32 PageFaultCount
  .field public uint32 PeakWorkingSetSize
  .custom instance void [mscorlib]System.Runtime.InteropServices.ComAliasNameAttribute::.ctor(string) = ( 01 00 1D 50 61 72 74 43 6F 76 65 72 2E 46 72 61   // ...PartCover.Fra
                                                                                                          6D 65 77 6F 72 6B 2E 55 4C 4F 4E 47 5F 50 54 52   // mework.ULONG_PTR
                                                                                                          00 00 ) 
  .field public uint32 WorkingSetSize
  .custom instance void [mscorlib]System.Runtime.InteropServices.ComAliasNameAttribute::.ctor(string) = ( 01 00 1D 50 61 72 74 43 6F 76 65 72 2E 46 72 61   // ...PartCover.Fra
                                                                                                          6D 65 77 6F 72 6B 2E 55 4C 4F 4E 47 5F 50 54 52   // mework.ULONG_PTR
                                                                                                          00 00 ) 
  .field public uint32 QuotaPeakPagedPoolUsage
  .custom instance void [mscorlib]System.Runtime.InteropServices.ComAliasNameAttribute::.ctor(string) = ( 01 00 1D 50 61 72 74 43 6F 76 65 72 2E 46 72 61   // ...PartCover.Fra
                                                                                                          6D 65 77 6F 72 6B 2E 55 4C 4F 4E 47 5F 50 54 52   // mework.ULONG_PTR
                                                                                                          00 00 ) 
  .field public uint32 QuotaPagedPoolUsage
  .custom instance void [mscorlib]System.Runtime.InteropServices.ComAliasNameAttribute::.ctor(string) = ( 01 00 1D 50 61 72 74 43 6F 76 65 72 2E 46 72 61   // ...PartCover.Fra
                                                                                                          6D 65 77 6F 72 6B 2E 55 4C 4F 4E 47 5F 50 54 52   // mework.ULONG_PTR
                                                                                                          00 00 ) 
  .field public uint32 QuotaPeakNonPagedPoolUsage
  .custom instance void [mscorlib]System.Runtime.InteropServices.ComAliasNameAttribute::.ctor(string) = ( 01 00 1D 50 61 72 74 43 6F 76 65 72 2E 46 72 61   // ...PartCover.Fra
                                                                                                          6D 65 77 6F 72 6B 2E 55 4C 4F 4E 47 5F 50 54 52   // mework.ULONG_PTR
                                                                                                          00 00 ) 
  .field public uint32 QuotaNonPagedPoolUsage
  .custom instance void [mscorlib]System.Runtime.InteropServices.ComAliasNameAttribute::.ctor(string) = ( 01 00 1D 50 61 72 74 43 6F 76 65 72 2E 46 72 61   // ...PartCover.Fra
                                                                                                          6D 65 77 6F 72 6B 2E 55 4C 4F 4E 47 5F 50 54 52   // mework.ULONG_PTR
                                                                                                          00 00 ) 
  .field public uint32 PagefileUsage
  .custom instance void [mscorlib]System.Runtime.InteropServices.ComAliasNameAttribute::.ctor(string) = ( 01 00 1D 50 61 72 74 43 6F 76 65 72 2E 46 72 61   // ...PartCover.Fra
                                                                                                          6D 65 77 6F 72 6B 2E 55 4C 4F 4E 47 5F 50 54 52   // mework.ULONG_PTR
                                                                                                          00 00 ) 
  .field public uint32 PeakPagefileUsage
  .custom instance void [mscorlib]System.Runtime.InteropServices.ComAliasNameAttribute::.ctor(string) = ( 01 00 1D 50 61 72 74 43 6F 76 65 72 2E 46 72 61   // ...PartCover.Fra
                                                                                                          6D 65 77 6F 72 6B 2E 55 4C 4F 4E 47 5F 50 54 52   // mework.ULONG_PTR
                                                                                                          00 00 ) 
} // end of class PartCover.Framework.MEMORY_COUNTERS


// =============================================================

// *********** DISASSEMBLY COMPLETE ***********************
// WARNING: Created Win32 resource file Interop\Interop.PartCover.res
