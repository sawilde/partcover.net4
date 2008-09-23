using System;
using System.Collections.Generic;
using System.Text;
using PartCover.Framework;
using PartCover.Browser.Api;

namespace PartCover.Browser.Stuff
{
    class TargetRunnerCallback : IConnectorActionCallback
    {
        private IProgressCallback callback;

        public TargetRunnerCallback(IProgressCallback callback)
        {
            this.callback = callback;
        }

        public void DriverConnected() { callback.writeStatus("driver connected successfully"); }

        public void DriverSendRules() { callback.writeStatus("send rules to the driver"); }

        public void DriverWaitEoIConfirm() { callback.writeStatus("wait for rules confirm"); }

        public void FunctionsCount(uint count) { callback.writeStatus(string.Format("functions count {0}", count)); }

        public void FunctionsReceiveBegin() { callback.writeStatus("functions loading is being started"); }

        public void FunctionsReceiveEnd() { callback.writeStatus("functions loading is complete"); }

        public void FunctionsReceiveStat(uint index) { callback.writeStatus(string.Format("functions loaded: {0}", index)); }

        public void InstrumentDataReceiveBegin() { callback.writeStatus("instrument data is being received"); }

        public void InstrumentDataReceiveCountersAsm(string name, string mod, uint typeDefCount) { callback.writeStatus(string.Format("load assembly {0} ({1} types)", name, typeDefCount)); }

        public void InstrumentDataReceiveCountersAsmCount(uint asmCount) { callback.writeStatus(string.Format("assembly count: {0}", asmCount)); }

        public void InstrumentDataReceiveCountersBegin() { callback.writeStatus("InstrumentDataReceiveCountersBegin"); }

        public void InstrumentDataReceiveCountersEnd() { callback.writeStatus("InstrumentDataReceiveCountersEnd"); }

        public void InstrumentDataReceiveEnd() { callback.writeStatus("instrument data load complete"); }

        public void InstrumentDataReceiveFilesBegin() { callback.writeStatus("file list is being received"); }

        public void InstrumentDataReceiveFilesCount(uint fileCount) { callback.writeStatus(string.Format("{0} files to load", fileCount)); }

        public void InstrumentDataReceiveFilesEnd() { callback.writeStatus("file list load is complete"); }

        public void InstrumentDataReceiveFilesStat(uint index) { callback.writeStatus(string.Format("{0} files loaded", index)); }

        public void InstrumentDataReceiveStatus() { callback.writeStatus("driver connected"); }

        public void MethodsReceiveBegin() { callback.writeStatus("function map is being received"); }

        public void MethodsReceiveEnd() { callback.writeStatus("function map load complete"); }

        public void MethodsReceiveStatus() { callback.writeStatus("function map load status"); }

        public void OpenMessagePipe() { callback.writeStatus("open driver pipe"); }

        public void SetConnected(bool connected) { callback.writeStatus(connected ? "driver connected" : "driver disconnected"); }

        public void TargetCreateProcess() { callback.writeStatus("create target process"); }

        public void TargetRequestShutdown() { callback.writeStatus("request target shutdown"); }

        public void TargetSetEnvironmentVars() { callback.writeStatus("modify target environment variables"); }

        public void TargetWaitDriver() { callback.writeStatus("wait for driver connection"); }

    }
}
