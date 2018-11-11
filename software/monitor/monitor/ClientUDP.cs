using System;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace monitor
{
    public static class ClientUDP
    {
        private const int listenPort = 11000;
        private static byte[] ImageBuffer = null;

        private static UdpClient listener = null;
        private static IPEndPoint groupEP = null;

        public static void UDPOpen(int port)
        {
            listener = new UdpClient(port);
            groupEP = new IPEndPoint(IPAddress.Any, port);
        }

        public static void UDPClose()
        {
            listener.Close();
        }

        public static byte[] GetImage()
        {
            bool done = false;

            try
            {
                while (!done)
                {
                    Console.WriteLine("Waiting for broadcast");
                    byte[] bytes = listener.Receive(ref groupEP);

                    Console.WriteLine("Received broadcast from {0} :\n {1}\n",
                        groupEP.ToString(),
                        Encoding.ASCII.GetString(bytes, 0, bytes.Length));

                    if (bytes[0]=='I') {
                        // Nouvelle trame recu
                        ImageBuffer = bytes;
                    }
                    else if (bytes[bytes.Length-1]=='D')
                    {
                        Array.Resize<byte>(ref ImageBuffer, ImageBuffer.Length + bytes.Length); // resize currrent buffer

                        System.Buffer.BlockCopy(ImageBuffer, 0, bytes, ImageBuffer.Length-bytes.Length, bytes.Length);
                        done = true;
                    }
                    else{
                        Array.Resize<byte>(ref ImageBuffer, ImageBuffer.Length + bytes.Length); // resize currrent buffer

                        System.Buffer.BlockCopy(ImageBuffer, 0, bytes, ImageBuffer.Length-bytes.Length, bytes.Length);
                    }
                }

            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
     
            return ImageBuffer;
        }
    }
}
