//
//  Client.cs
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
using System.Net.Sockets;
using System.Text;

namespace monitor
{
    /// <summary>
    /// Static class for TCP client
    /// </summary>
    public static class Client
    {
        /// <summary>
        /// Default server name
        /// </summary>
        public const string defaultIP = "localhost";

        /// <summary>
        /// Default server port number
        /// </summary>
        public const int defaultPort = 5544;

        /// <summary>
        /// Tcp client object
        /// </summary>
        private static TcpClient client = null;

        /// <summary>
        /// Stream object used for communication
        /// </summary>
        private static NetworkStream stream = null;

        /// <summary>
        /// Size of internal buffer used when reading data from server
        /// </summary>
        private const int BufferMaxSize = 512;

        /// <summary>
        /// Internal buffer used when reading data from server
        /// </summary>
        private static byte[] buffer = new byte[BufferMaxSize];

        /// <summary>
        /// buffer containing received message from TCP server
        /// Used to concatenate internal buffers into one
        /// </summary>
        private static byte[] receiveBuffer;

        private static int initialReceiveBufferIndex = 0;

        /// <summary>
        /// String containing received message from tcp server
        /// </summary>
        private static StringBuilder message = new StringBuilder();
        private static int newLength = 1;
        private static int packetCounter = 0;

        /// <summary>
        /// Callback to send received message to upper level
        /// </summary>
        public delegate void ReadEvent(string msg, byte[] buffer);
        public static ReadEvent readEvent = null;

        /// <summary>
        /// Open connection to server "host", on default port number.
        /// </summary>
        /// <returns>true if connection succeded, false otherwise</returns>
        /// <param name="host">Hostname to connect to</param>
        public static bool Open(string host)
        {
            return Client.Open(host, defaultPort);
        }

        /// <summary>
        /// Open connection to server "host", with port number "port"
        /// </summary>
        /// <returns>true if connection succeded, false otherwise</returns>
        /// <param name="host">Hostname to connect to</param>
        /// <param name="port">Port number for connection</param>
        public static bool Open(string host, int port)
        {
            bool status = true;

            try
            {
                client = new TcpClient(host, port);

                stream = client.GetStream();

                // Start reading tcp stream and call "ReadCallback" method when newLength data
                // will be received
                // initially, "newLength" is equal to 1, so first call to ReadCallback
                // will be done after reception of 1 byte.

                // received data are stored in buffer
                // Next reading will be done in ReadCallback method
                stream.BeginRead(buffer, 0, newLength, new AsyncCallback(ReadCallback), message);
            }
            catch (ArgumentNullException e)
            {
                Console.WriteLine("ArgumentNullException: " + e);
                status = false;
            }
            catch (SocketException e)
            {
                Console.WriteLine("SocketException: " + e.ToString());
                status = false;
            }
            catch (Exception e)
            {
                Console.WriteLine("Unknown Exception: " + e.ToString());
                status = false;
            }

            return status;
        }

        /// <summary>
        /// Close connection to server
        /// </summary>
        public static void Close()
        {
            if (stream != null) stream.Close();
            if (client != null) client.Close();
        }

        /// <summary>
        /// Callback call by stream.BeginRead after reception of newLength data
        /// </summary>
        /// <param name="ar">Not sure of what is it, but needed for terminate reading</param>
        private static void ReadCallback(IAsyncResult ar)
        {
            if (client.Connected)
            {
                int bytesRead;

                try
                {
                    // Termintae read operation, and get number of byte stored in buffer
                    bytesRead = stream.EndRead(ar);
                }
                catch (ObjectDisposedException e)
                {
                    Console.WriteLine("Connection to server dropped: " + e.ToString());
                    return;
                }

                newLength = 1;

                // if number of byte read is not 0, concatenate string and buffer
                if (bytesRead > 0)
                {
                    packetCounter++;

                    if (packetCounter >= 3)
                    {
                        //Console.WriteLine("Supplementary packet " + packetCounter);
                    }

                    // Append new data to current string (expecting data are ascii)
                    message.Append(Encoding.ASCII.GetString(buffer, 0, bytesRead));

                    // Similarly, append received bytes to current buffer 
                    if (receiveBuffer == null) receiveBuffer = new byte[bytesRead];
                    else Array.Resize<byte>(ref receiveBuffer, initialReceiveBufferIndex + bytesRead); // resize currrent buffer

                    System.Buffer.BlockCopy(buffer, 0, receiveBuffer, initialReceiveBufferIndex, bytesRead); // and add received data
                    initialReceiveBufferIndex = receiveBuffer.Length; // move last index of current buffer
                }

                // if it remains received data, prepare for a new reading (get another buffer to append to current one)
                if (client.Available > 0)
                {
                    newLength = client.Available;
                    if (newLength > BufferMaxSize) newLength = BufferMaxSize;
                    else newLength = client.Available;
                }
                else
                {
                    // no more data to read, buffer and string can be send to upper level
                    readEvent?.Invoke(message.ToString(), receiveBuffer);

                    message.Clear();
                    receiveBuffer = null;
                    initialReceiveBufferIndex = 0;
                    packetCounter = 0;
                }

                // Prepare for reading new data
                if (newLength> BufferMaxSize) newLength = BufferMaxSize;
                if (newLength <= 0) newLength = 1;
                stream.BeginRead(buffer, 0, newLength, new AsyncCallback(ReadCallback), message);
            }
        }

        /// <summary>
        /// Write a string to server
        /// </summary>
        /// <returns>Nothing</returns>
        /// <param name="mes">Message to send to server</param>
        public static void Write(string mes)
        {
            if (client.Connected)
            {
                byte[] writeBuffer = Encoding.UTF8.GetBytes(mes);

                stream.Write(writeBuffer, 0, mes.Length);
            }
        }
    }
}
