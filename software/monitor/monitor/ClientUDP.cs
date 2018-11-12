using System;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace monitor
{
    public static class ClientUDP
    {
        private const int listenPort = 11000;

        private static UdpClient clientUDP = null;
        private static IPEndPoint ep = null;

        public static void UDPOpen(string addr, int port)
        {
            clientUDP = new UdpClient(port);
            ep = new IPEndPoint(IPAddress.Parse(addr), port);
            clientUDP.Connect(ep);

            SendPing();
        }

        public static void UDPClose()
        {
            if (clientUDP!=null)
                clientUDP.Close();
        }

        public static byte[] GetData()
        {
            Console.WriteLine("Waiting for broadcast");
            byte[] bytes = clientUDP.Receive(ref ep);

            Console.WriteLine("Received broadcast from {0} :\n {1}\n",
                ep.ToString(),
                Encoding.ASCII.GetString(bytes, 0, bytes.Length));
            
            return bytes;
        }

        public static bool SendPing()
        {
            byte[] msg = new byte[2];
            msg[0] = (byte)'O';
            msg[1] = (byte)'k';

            Console.WriteLine("Ping Server to send address");
            clientUDP.Send(msg, msg.Length);

            return true;
        }
    }
}
