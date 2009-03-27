using PartCover.Framework.Data;

namespace PartCover.Framework
{
    public class ConnectorActionCallback : IConnectorActionCallback
    {
        private readonly Connector connector;

        public ConnectorActionCallback(Connector connector)
        {
            this.connector = connector;
        }

        void IConnectorActionCallback.DriverConnected() { connector.OnStatusReceive("driver connected successfully"); }

        void IConnectorActionCallback.DriverSendRules() { connector.OnStatusReceive("send rules to the driver"); }

        void IConnectorActionCallback.DriverWaitEoIConfirm() { connector.OnStatusReceive("wait for rules confirm"); }

        void IConnectorActionCallback.FunctionsCount(uint count) { connector.OnStatusReceive(string.Format("functions count {0}", count)); }

        void IConnectorActionCallback.FunctionsReceiveBegin() { connector.OnStatusReceive("functions loading is being started"); }

        void IConnectorActionCallback.FunctionsReceiveEnd() { connector.OnStatusReceive("functions loading is complete"); }

        void IConnectorActionCallback.FunctionsReceiveStat(uint index) { connector.OnStatusReceive(string.Format("functions loaded: {0}", index)); }

        void IConnectorActionCallback.InstrumentDataReceiveBegin() { connector.OnStatusReceive("instrument data is being received"); }

        void IConnectorActionCallback.InstrumentDataReceiveCountersAsm(string name, string mod, uint typeDefCount) { connector.OnStatusReceive(string.Format("load assembly {0} ({1} types)", name, typeDefCount)); }

        void IConnectorActionCallback.InstrumentDataReceiveCountersAsmCount(uint asmCount) { connector.OnStatusReceive(string.Format("assembly count: {0}", asmCount)); }

        void IConnectorActionCallback.InstrumentDataReceiveCountersBegin() { connector.OnStatusReceive("InstrumentDataReceiveCountersBegin"); }

        void IConnectorActionCallback.InstrumentDataReceiveCountersEnd() { connector.OnStatusReceive("InstrumentDataReceiveCountersEnd"); }

        void IConnectorActionCallback.InstrumentDataReceiveEnd() { connector.OnStatusReceive("instrument data load complete"); }

        void IConnectorActionCallback.InstrumentDataReceiveFilesBegin() { connector.OnStatusReceive("file list is being received"); }

        void IConnectorActionCallback.InstrumentDataReceiveFilesCount(uint fileCount) { connector.OnStatusReceive(string.Format("{0} files to load", fileCount)); }

        void IConnectorActionCallback.InstrumentDataReceiveFilesEnd() { connector.OnStatusReceive("file list load is complete"); }

        void IConnectorActionCallback.InstrumentDataReceiveFilesStat(uint index) { }

        void IConnectorActionCallback.InstrumentDataReceiveStatus() { connector.OnStatusReceive("driver connected"); }

        void IConnectorActionCallback.MethodsReceiveBegin() { connector.OnStatusReceive("function map is being received"); }

        void IConnectorActionCallback.MethodsReceiveEnd() { connector.OnStatusReceive("function map load complete"); }

        void IConnectorActionCallback.MethodsReceiveStatus() { connector.OnStatusReceive("function map load status"); }

        void IConnectorActionCallback.OpenMessagePipe() { connector.OnStatusReceive("open driver pipe"); }

        void IConnectorActionCallback.SetConnected(bool connected) { connector.OnStatusReceive(connected ? "driver connected" : "driver disconnected"); }

        void IConnectorActionCallback.TargetCreateProcess() { connector.OnStatusReceive("create target process"); }

        void IConnectorActionCallback.TargetRequestShutdown() { connector.OnStatusReceive("request target shutdown"); }

        void IConnectorActionCallback.TargetSetEnvironmentVars() { connector.OnStatusReceive("modify target environment variables"); }

        void IConnectorActionCallback.TargetWaitDriver() { connector.OnStatusReceive("wait for driver connection"); }

        void IConnectorActionCallback.LogMessage(int threadId, int tick, string text)
        {
            var message = new LogEntry
            {
                Message = text, 
                MsOffset = tick, 
                ThreadId = threadId
            };
            connector.OnLogMessage(message);
        }
    }
}
