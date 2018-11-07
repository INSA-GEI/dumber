using System;
using System.Net.Sockets;
using System.Threading;
using System.Text;

namespace monitor
{
    public class ClientReadEvent
    {
        private static TcpClient myClient = null;
        private static NetworkStream myStream = null;
        private const int BufferMaxSize = 512;
        private static byte[] buffer = new byte[BufferMaxSize];
        private static StringBuilder sb = new StringBuilder();
        private static int newLength = 1;

        public delegate void ReadEvent(string str);
        public static ReadEvent readEvent = null;

        public static void Set(TcpClient client, NetworkStream stream)
        {
            myClient = client;
            myStream = stream;
        }

        public static void ReadThread()
        {
            while (true)
            {
                if (myClient.Connected)
                {
                    myStream.BeginRead(buffer, 0, newLength, new AsyncCallback(ReadCallback), sb);
                }
                else Thread.Sleep(200);
            }
        }

        public static void ReadCallback(IAsyncResult ar)
        {

        }
    }

    public class Client
    {
        public const string defaultIP = "localhost";
        public const int defaultPort = 4500;

        private static TcpClient client = null;
        private static NetworkStream stream = null;

        private const int BufferMaxSize = 512;
        private static byte[] buffer = new byte[BufferMaxSize];
        private static StringBuilder message = new StringBuilder();
        private static int newLength = 1;

        public delegate void ReadEvent(string msg);
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
                    message.Append(Encoding.ASCII.GetString(buffer, 0, bytesRead));
                }

                if (client.Available > 0)
                {
                    newLength = client.Available;
                    if (newLength > BufferMaxSize) newLength = BufferMaxSize;
                    else newLength = client.Available;
                }
                else
                {
                    readEvent?.Invoke(message.ToString());

                    message.Clear();
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
