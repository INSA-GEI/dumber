//
//  DestijlCommandManager.cs
//
//  Author:
//       Di MERCURIO Sébastien <dimercur@insa-toulouse.fr>
//
//  Copyright (c) 2018 INSA - DGEI
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

using System;

namespace monitor
{
    /// <summary>
    /// Commands and options parameters used in Destijl project when communicating with server
    /// </summary>
    public static class DestijlCommandList
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

    /// <summary>
    /// Commands used for robot messages
    /// </summary>
    public static class RobotCommandList
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

    /// <summary>
    /// Specialization class for command manager, which implemnent destijl protocol between monitor and server
    /// </summary>
    public class DestijlCommandManager
    {
        /// <summary>
        /// Command Manager object
        /// </summary>
        private CommandManager commandManager = null;

        /// <summary>
        /// Part of received message corresponding to command header
        /// </summary>
        private string receivedHeader = null;

        /// <summary>
        /// Part of received message corresponding to command data
        /// </summary>
        private string receivedData = null;

        /// <summary>
        /// Callback for sending received data to application level
        /// </summary>
        public delegate void CommandReceivedEvent(string header, string data, byte[] buffer);
        public CommandReceivedEvent commandReceivedEvent = null;

        /// <summary>
        /// Timeout used for command with acknowledge
        /// </summary>
        public double timeout = 100; 

        /// <summary>
        /// List of available return status
        /// </summary>
        public enum CommandStatus
        {
            Success,
            Rejected,
            InvalidAnswer,
            Busy,
            CommunicationLostWithRobot,
            CommunicationLostWithServer
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="monitor.DestijlCommandManager"/> class.
        /// </summary>
        /// <param name="callback">Callback reference for reception of data</param>
        public DestijlCommandManager(CommandReceivedEvent callback)
        {
            commandManager = new CommandManager(OnCommandReceived);
            this.commandReceivedEvent += callback;
        }

        /// <summary>
        /// Releases unmanaged resources and performs other cleanup operations before the
        /// <see cref="monitor.DestijlCommandManager"/> is reclaimed by garbage collection.
        /// </summary>
        ~DestijlCommandManager()
        {
            if (commandManager != null) commandManager.Close();
        }

        /// <summary>
        /// Callback used for receiving data from lower layer (CommandManager class)
        /// </summary>
        /// <param name="msg">String containing received message</param>
        /// <param name="buffer">Raw buffer to be used when data are not in ascii format (image for example)</param>
        private void OnCommandReceived(string msg, byte[] buffer)
        {
            // Firstly, split message in (at least) two part : header, and data
            string[] msgs = msg.Split(':');

            // If it exist at least on element in string array, it should be command header
            if (msgs.Length >= 1) receivedHeader = msgs[0];
            else receivedHeader = null;

            // if msgs array contains at least two elements, second element is normally data
            if (msgs.Length >= 2) receivedData = msgs[1];
            else receivedData = null;

            // when split is done, provide data to application
            this.commandReceivedEvent?.Invoke(receivedHeader, receivedData, buffer);
        }

        /// <summary>
        /// Open the specified hostname server, using default port number.
        /// </summary>
        /// <returns>true if connection succeded, false otherwise</returns>
        /// <param name="hostname">Hostname to connect to</param>
        public bool Open(string hostname)
        {
            return this.Open(hostname, Client.defaultPort);
        }

        /// <summary>
        /// Open connection to server "host", with port number "port"
        /// </summary>
        /// <returns>true if connection succeded, false otherwise</returns>
        /// <param name="hostname">Hostname to connect to</param>
        /// <param name="port">Port number for connection</param>
        public bool Open(string hostname, int port)
        {
            if (commandManager != null) return commandManager.Open(hostname, port);
            else return false;
        }

        /// <summary>
        /// Close connection to server
        /// </summary>
        public void Close()
        {
            if (commandManager != null) commandManager.Close();
        }

        /// <summary>
        /// Creates the command to send to server, based on header and data provided
        /// </summary>
        /// <returns>The command string</returns>
        /// <param name="header">Header part of the command</param>
        /// <param name="data">Data part of the command</param>
        private string CreateCommand(string header, string data)
        {
            return header + ":" + data;
        }

        /// <summary>
        /// Provide DestijlCommandManager.CommandStatus based on status received by CommandManager.SendCommand and answer string 
        /// </summary>
        /// <returns>Status compatible with DestijlCommandManager.CommandStatus type</returns>
        /// <param name="localStatus">Status provided by CommandManager.SendCommand</param>
        /// <param name="answer">Answer provided by CommandManager.SendCommand</param>
        private CommandStatus DecodeStatus(CommandManager.CommandManagerStatus localStatus, string answer)
        {
            CommandStatus status = CommandStatus.Success;

            // if timeout occures, return CommandStatus.CommunicationLostWithServer
            if (localStatus == CommandManager.CommandManagerStatus.Timeout) status = CommandStatus.CommunicationLostWithServer;
            // if a command is currently processed, return Busy
            else if (localStatus == CommandManager.CommandManagerStatus.Busy) status = CommandStatus.Busy;
            else
            {
                if (answer != null)
                {
                    // if command is not acknowledged, return Rejected
                    if (answer.ToUpper().Contains(DestijlCommandList.HeaderStmNoAck)) status = CommandStatus.Rejected;
                    // if communication is lost with robot, return CommunicationLostWithRobot
                    else if (answer.ToUpper().Contains(DestijlCommandList.HeaderStmLostDmb)) status = CommandStatus.CommunicationLostWithRobot;
                    // if answer is empty, communication with robot is lost
                    else if (answer.Length == 0) status = CommandStatus.CommunicationLostWithServer;
                    //else status = CommandStatus.InvalidAnswer;
                }
            }

            return status;
        }

        /// <summary>
        /// Open communication with robot and wait acknowledge
        /// </summary>
        /// <returns>Command status (see DecodeStatus)</returns>
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

        /// <summary>
        /// Close communication with robot and wait acknowledge
        /// </summary>
        /// <returns>Command status (see DecodeStatus)</returns>
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

        /// <summary>
        /// Ping the robot.
        /// </summary>
        /// <returns>Command status (see DecodeStatus)</returns>
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

        /// <summary>
        /// Reset robot and let it in idle mode
        /// </summary>
        /// <returns>Command status (see DecodeStatus)</returns>
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

        /// <summary>
        /// Start robot, enabling watchdog
        /// </summary>
        /// <returns>Command status (see DecodeStatus)</returns>
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

        /// <summary>
        /// Start robot, without enabling watchdog 
        /// </summary>
        /// <returns>Command status (see DecodeStatus)</returns>
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

        /// <summary>
        /// Move robot forward or backward, for a distance expressed in millimeter
        /// </summary>
        /// <returns>Command status (see DecodeStatus)</returns>
        /// <param name="distance">Distance of mouvment, in millimeter</param>
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

        /// <summary>
        /// Make robot turn left or right, for a given angle
        /// </summary>
        /// <returns>Command status (see DecodeStatus)</returns>
        /// <param name="angle">Angle of turn, in degree (negative for left, positive for right)</param>
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

        /// <summary>
        /// Request robot battery level
        /// </summary>
        /// <returns>Command status (see DecodeStatus)</returns>
        public CommandStatus RobotGetBattery()
        {
            CommandManager.CommandManagerStatus localStatus;
            string answer;

            localStatus = commandManager.SendCommand(
                CreateCommand(DestijlCommandList.HeaderMtsDmbOrder, RobotCommandList.RobotGetBattery),
                out answer,
                0);

            return DecodeStatus(localStatus, answer);
        }

        /// <summary>
        /// Request robot firmware version
        /// </summary>
        /// <returns>Command status (see DecodeStatus)</returns>
        /// <param name="version">todo</param>
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

        /// <summary>
        /// Power off robot
        /// </summary>
        /// <returns>Command status (see DecodeStatus)</returns>
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

        /// <summary>
        /// Open camera on remote device
        /// </summary>
        /// <returns>Command status (see DecodeStatus)</returns>
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

        /// <summary>
        /// Close camera on remote device
        /// </summary>
        /// <returns>Command status (see DecodeStatus)</returns>
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

        /// <summary>
        /// Request still image of detected arena
        /// </summary>
        /// <returns>Command status (see DecodeStatus)</returns>
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

        /// <summary>
        /// Confirm arena detection (after requesting image of detected arena, using CameraAskArena
        /// </summary>
        /// <returns>Command status (see DecodeStatus)</returns>
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

        /// <summary>
        /// Reject arena detected (after requesting image of detected arena, using CameraAskArena
        /// </summary>
        /// <returns>Command status (see DecodeStatus)</returns>
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

        /// <summary>
        /// Request robot position computing
        /// </summary>
        /// <returns>Command status (see DecodeStatus)</returns>
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

        /// <summary>
        /// Stop robot position computing
        /// </summary>
        /// <returns>Command status (see DecodeStatus)</returns>
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
