using System;
using Gtk;
using Gdk;

using monitor;

/// <summary>
/// Main window.
/// </summary>
public partial class MainWindow : Gtk.Window
{
    private DestijlCommandManager cmdManager;
    private Pixbuf drawingareaCameraPixbuf;

    enum SystemState
    {
        NotConnected,
        ServerConnected,
        RobotConnected
    };

    private SystemState systemState = SystemState.NotConnected;
    private System.Timers.Timer batteryTimer;

    public MainWindow() : base(Gtk.WindowType.Toplevel)
    {
        Build();

        cmdManager = new DestijlCommandManager(OnCommandReceivedEvent);

        batteryTimer = new System.Timers.Timer(10000.0);
        batteryTimer.Elapsed += OnBatteryTimerElapsed;

        AdjustControls();
        PixbufFormat[] format = Gdk.Pixbuf.Formats;
        foreach (PixbufFormat f in format)
        {
            Console.WriteLine("Format: " + f.Name);
        }
    }

    public void AdjustControls()
    {
        ChangeState(SystemState.NotConnected);

        drawingareaCameraPixbuf = new Pixbuf((string)null);
        drawingareaCameraPixbuf = Pixbuf.LoadFromResource("monitor.ressources.missing_picture.png");

        entryServerName.Text = Client.defaultIP;
        entryServerPort.Text = Client.defaultPort.ToString();
        entryTimeout.Text = "10000";
    }

    private void ChangeState(SystemState newState)
    {
        switch (newState)
        {
            case SystemState.NotConnected:
                labelRobot.Sensitive = false;
                gtkAlignmentRobot.Sensitive = false;

                labelRobotControl.Sensitive = false;
                gtkAlignmentRobotControl.Sensitive = false;
                boxCamera.Sensitive = false;

                buttonServerConnection.Label = "Connect";
                buttonRobotActivation.Label = "Activate";
                labelBatteryLevel.Text = "Unknown";

                checkButtonCameraOn.Active = false;
                checkButtonRobotPosition.Active = false;
                if (cmdManager != null) cmdManager.Close();

                batteryTimer.Stop();
                break;
            case SystemState.ServerConnected:
                buttonServerConnection.Label = "Disconnect";
                buttonRobotActivation.Label = "Activate";
                labelBatteryLevel.Text = "Unknown";

                labelRobot.Sensitive = true;
                gtkAlignmentRobot.Sensitive = true;
                boxCamera.Sensitive = true;

                labelRobotControl.Sensitive = false;
                gtkAlignmentRobotControl.Sensitive = false;

                batteryTimer.Stop();
                break;
            case SystemState.RobotConnected:
                buttonRobotActivation.Label = "Reset";
                labelRobotControl.Sensitive = true;
                gtkAlignmentRobotControl.Sensitive = true;

                batteryTimer.Start();
                break;
            default:
                labelRobot.Sensitive = false;
                gtkAlignmentRobot.Sensitive = false;

                labelRobotControl.Sensitive = false;
                gtkAlignmentRobotControl.Sensitive = false;
                boxCamera.Sensitive = false;

                buttonServerConnection.Label = "Connect";
                buttonRobotActivation.Label = "Activate";
                labelBatteryLevel.Text = "Unknown";

                checkButtonCameraOn.Active = false;
                checkButtonRobotPosition.Active = false;

                systemState = SystemState.NotConnected;

                return;
        }

        systemState = newState;
    }

    private void MessagePopup(MessageType type, ButtonsType buttons, string title, string message)
    {
        MessageDialog md = new MessageDialog(this, DialogFlags.DestroyWithParent, type, buttons, message)
        {
            Title = title
        };

        md.Run();
        md.Destroy();
    }

    protected void OnDeleteEvent(object sender, DeleteEventArgs a)
    {
        Console.WriteLine("Bye bye");

        if (cmdManager != null) cmdManager.Close();
        Application.Quit();
        a.RetVal = true;
    }

    public void OnCommandReceivedEvent(string header, string data, byte[] buffer)
    {
        if (header != null)
        {
            Console.WriteLine("Received header (" + header.Length + "): " + header);
            if (header.ToUpper() != DestijlCommandList.HeaderStmImage)
            {
                if (data != null) Console.WriteLine("Received data (" + data.Length + "): " + data);
            }

            if (header.ToUpper() == DestijlCommandList.HeaderStmBat)
            {
                switch (data[0])
                {
                    case '2':
                        labelBatteryLevel.Text = "High";
                        break;
                    case '1':
                        labelBatteryLevel.Text = "Low";
                        break;
                    case '0':
                        labelBatteryLevel.Text = "Empty";
                        break;
                    default:
                        labelBatteryLevel.Text = "Invalid value";
                        break;
                }
            }
            else if (header.ToUpper() == DestijlCommandList.HeaderStmImage)
            {
                byte[] image = new byte[buffer.Length - 4];
                System.Buffer.BlockCopy(buffer, 4, image, 0, image.Length);

                drawingareaCameraPixbuf = new Pixbuf(image);
                drawingAreaCamera.QueueDraw();
            }
        }
    }

    protected void OnQuitActionActivated(object sender, EventArgs e)
    {
        Console.WriteLine("Bye bye 2");
        if (cmdManager != null) cmdManager.Close();
        this.Destroy();
        Application.Quit();
    }

    protected void OnShowLogWindowActionActivated(object sender, EventArgs e)
    {
        MessagePopup(MessageType.Info,
                     ButtonsType.Ok, "Info",
                     "Logger not yet implemented");
    }

    protected void OnButtonServerConnectionClicked(object sender, EventArgs e)
    {
        DestijlCommandManager.CommandStatus statusCmd;

        if (buttonServerConnection.Label == "Disconnect")
        {
            ChangeState(SystemState.NotConnected);
        }
        else
        {
            if ((entryServerName.Text == "") || (entryServerPort.Text == ""))
            {
                MessagePopup(MessageType.Error,
                             ButtonsType.Ok, "Error",
                             "Server name or port is invalid");
            }
            else
            {
                Console.WriteLine("Connecting to " + entryServerName.Text + ":" + entryServerPort.Text);
                bool status = false;

                try
                {
                    cmdManager.timeout = Convert.ToDouble(entryTimeout.Text);
                }
                catch (Exception)
                {
                    cmdManager.timeout = 100;
                    entryTimeout.Text = cmdManager.timeout.ToString();
                }

                try
                {
                    status = cmdManager.Open(entryServerName.Text, Convert.ToInt32(entryServerPort.Text));
                }
                catch (Exception)
                {
                    Console.WriteLine("Something went wrong during connection");
                    return;
                }

                if (status != true)
                {
                    MessagePopup(MessageType.Error,
                                 ButtonsType.Ok, "Error",
                                 "Unable to connect to server " + entryServerName.Text + ":" + Convert.ToInt32(entryServerPort.Text));
                }
                else
                {
                    Console.Write("Send command RobotOpenCom: ");
                    statusCmd = cmdManager.RobotOpenCom();
                    Console.WriteLine(statusCmd.ToString());

                    if (statusCmd == DestijlCommandManager.CommandStatus.Success)
                    {
                        ChangeState(SystemState.ServerConnected);
                    }
                    else
                    {
                        MessagePopup(MessageType.Error,
                                     ButtonsType.Ok, "Error",
                                     "Unable to open communication with robot.\nCheck that supervisor is accepting OPEN_COM_DMB command");

                        cmdManager.Close();
                    }
                }
            }
        }
    }

    protected void OnButtonRobotActivationClicked(object sender, EventArgs e)
    {
        DestijlCommandManager.CommandStatus status;

        if (buttonRobotActivation.Label == "Activate") // activation du robot
        {
            if (radioButtonWithWatchdog.Active) // Demarrage avec watchdog
            {
                status = cmdManager.RobotStartWithWatchdog();
            }
            else // Demarrage sans watchdog
            {
                status = cmdManager.RobotStartWithoutWatchdog();
            }

            if (status == DestijlCommandManager.CommandStatus.Success)
            {
                ChangeState(SystemState.RobotConnected);
            }
            else
            {
                if (status == DestijlCommandManager.CommandStatus.CommunicationLostWithServer)
                {
                    MessagePopup(MessageType.Error, ButtonsType.Ok, "Error", "Connection lost with server");
                    ChangeState(SystemState.NotConnected);
                }
                else
                {
                    MessagePopup(MessageType.Error, ButtonsType.Ok, "Error", "Command rejected\nCheck that supervisor accept \nDMB_START_WITH_WD and/or DMB_START_WITHOUT_WD");
                }
            }
        }
        else // Reset du robot
        {
            status = cmdManager.RobotReset();

            if (status == DestijlCommandManager.CommandStatus.Success)
            {
                ChangeState(SystemState.ServerConnected);
            }
            else
            {
                if (status == DestijlCommandManager.CommandStatus.CommunicationLostWithServer)
                {
                    MessagePopup(MessageType.Error, ButtonsType.Ok, "Error", "Connection lost with server");
                    ChangeState(SystemState.NotConnected);
                }
                else
                {
                    MessagePopup(MessageType.Error, ButtonsType.Ok, "Error", "Unknown error");
                }
            }
        }
    }

    protected void OnButtonMouvClicked(object sender, EventArgs e)
    {
        if (sender == buttonRight)
        {
            cmdManager.RobotTurn(90);
        }
        else if (sender == buttonLeft)
        {
            cmdManager.RobotTurn(-90);
        }
        else if (sender == buttonForward)
        {
            cmdManager.RobotMove(100);
        }
        else if (sender == buttonDown)
        {
            cmdManager.RobotMove(-100);
        }
        else
        {
            MessagePopup(MessageType.Warning, ButtonsType.Ok, "Abnormal behavior", "Callback OnButtonMouvClicked called by unknown sender");
        }
    }

    void OnBatteryTimerElapsed(object sender, System.Timers.ElapsedEventArgs e)
    {
        DestijlCommandManager.CommandStatus status;
        batteryTimer.Stop();

        if (checkButtonGetBattery.Active)
        {
            status = cmdManager.RobotGetBattery();
            switch (status)
            {
                case DestijlCommandManager.CommandStatus.Success:
                    batteryTimer.Start();
                    break;
                case DestijlCommandManager.CommandStatus.CommunicationLostWithServer:
                    Console.WriteLine("Error: Connection lost with server");
                    batteryTimer.Stop();
                    labelBatteryLevel.Text = "Unknown";

                    ChangeState(SystemState.NotConnected);
                    break;
                case DestijlCommandManager.CommandStatus.CommunicationLostWithRobot:
                    Console.WriteLine("Error: Connection lost with robot");
                    batteryTimer.Stop();
                    labelBatteryLevel.Text = "Unknown";

                    ChangeState(SystemState.ServerConnected);
                    break;
                default:
                    labelBatteryLevel.Text = "Unknown";
                    batteryTimer.Start();
                    break;
            }
        }
        else batteryTimer.Start();
    }

    protected void OnCheckButtonCameraOnClicked(object sender, EventArgs e)
    {
        if (!checkButtonCameraOn.Active)
        {
            if (cmdManager.CameraClose() != DestijlCommandManager.CommandStatus.Success)
            {
                MessagePopup(MessageType.Error,
                             ButtonsType.Ok, "Error",
                             "Error when closing camera: bad answer for supervisor or timeout");
            }
        }
        else
        {
            if (cmdManager.CameraOpen() != DestijlCommandManager.CommandStatus.Success)
            {
                MessagePopup(MessageType.Error,
                             ButtonsType.Ok, "Error",
                             "Error when opening camera: bad answer for supervisor or timeout");
                checkButtonCameraOn.Active = false;
            }
        }
    }

    protected void OnCheckButtonRobotPositionClicked(object sender, EventArgs e)
    {
        if (!checkButtonRobotPosition.Active)
        {
            if (cmdManager.CameraStopComputePosition() != DestijlCommandManager.CommandStatus.Success)
            {
                MessagePopup(MessageType.Error,
                             ButtonsType.Ok, "Error",
                             "Error when stopping position reception: bad answer for supervisor or timeout");
            }
        }
        else
        {
            if (cmdManager.CameraComputePosition() != DestijlCommandManager.CommandStatus.Success)
            {
                MessagePopup(MessageType.Error,
                             ButtonsType.Ok, "Error",
                             "Error when starting getting robot position: bad answer for supervisor or timeout");

                checkButtonRobotPosition.Active = false;
            }
        }
    }

    protected void OnDrawingAreaCameraExposeEvent(object o, ExposeEventArgs args)
    {
        //Console.WriteLine("Event expose. Args = " + args.ToString());

        DrawingArea area = (DrawingArea)o;
        Gdk.Pixbuf displayPixbuf;
        int areaWidth, areaHeight;

        Gdk.GC gc = area.Style.BackgroundGC(Gtk.StateType.Normal);

        area.GdkWindow.GetSize(out areaWidth, out areaHeight);
        int width = drawingareaCameraPixbuf.Width;
        int height = drawingareaCameraPixbuf.Height;
        float ratio = (float)width / (float)height;

        if (areaWidth <= width)
        {
            width = areaWidth;
            height = (int)(width / ratio);
        }

        if (width > areaWidth)
        {
            width = areaWidth;
        }

        if (height > areaHeight)
        {
            height = areaHeight;
        }

        displayPixbuf = drawingareaCameraPixbuf.ScaleSimple(width, height, InterpType.Bilinear);

        area.GdkWindow.DrawPixbuf(gc, displayPixbuf,
                                  0, 0,
                                  (areaWidth - displayPixbuf.Width) / 2,
                                  (areaHeight - displayPixbuf.Height) / 2,
                                  displayPixbuf.Width, displayPixbuf.Height,
                                  RgbDither.Normal, 0, 0);
    }

    protected void DetectArena()
    {
        DestijlCommandManager.CommandStatus status;
        MessageDialog md = new MessageDialog(this, DialogFlags.DestroyWithParent,
                                             MessageType.Question, ButtonsType.YesNo, "Arena is correct ?");
        {
            Title = "Check arena";
        };

        ResponseType result = (ResponseType)md.Run();
        md.Destroy();

        if (result == ResponseType.Yes)
        {
            status = cmdManager.CameraArenaConfirm();
        }
        else
        {
            status = cmdManager.CameraArenaInfirm();
        }

        if (status != DestijlCommandManager.CommandStatus.Success)
        {
            MessagePopup(MessageType.Error,
                            ButtonsType.Ok, "Error",
                            "Unable to send Confirm or Infirm arena command to supervisor");
        }
    }

    protected void OnButtonAskArenaClicked(object sender, EventArgs e)
    {
        if (cmdManager.CameraAskArena() != DestijlCommandManager.CommandStatus.Success)
        {
            MessagePopup(MessageType.Error,
                         ButtonsType.Ok, "Error",
                         "Error when asking for arena rendering");
            return;
        }

        DetectArena();
    }
}
