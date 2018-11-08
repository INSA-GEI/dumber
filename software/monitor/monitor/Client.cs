using System;
using System.Net.Sockets;
using System.Threading;
using System.Text;

namespace monitor
{
    public class Client
    {
        public const string defaultIP = "localhost";
        public const int defaultPort = 4500;

        private static TcpClient client = null;
        private static NetworkStream stream = null;

        private const int BufferMaxSize = 512;
        private static byte[] buffer = new byte[BufferMaxSize];
        private static byte[] receiveBuffer;
        private static int initialReceiveBufferIndex = 0;
        private static StringBuilder message = new StringBuilder();
        private static int newLength = 1;
        private static int packetCounter = 0;

        public delegate void ReadEvent(string msg, byte[] buffer);
        public static ReadEvent readEvent = null;

        public Client()
        {
        }

        public static bool Open(string host)
        {
            return Client.Open(host, defaultPort);
        }

        public static bool Open(string host, int port)
        {
            bool status = true;

            try
            {
                client = new TcpClient(host, port);

                stream = client.GetStream();

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

        public static void Close() 
        {
            if (stream!=null) stream.Close();
            if (client!=null) client.Close();
        }

        private static void ReadCallback(IAsyncResult ar)
        {
            if (client.Connected)
            {
                int bytesRead;

                try 
                {
                    bytesRead = stream.EndRead(ar); 
                }
                catch (ObjectDisposedException e)
                {
                    Console.WriteLine("Connection to server dropped: " + e.ToString());
                    return;
                }

                newLength = 1;

                if (bytesRead > 0)
                {
                    packetCounter++;

                    if (packetCounter>=3)
                    {
                        //Console.WriteLine("Supplementary packet " + packetCounter);
                    }

                    message.Append(Encoding.ASCII.GetString(buffer, 0, bytesRead));
                    if (receiveBuffer == null) receiveBuffer = new byte[bytesRead];
                    else Array.Resize<byte>(ref receiveBuffer, initialReceiveBufferIndex + bytesRead);

                    System.Buffer.BlockCopy(buffer, 0, receiveBuffer, initialReceiveBufferIndex, bytesRead);
                    initialReceiveBufferIndex = receiveBuffer.Length;
                }

                if (client.Available > 0)
                {
                    newLength = client.Available;
                    if (newLength > BufferMaxSize) newLength = BufferMaxSize;
                    else newLength = client.Available;
                }
                else
                {
                    readEvent?.Invoke(message.ToString(), receiveBuffer);

                    message.Clear();
                    receiveBuffer = null;
                    initialReceiveBufferIndex = 0;
                    packetCounter = 0;
                }

                stream.BeginRead(buffer, 0, newLength, new AsyncCallback(ReadCallback), message);
            }
        }

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
