using System;

namespace monitor
{
    public class DestijlCommandList
    {
        public const string HeaderMtsComDmb = "COM";
        public const string HeaderMtsDmbOrder = "DMB";
        public const string HeaderMtsCamera = "CAM";
        public const string HeaderMtsMessage = "MSG";

        public const string DataComOpen = "o";
        public const string DataComClose = "C";

        public const string DataCamOpen = "A";
        public const string DataCamClose = "I";
        public const string DataCamAskArena = "y";
        public const string DataCamArenaConfirm = "x";
        public const string DataCamInfirm = "z";
        public const string DataCamComputePosition = "p";
        public const string DataCamStopComputePosition = "s";

        public const string HeaderStmAck = "ACK";
        public const string HeaderStmNoAck = "NAK";
        public const string HeaderStmLostDmb = "LCD";
        public const string HeaderStmImage = "IMG";
        public const string HeaderStmPos = "POS";
        public const string HeaderStmMes = "MSG";
        public const string HeaderStmBat = "BAT";
    }

    public class RobotCommandList
    {
        public const string RobotPing = "p";
        public const string RobotReset = "r";
        public const string RobotStartWithoutWatchdog = "u";
        public const string RobotStartWithWatchdog = "W";
        public const string RobotGetBattery = "v";
        public const string RobotGetBusyState = "b";
        public const string RobotMove = "M";
        public const string RobotTurn = "T";
        public const string RobotGetVersion = "V";
        public const string RobotPowerOff = "z";
    }

    public class DestijlCommandManager
    {
        private CommandManager commandManager = null;

        private string receivedHeader = null;
        private string receivedData = null;

        public delegate void CommandReceivedEvent(string header, string data, byte[] buffer);
        public CommandReceivedEvent commandReceivedEvent = null;

        public double timeout = 100; // timeout pour les commandes avec acquitement

        public enum CommandStatus
        {
            Success,
            Rejected,
            InvalidAnswer,
            Busy,
            CommunicationLostWithRobot,
            CommunicationLostWithServer
        }

        public DestijlCommandManager(CommandReceivedEvent callback)
        {
            commandManager = new CommandManager(OnCommandReceived);
            this.commandReceivedEvent += callback;
        }

        ~DestijlCommandManager()
        {
            if (commandManager != null) commandManager.Close();
        }

        private void OnCommandReceived(string msg, byte[] buffer)
        {
            string[] msgs = msg.Split(':');

            if (msgs.Length >= 1) receivedHeader = msgs[0];
            else receivedHeader = null;

            if (msgs.Length >= 2) receivedData = msgs[1];
            else receivedData = null;

            this.commandReceivedEvent?.Invoke(receivedHeader, receivedData, buffer);
        }

        public bool Open(string hostname)
        {
            return this.Open(hostname, Client.defaultPort);
        }

        public bool Open(string hostname, int port)
        {
            if (commandManager != null) return commandManager.Open(hostname, port);
            else return false;
        }

        public void Close()
        {
            if (commandManager != null) commandManager.Close();
        }

        private string CreateCommand(string header, string data)
        {
            return header + ":" + data;
        }

        private void SplitCommand(string cmd, out string header, out string data)
        {
            string[] cmdParts = cmd.Split(':');

            if (cmdParts.Length > 0) header = cmdParts[0];
            else header = null;

            if (cmdParts.Length > 1) data = cmdParts[1];
            else data = null;
        }

        private CommandStatus DecodeStatus(CommandManager.CommandManagerStatus localStatus, string answer)
        {
            CommandStatus status = CommandStatus.Success;

            if (localStatus == CommandManager.CommandManagerStatus.Timeout) status = CommandStatus.CommunicationLostWithServer;
            else if (localStatus == CommandManager.CommandManagerStatus.Busy) status = CommandStatus.Busy;
            else
            {
                if (answer != null)
                {
                    if (answer.ToUpper().Contains(DestijlCommandList.HeaderStmNoAck)) status = CommandStatus.Rejected;
                    else if (answer.ToUpper().Contains(DestijlCommandList.HeaderStmLostDmb)) status = CommandStatus.CommunicationLostWithRobot;
                    else if (answer.Length == 0) status = CommandStatus.CommunicationLostWithServer;
                    //else status = CommandStatus.InvalidAnswer;
                }
            }

            return status;
        }

        public CommandStatus RobotOpenCom()
        {
            CommandManager.CommandManagerStatus localStatus;
            string answer;

            localStatus = commandManager.SendCommand(
                CreateCommand(DestijlCommandList.HeaderMtsComDmb, DestijlCommandList.DataComOpen),
                out answer,
                this.timeout);

            return DecodeStatus(localStatus, answer);
        }

        public CommandStatus RobotCloseCom()
        {
            CommandManager.CommandManagerStatus localStatus;
            string answer;

            localStatus = commandManager.SendCommand(
                CreateCommand(DestijlCommandList.HeaderMtsComDmb, DestijlCommandList.DataComClose),
                out answer,
                this.timeout);

            return DecodeStatus(localStatus, answer);
        }

        public CommandStatus RobotPing()
        {
            CommandManager.CommandManagerStatus localStatus;
            string answer;

            localStatus = commandManager.SendCommand(
                CreateCommand(DestijlCommandList.HeaderMtsDmbOrder, RobotCommandList.RobotPing),
                out answer,
                this.timeout);

            return DecodeStatus(localStatus, answer);
        }

        public CommandStatus RobotReset()
        {
            CommandManager.CommandManagerStatus localStatus;
            string answer;

            localStatus = commandManager.SendCommand(
                CreateCommand(DestijlCommandList.HeaderMtsDmbOrder, RobotCommandList.RobotReset),
                out answer,
                0);

            return DecodeStatus(localStatus, answer);
        }

        public CommandStatus RobotStartWithWatchdog()
        {
            CommandManager.CommandManagerStatus localStatus;
            string answer;

            localStatus = commandManager.SendCommand(
                CreateCommand(DestijlCommandList.HeaderMtsDmbOrder, RobotCommandList.RobotStartWithWatchdog),
                out answer,
                this.timeout);

            return DecodeStatus(localStatus, answer);
        }

        public CommandStatus RobotStartWithoutWatchdog()
        {
            CommandManager.CommandManagerStatus localStatus;
            string answer;

            localStatus = commandManager.SendCommand(
                CreateCommand(DestijlCommandList.HeaderMtsDmbOrder, RobotCommandList.RobotStartWithoutWatchdog),
                out answer,
                this.timeout);

            return DecodeStatus(localStatus, answer);
        }

        public CommandStatus RobotMove(int distance)
        {
            CommandManager.CommandManagerStatus localStatus;
            string answer;

            localStatus = commandManager.SendCommand(
                CreateCommand(DestijlCommandList.HeaderMtsDmbOrder, RobotCommandList.RobotMove + "=" + distance),
                out answer,
                0);

            return DecodeStatus(localStatus, answer);
        }

        public CommandStatus RobotTurn(int angle)
        {
            CommandManager.CommandManagerStatus localStatus;
            string answer;

            localStatus = commandManager.SendCommand(
                CreateCommand(DestijlCommandList.HeaderMtsDmbOrder, RobotCommandList.RobotTurn + "=" + angle),
                out answer,
                0);

            return DecodeStatus(localStatus, answer);
        }

        //public CommandStatus RobotGetBattery(out int battery)
        public CommandStatus RobotGetBattery()
        {
            CommandManager.CommandManagerStatus localStatus;
            //CommandStatus status = CommandStatus.Success;

            //battery = -1;

            string answer;

            localStatus = commandManager.SendCommand(
                CreateCommand(DestijlCommandList.HeaderMtsDmbOrder, RobotCommandList.RobotGetBattery),
                out answer,
                0);

            //if (localStatus == CommandManager.CommandManagerStatus.AnswerReceived) {
            //    string[] msg = answer.Split(':');

            //    if (msg.Length > 1)
            //    {
            //        try
            //        {
            //            battery = Convert.ToInt32(msg[1]);
            //        }
            //        catch (Exception) { }
            //    }
            //}
            //else if (localStatus == CommandManager.CommandManagerStatus.Timeout)
            //{
            //    status = CommandStatus.CommunicationLostWithServer;
            //}

            //return status;
            return DecodeStatus(localStatus, answer);
        }

        public CommandStatus RobotGetVersion(out string version)
        {
            CommandManager.CommandManagerStatus localStatus;
            CommandStatus status = CommandStatus.Success;

            version = "";

            string answer;

            localStatus = commandManager.SendCommand(
                CreateCommand(DestijlCommandList.HeaderMtsDmbOrder, RobotCommandList.RobotGetVersion),
                out answer,
                this.timeout);

            if (localStatus == CommandManager.CommandManagerStatus.AnswerReceived)
            {
                string[] msg = answer.Split(':');

                if (msg.Length > 1)
                {
                    version = msg[1];
                }
            }
            else if (localStatus == CommandManager.CommandManagerStatus.Timeout)
            {
                status = CommandStatus.CommunicationLostWithServer;
            }

            return status;
        }

        public CommandStatus RobotPowerOff()
        {
            CommandManager.CommandManagerStatus localStatus;
            string answer;

            localStatus = commandManager.SendCommand(
                CreateCommand(DestijlCommandList.HeaderMtsDmbOrder, RobotCommandList.RobotPowerOff),
                out answer,
                0);

            return DecodeStatus(localStatus, answer);
        }

        public CommandStatus CameraOpen()
        {
            CommandManager.CommandManagerStatus localStatus;
            string answer;

            localStatus = commandManager.SendCommand(
                CreateCommand(DestijlCommandList.HeaderMtsCamera, DestijlCommandList.DataCamOpen),
                out answer,
                this.timeout);

            return DecodeStatus(localStatus, answer);
        }

        public CommandStatus CameraClose()
        {
            CommandManager.CommandManagerStatus localStatus;
            string answer;

            localStatus = commandManager.SendCommand(
                CreateCommand(DestijlCommandList.HeaderMtsCamera, DestijlCommandList.DataCamClose),
                out answer,
                0);

            return DecodeStatus(localStatus, answer);
        }

        public CommandStatus CameraAskArena()
        {
            CommandManager.CommandManagerStatus localStatus;
            string answer;

            localStatus = commandManager.SendCommand(
                CreateCommand(DestijlCommandList.HeaderMtsCamera, DestijlCommandList.DataCamAskArena),
                out answer,
                0);

            return DecodeStatus(localStatus, answer);
        }

        public CommandStatus CameraArenaConfirm()
        {
            CommandManager.CommandManagerStatus localStatus;
            string answer;

            localStatus = commandManager.SendCommand(
                CreateCommand(DestijlCommandList.HeaderMtsCamera, DestijlCommandList.DataCamArenaConfirm),
                out answer,
                0);

            return DecodeStatus(localStatus, answer);
        }

        public CommandStatus CameraArenaInfirm()
        {
            CommandManager.CommandManagerStatus localStatus;
            string answer;

            localStatus = commandManager.SendCommand(
                CreateCommand(DestijlCommandList.HeaderMtsCamera, DestijlCommandList.DataCamInfirm),
                out answer,
                0);

            return DecodeStatus(localStatus, answer);
        }

        public CommandStatus CameraComputePosition()
        {
            CommandManager.CommandManagerStatus localStatus;
            string answer;

            localStatus = commandManager.SendCommand(
                CreateCommand(DestijlCommandList.HeaderMtsCamera, DestijlCommandList.DataCamComputePosition),
                out answer,
                0);

            return DecodeStatus(localStatus, answer);
        }

        public CommandStatus CameraStopComputePosition()
        {
            CommandManager.CommandManagerStatus localStatus;
            string answer;

            localStatus = commandManager.SendCommand(
                CreateCommand(DestijlCommandList.HeaderMtsCamera, DestijlCommandList.DataCamStopComputePosition),
                out answer,
                0);

            return DecodeStatus(localStatus, answer);
        }
    }
}
