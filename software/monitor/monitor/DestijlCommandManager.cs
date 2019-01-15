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

// 15/01/2019 dimercur
// Demande #41: Modifier les messages envoyés par les flèches de direction

using System;
using System.Globalization;

namespace monitor
{
    /// <summary>
    /// Commands and options parameters used in Destijl project when communicating with server
    /// </summary>
    public static class DestijlCommandList
    {
        public const string ANSWER_ACK = "AACK";
        public const string ANSWER_NACK = "ANAK";
        public const string ANSWER_COM_ERROR = "ACER";
        public const string ANSWER_TIMEOUT = "ATIM";
        public const string ANSWER_CMD_REJECTED = "ACRJ";
        public const string MESSAGE = "MSSG";
        public const string CAMERA_OPEN = "COPN";
        public const string CAMERA_CLOSE = "CCLS";
        public const string CAMERA_IMAGE = "CIMG";
        public const string CAMERA_ARENA_ASK = "CASA";
        public const string CAMERA_ARENA_INFIRM = "CAIN";
        public const string CAMERA_ARENA_CONFIRM = "CACO";
        public const string CAMERA_POSITION_COMPUTE = "CPCO";
        public const string CAMERA_POSITION_STOP = "CPST";
        public const string CAMERA_POSITION = "CPOS";
        public const string ROBOT_COM_OPEN = "ROPN";
        public const string ROBOT_COM_CLOSE = "RCLS";
        public const string ROBOT_PING = "RPIN";
        public const string ROBOT_RESET = "RRST";
        public const string ROBOT_START_WITHOUT_WD = "RSOW";
        public const string ROBOT_START_WITH_WD = "RSWW";
        public const string ROBOT_RELOAD_WD = "RLDW";
        public const string ROBOT_MOVE = "RMOV";
        public const string ROBOT_TURN = "RTRN";
        public const string ROBOT_GO_FORWARD = "RGFW";
        public const string ROBOT_GO_BACKWARD = "RGBW";
        public const string ROBOT_GO_LEFT = "RGLF";
        public const string ROBOT_GO_RIGHT = "RGRI";
        public const string ROBOT_STOP = "RSTP";
        public const string ROBOT_POWEROFF = "RPOF";
        public const string ROBOT_BATTERY_LEVEL = "RBLV";
        public const string ROBOT_GET_BATTERY = "RGBT";
        public const string ROBOT_GET_STATE = "RGST";
        public const string ROBOT_CURRENT_STATE = "RCST";

        public const char SEPARATOR_CHAR = ':';
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
        public delegate void CommandReceivedEvent(string header, string data);
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

        public struct Point {
            public double x;
            public double y;
        } 

        public class Position {
            public int robotID;
            public double angle;
            public Point centre;
            public Point direction;

            public Position() {
                robotID = 0;
                angle = 0.0;
                centre.x = 0.0;
                centre.y = 0.0;
                direction.x = 0.0;
                direction.y = 0.0;
            }

            public override string ToString() {
                string s = "ID: " + robotID + ", Angle: " + angle +
                    ", Centre (x: " + centre.x + ", y: " + centre.y +
                                          "), Direction (x: " + direction.x + ", y: " + direction.y + ")";
                return s;
            }
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
        private void OnCommandReceived(string msg)
        {
            // Firstly, remove ending \n and everything after
            string[] msgsCarriageReturn = msg.Split('\n');

            // Second, split message in (at least) two part : header, and data
            string[] msgs = msgsCarriageReturn[0].Split(DestijlCommandList.SEPARATOR_CHAR);

            // If it exist at least on element in string array, it should be command header
            if (msgs.Length >= 1) receivedHeader = msgs[0];
            else receivedHeader = null;

            // if msgs array contains at least two elements, second element is normally data
            if (msgs.Length >= 2) receivedData = msgs[1];
            else receivedData = null;

            // when split is done, provide data to application
            this.commandReceivedEvent?.Invoke(receivedHeader, receivedData);
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
            return header + DestijlCommandList.SEPARATOR_CHAR + data+"\n";
        }

        /// <summary>
        /// Creates the command to send to server, based on header
        /// </summary>
        /// <returns>The command string</returns>
        /// <param name="header">Header part of the command</param>
        private string CreateCommand(string header)
        {
            return header + DestijlCommandList.SEPARATOR_CHAR+"\n";
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
                    if (answer.ToUpper().Contains(DestijlCommandList.ANSWER_NACK)) status = CommandStatus.Rejected;
                    // if communication is lost with robot, return CommunicationLostWithRobot
                    else if (answer.ToUpper().Contains(DestijlCommandList.ANSWER_TIMEOUT)) status = CommandStatus.CommunicationLostWithRobot;
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
                CreateCommand(DestijlCommandList.ROBOT_COM_OPEN),
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
                CreateCommand(DestijlCommandList.ROBOT_COM_CLOSE),
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
                CreateCommand(DestijlCommandList.ROBOT_PING),
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
                CreateCommand(DestijlCommandList.ROBOT_RESET),
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
                CreateCommand(DestijlCommandList.ROBOT_START_WITH_WD),
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
                CreateCommand(DestijlCommandList.ROBOT_START_WITHOUT_WD),
                out answer,
                this.timeout);

            return DecodeStatus(localStatus, answer);
        }

        /// <summary>
        /// Move robot forward for an unlimited distance
        /// </summary>
        /// <returns>Command status (see DecodeStatus)</returns>
        public CommandStatus RobotGoForward()
        {
            CommandManager.CommandManagerStatus localStatus;
            string answer;

            localStatus = commandManager.SendCommand(
                CreateCommand(DestijlCommandList.ROBOT_GO_FORWARD),
                out answer,
                0);

            return DecodeStatus(localStatus, answer);
        }

        /// <summary>
        /// Move robot backward for an unlimited distance
        /// </summary>
        /// <returns>Command status (see DecodeStatus)</returns>
        public CommandStatus RobotGoBackward()
        {
            CommandManager.CommandManagerStatus localStatus;
            string answer;

            localStatus = commandManager.SendCommand(
                CreateCommand(DestijlCommandList.ROBOT_GO_BACKWARD),
                out answer,
                0);

            return DecodeStatus(localStatus, answer);
        }

        /// <summary>
        /// Turn robot to the left for an unlimited number of turn
        /// </summary>
        /// <returns>Command status (see DecodeStatus)</returns>
        public CommandStatus RobotGoLeft()
        {
            CommandManager.CommandManagerStatus localStatus;
            string answer;

            localStatus = commandManager.SendCommand(
                CreateCommand(DestijlCommandList.ROBOT_GO_LEFT),
                out answer,
                0);

            return DecodeStatus(localStatus, answer);
        }

        /// <summary>
        /// Turn robot to the right for an unlimited number of turn
        /// </summary>
        /// <returns>Command status (see DecodeStatus)</returns>
        public CommandStatus RobotGoRight()
        {
            CommandManager.CommandManagerStatus localStatus;
            string answer;

            localStatus = commandManager.SendCommand(
                CreateCommand(DestijlCommandList.ROBOT_GO_RIGHT),
                out answer,
                0);

            return DecodeStatus(localStatus, answer);
        }

        /// <summary>
        /// Stop robot mouvement
        /// </summary>
        /// <returns>Command status (see DecodeStatus)</returns>
        public CommandStatus RobotStop()
        {
            CommandManager.CommandManagerStatus localStatus;
            string answer;

            localStatus = commandManager.SendCommand(
                CreateCommand(DestijlCommandList.ROBOT_STOP),
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
                CreateCommand(DestijlCommandList.ROBOT_TURN, Convert.ToString(angle)),
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
                CreateCommand(DestijlCommandList.ROBOT_GET_BATTERY),
                out answer,
                0);

            return DecodeStatus(localStatus, answer);
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
                CreateCommand(DestijlCommandList.ROBOT_POWEROFF),
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
                CreateCommand(DestijlCommandList.CAMERA_OPEN),
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
                CreateCommand(DestijlCommandList.CAMERA_CLOSE),
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
                CreateCommand(DestijlCommandList.CAMERA_ARENA_ASK),
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
                CreateCommand(DestijlCommandList.CAMERA_ARENA_CONFIRM),
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
                CreateCommand(DestijlCommandList.CAMERA_ARENA_INFIRM),
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
                CreateCommand(DestijlCommandList.CAMERA_POSITION_COMPUTE),
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
                CreateCommand(DestijlCommandList.CAMERA_POSITION_STOP),
                out answer,
                0);

            return DecodeStatus(localStatus, answer);
        }

        public static Position DecodePosition(string data) {
            Position pos = new Position();

            pos.robotID = 0;
            pos.angle = 0.0;
            pos.centre.x = 0.0; 
            pos.centre.y=0.0;
            pos.direction.x = 0.0;
            pos.direction.y = 0.0;

            string[] parts = data.Split(';');

            NumberFormatInfo provider = new NumberFormatInfo();
            provider.NumberDecimalSeparator = ".";
            provider.NumberGroupSeparator = ",";
            provider.NumberGroupSizes = new int[] { 3 };

            if (parts.Length == 6) {
                pos.robotID = Convert.ToInt32(parts[0]);

                try
                {
                    pos.angle = Convert.ToDouble(parts[1]);
                } catch (FormatException)
                {
                    pos.angle = Convert.ToDouble(parts[1],provider);
                }

                try
                {
                    pos.centre.x = Convert.ToDouble(parts[2]);
                } catch (FormatException)
                {
                    pos.centre.x = Convert.ToDouble(parts[2], provider);
                }

                try
                {
                    pos.centre.y = Convert.ToDouble(parts[3]);
                }
                catch (FormatException)
                {
                    pos.centre.y = Convert.ToDouble(parts[3], provider);
                }

                try
                {
                    pos.direction.x = Convert.ToDouble(parts[4]);
                }
                catch (FormatException)
                {
                    pos.direction.x = Convert.ToDouble(parts[4], provider);
                }

                try
                {
                    pos.direction.y = Convert.ToDouble(parts[5]);
                }
                catch (FormatException)
                {
                    pos.direction.y = Convert.ToDouble(parts[5], provider);
                }

            } else {
                // misformatted data, return 0 filled position
                Console.WriteLine("Misformated position");
            }

            return pos;
        }
    }
}
