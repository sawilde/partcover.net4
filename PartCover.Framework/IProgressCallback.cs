namespace PartCover.Framework
{
    public interface IProgressCallback
    {
        void writeStatus(string value);
    }

    public interface IConnectorActionCallback
    {
        void DriverConnected();
        void DriverSendRules();
        void DriverWaitEoIConfirm();
        void FunctionsCount(uint count);
        void FunctionsReceiveBegin();
        void FunctionsReceiveEnd();
        void FunctionsReceiveStat(uint index);
        void InstrumentDataReceiveBegin();
        void InstrumentDataReceiveCountersAsm(string name, string mod, uint typeDefCount);
        void InstrumentDataReceiveCountersAsmCount(uint asmCount);
        void InstrumentDataReceiveCountersBegin();
        void InstrumentDataReceiveCountersEnd();
        void InstrumentDataReceiveEnd();
        void InstrumentDataReceiveFilesBegin();
        void InstrumentDataReceiveFilesCount(uint fileCount);
        void InstrumentDataReceiveFilesEnd();
        void InstrumentDataReceiveFilesStat(uint index);
        void InstrumentDataReceiveStatus();
        void MethodsReceiveBegin();
        void MethodsReceiveEnd();
        void MethodsReceiveStatus();
        void OpenMessagePipe();
        void SetConnected(bool connected);
        void TargetCreateProcess();
        void TargetRequestShutdown();
        void TargetSetEnvironmentVars();
        void TargetWaitDriver();
    }

    public class ProgressCallbackStub : IProgressCallback
    {
        public void writeStatus(string value) { }
    }

    internal class ActionCallbackStub : PartCover.IConnectorActionCallback
    {
        private IConnectorActionCallback actionCallback;
        public ActionCallbackStub(IConnectorActionCallback actionCallback)
        {
            this.actionCallback = actionCallback;
        }

        public void DriverConnected() { if (null != actionCallback) actionCallback.DriverConnected(); }

        public void DriverSendRules() { if (null != actionCallback) actionCallback.DriverSendRules(); }

        public void DriverWaitEoIConfirm() { if (null != actionCallback) actionCallback.DriverWaitEoIConfirm(); }

        public void FunctionsCount(uint count) { if (null != actionCallback) actionCallback.FunctionsCount(count); }

        public void FunctionsReceiveBegin() { if (null != actionCallback) actionCallback.FunctionsReceiveBegin(); }

        public void FunctionsReceiveEnd() { if (null != actionCallback) actionCallback.FunctionsReceiveEnd(); }

        public void FunctionsReceiveStat(uint index) { if (null != actionCallback) actionCallback.FunctionsReceiveStat(index); }

        public void InstrumentDataReceiveBegin() { if (null != actionCallback) actionCallback.InstrumentDataReceiveBegin(); }

        public void InstrumentDataReceiveCountersAsm(string name, string mod, uint typeDefCount) { if (null != actionCallback) actionCallback.InstrumentDataReceiveCountersAsm(name, mod, typeDefCount); }

        public void InstrumentDataReceiveCountersAsmCount(uint asmCount) { if (null != actionCallback) actionCallback.InstrumentDataReceiveCountersAsmCount(asmCount); }

        public void InstrumentDataReceiveCountersBegin() { if (null != actionCallback) actionCallback.InstrumentDataReceiveCountersBegin(); }

        public void InstrumentDataReceiveCountersEnd() { if (null != actionCallback) actionCallback.InstrumentDataReceiveCountersEnd(); }

        public void InstrumentDataReceiveEnd() { if (null != actionCallback) actionCallback.InstrumentDataReceiveEnd(); }

        public void InstrumentDataReceiveFilesBegin() { if (null != actionCallback) actionCallback.InstrumentDataReceiveFilesBegin(); }

        public void InstrumentDataReceiveFilesCount(uint fileCount) { if (null != actionCallback) actionCallback.InstrumentDataReceiveFilesCount(fileCount); }

        public void InstrumentDataReceiveFilesEnd() { if (null != actionCallback) actionCallback.InstrumentDataReceiveFilesEnd(); }

        public void InstrumentDataReceiveFilesStat(uint index) { if (null != actionCallback) actionCallback.InstrumentDataReceiveFilesStat(index); }

        public void InstrumentDataReceiveStatus() { if (null != actionCallback) actionCallback.InstrumentDataReceiveStatus(); }

        public void MethodsReceiveBegin() { if (null != actionCallback) actionCallback.MethodsReceiveBegin(); }

        public void MethodsReceiveEnd() { if (null != actionCallback) actionCallback.MethodsReceiveEnd(); }

        public void MethodsReceiveStatus() { if (null != actionCallback) actionCallback.MethodsReceiveStatus(); }

        public void OpenMessagePipe() { if (null != actionCallback) actionCallback.OpenMessagePipe(); }

        public void SetConnected(bool connected) { if (null != actionCallback) actionCallback.SetConnected(connected); }

        public void TargetCreateProcess() { if (null != actionCallback) actionCallback.TargetCreateProcess(); }

        public void TargetRequestShutdown() { if (null != actionCallback) actionCallback.TargetRequestShutdown(); }

        public void TargetSetEnvironmentVars() { if (null != actionCallback) actionCallback.TargetSetEnvironmentVars(); }

        public void TargetWaitDriver() { if (null != actionCallback) actionCallback.TargetWaitDriver(); }
    }
}
