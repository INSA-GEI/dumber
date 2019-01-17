//
//  CommandManager.cs
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

using System.Threading;

namespace monitor
{
    /// <summary>
    /// Command Manager. Use for timeout managment during reception of data
    /// Used as intermediate layer between TCP client class (Client) and application level
    /// managment of command and answers
    /// </summary>
    public class CommandManager
    {
        /// <summary>
        /// Callback for sending received data to upper level
        /// </summary>
        public delegate void CommandReceivedEvent(string msg);
        public CommandReceivedEvent commandReceivedEvent = null;

        /// <summary>
        /// Timer for managing timeout
        /// </summary>
        private System.Timers.Timer waitTimer = new System.Timers.Timer();
        private ManualResetEvent waitEvent = new ManualResetEvent(false);

        /// <summary>
        /// Flag to tell rogram to wait for an acknowledge from server
        /// </summary>
        private bool waitForAcknowledge = false;

        /// <summary>
        /// received message
        /// </summary>
        private string messageReceived = null;

        /// <summary>
        /// flag indicating command manager is currently busy waiting an acknowledge
        /// </summary>
        private bool isBusy = false;

        /// <summary>
        /// Available status when sending command
        /// </summary>
        public enum CommandManagerStatus
        {
            AnswerReceived,
            Timeout,
            Busy
        };

        /// <summary>
        /// Initializes a new instance of the <see cref="T:monitor.CommandManager"/> class.
        /// </summary>
        /// <param name="callback">Callback used when new message are received</param>
        public CommandManager(CommandReceivedEvent callback)
        {
            Client.readEvent += this.OnMessageReception;

            this.commandReceivedEvent += callback;
            waitTimer.Elapsed += OnMessageTimeout;
        }

        /// <summary>
        /// Releases unmanaged resources and performs other cleanup operations before the
        /// <see cref="T:monitor.CommandManager"/> is reclaimed by garbage collection.
        /// </summary>
        ~CommandManager()
        {
            Client.Close();
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
            return Client.Open(hostname, port);
        }

        /// <summary>
        /// Close connection to server
        /// </summary>
        public void Close()
        {
            Client.Close();
        }

        /// <summary>
        /// Callback called by Client class after reception of new message
        /// </summary>
        /// <param name="message">Message received from server</param>
        /// <param name="buffer">Raw buffer reived from server</param>
        private void OnMessageReception(string message)
        {
            waitTimer.Stop(); // Stop timeout stopwatch

            this.messageReceived = message;
            isBusy = false;

            // if SendCommand wait for an acknowledge, release semaphore waitEvent
            // so that SendCommand will be able to read received answer
            // Received answer will not be sent to upper level
            if (waitForAcknowledge)
            {
                waitForAcknowledge = false;
                waitEvent.Set(); // Envoi de l'evenement
            }
            else
                // if sendCommand doesn't wait for an acknowledge, message received
                // is for upper level, so call callback
            {
                
                waitForAcknowledge = false;

                this.commandReceivedEvent?.Invoke(message);
            }
        }

        /// <summary>
        /// Callback called by stopwatch on timeout
        /// </summary>
        /// <param name="sender">Sender object</param>
        /// <param name="e">Information on elapsed condition</param>
        private void OnMessageTimeout(object sender, System.Timers.ElapsedEventArgs e)
        {
            messageReceived = null;
            // set buffer and message as null to indicate that no message was received
            // and call to OnMessagereception is due to timeout
            OnMessageReception(messageReceived);
        }

        /// <summary>
        /// Sends a command to TCP server
        /// </summary>
        /// <returns>status that is part of CommandManagerStatus enumerate</returns>
        /// <param name="cmd">Command message to send to server</param>
        /// <param name="answer">Answer from server, in case of acknowledge</param>
        /// <param name="timeout">Timeout (ms) waiting an acknowledge, 0 if no acknowledge needed</param>
        public CommandManagerStatus SendCommand(string cmd, out string answer, double timeout)
        {
            CommandManagerStatus status = CommandManagerStatus.AnswerReceived;
            answer = null;


            if (isBusy) status = CommandManagerStatus.Busy;
            else
            {
                isBusy = true;

                // Send command to server
                Client.Write(cmd);

                if (timeout > 0) // Command request an acknowledge
                {
                    
                    waitForAcknowledge = true;  // Flag used in OnMessageReception callback to avoid
                                                // sending acknowledge message to upper level
                    waitTimer.Interval = timeout;
                    waitTimer.Start();          // Start timeout timer

                    waitEvent.WaitOne();        // Stop current thread, waiting for waitEvent semaphore 
                                                // produced in OnMessageReception when either a message is received
                                                // or a timeout occur

                    waitEvent.Reset();          // reset semaphore for next message

                    if (this.messageReceived == null) // timeout: server connection error
                    {
                        status = CommandManagerStatus.Timeout;
                    }
                }
                else isBusy = false;

                // return received answer, null in case of timeout
                answer = this.messageReceived;
                this.messageReceived = null;
            }

            return status;
        }
    }
}
