using System.Threading;

namespace monitor
{
    public class CommandManager
    {
        public delegate void CommandReceivedEvent(string msg);
        public CommandReceivedEvent commandReceivedEvent = null;

        private System.Timers.Timer waitTimer = new System.Timers.Timer();
        private ManualResetEvent waitEvent = new ManualResetEvent(false);

        private bool waitForAcknowledge = false;

        private string messageReceived = null;
        private bool isBusy = false;

        public enum CommandManagerStatus
        {
            AnswerReceived,
            Timeout,
            Busy
        };

        public CommandManager(CommandReceivedEvent callback)
        {
            Client.readEvent += this.OnMessageReception;

            this.commandReceivedEvent += callback;
            waitTimer.Elapsed += OnMessageTimeout;
        }

        ~CommandManager()
        {
            Client.Close();    
        }

        public bool Open(string hostname)
        {
            return this.Open(hostname, Client.defaultPort);
        }

        public bool Open(string hostname, int port)
        {
            return Client.Open(hostname, port);
        }

        public void Close()
        {
            Client.Close();
        }

        private void OnMessageReception(string message)
        {
            waitTimer.Stop();
            this.messageReceived = message;
            isBusy = false;

            if (waitForAcknowledge) {
                waitForAcknowledge = false;
                waitEvent.Set(); // Envoi de l'evenement
            }
            else {
                waitForAcknowledge = false;
                this.commandReceivedEvent?.Invoke(message);
            }
        }

        private void OnMessageTimeout(object sender, System.Timers.ElapsedEventArgs e)
        {
            messageReceived = null;
            OnMessageReception(messageReceived);
        }

        public CommandManagerStatus SendCommand(string cmd, out string answer, double timeout)
        {
            CommandManagerStatus status = CommandManagerStatus.AnswerReceived;
            answer = null;

            if (isBusy) status = CommandManagerStatus.Busy;
            else
            {
                isBusy = true;

                Client.Write(cmd);

                if (timeout > 0) // la commande attend un acquitement
                {
                    waitForAcknowledge = true;
                    waitTimer.Interval = timeout;
                    waitTimer.Start();

                    waitEvent.WaitOne();
                    waitEvent.Reset(); // remise à zero pour une prochaine commande

                    if (this.messageReceived == null) // timeout: connection au serveur defectueuse
                    {
                        status = CommandManagerStatus.Timeout;
                    }
                }
                else isBusy = false;

                answer = this.messageReceived;
                this.messageReceived = null;
            }

            return status;
        }
    }
}
