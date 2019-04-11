//
//  MonitorUI.cs
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

// 15/10/2019 dimercur
// Demande #43: Migrer le code lié à la gestion des images dans sa propre classe widget

// 11/04/2019 dimercur
// Suppression du timer battery
// suppression de la case à cocher getbattery
// Prise en charge des messages ANSWER_TIMEOUT et ANSWER_COM_ERROR dans OnCommandReceivedEvent

using System;
using Gtk;
using Gdk;
using Cairo;

using monitor;
using System.Timers;

/// <summary>
/// Main part of the program, behavior of main window
/// </summary>
public partial class MainWindow : Gtk.Window
{
    /// <summary>
    /// Destijl command manager reference
    /// </summary>
    private DestijlCommandManager cmdManager;

    /// <summary>
    /// Position used for displaying position
    /// </summary>
    private DestijlCommandManager.Position position=new DestijlCommandManager.Position();

    /// <summary>
    /// List of availble state for the application
    /// </summary>
    enum SystemState
    {
        NotConnected,
        ServerConnected,
        RobotConnected
    };

    /// <summary>
    /// The state of the system. Can take a value from SystemState
    /// </summary>
    private SystemState systemState = SystemState.NotConnected;

    /// <summary>
    /// Object used for displaying image on a valid DrawingArea widget
    /// </summary>
    private ImageWidget imageWidget;

    /// <summary>
    /// Counter for image reception and detecting bad picture ratio
    /// </summary>
    private int imageReceivedCounter = 0;
    private int badImageReceivedCounter = 0;

    /// <summary>
    /// Initializes a new instance of the <see cref="MainWindow"/> class.
    /// </summary>
    public MainWindow() : base(Gtk.WindowType.Toplevel)
    {
        Build();

        cmdManager = new DestijlCommandManager(OnCommandReceivedEvent);

        // Init of image widget
        imageWidget = new ImageWidget(drawingAreaCamera);

        // Customize controls
        AdjustControls();
    }

    /// <summary>
    /// Make some adjustement to controls, like disabling some controls
    /// </summary>
    public void AdjustControls()
    {
        // Change state of system, and grey every controls not needed
        ChangeState(SystemState.NotConnected);

        // Load "no picture" image from disque
        imageWidget.ShowImage("monitor.ressources.missing_picture.png");

        // setup server controls
        entryServerName.Text = Client.defaultIP;
        entryServerPort.Text = Client.defaultPort.ToString();
        entryTimeout.Text = "1000";
    }

    /// <summary>
    /// Method used to change controls visibility (greyed or not) depending on current state
    /// </summary>
    /// <param name="newState">New state</param>
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
                checkButtonFPS.Active = false;

                imageWidget.ShowFPS = false;
                imageWidget.showPosition = false;

                if (cmdManager != null) cmdManager.Close();

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

                break;
            case SystemState.RobotConnected:
                buttonRobotActivation.Label = "Reset";
                labelRobotControl.Sensitive = true;
                gtkAlignmentRobotControl.Sensitive = true;

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
                checkButtonFPS.Active = false;

                imageWidget.ShowFPS = false;
                imageWidget.showPosition = false;

                systemState = SystemState.NotConnected;

                return;
        }

        systemState = newState;
    }

    /// <summary>
    /// Display a popup message window
    /// </summary>
    /// <param name="type">Type of popup window (question, error, information,...)</param>
    /// <param name="buttons">Buttons available on popup window</param>
    /// <param name="title">Title of window</param>
    /// <param name="message">Message</param>
    private void MessagePopup(MessageType type, ButtonsType buttons, string title, string message)
    {
        MessageDialog md = new MessageDialog(this, DialogFlags.DestroyWithParent, type, buttons, message);

        md.Title = title;
        md.Run();
        md.Destroy();
    }

    /// <summary>
    /// Callback called when delete event is sent by window
    /// </summary>
    /// <param name="sender">Sender object</param>
    /// <param name="a">Not really sure of what it is...</param>
    protected void OnDeleteEvent(object sender, DeleteEventArgs a)
    {
        Console.WriteLine("Bye bye");

        if (cmdManager != null) cmdManager.Close();
        Application.Quit();
        a.RetVal = true;
    }

    /// <summary>
    /// Callback called when new message is received from server
    /// </summary>
    /// <param name="header">Header of message</param>
    /// <param name="data">Data of message</param>
    public void OnCommandReceivedEvent(string header, string data)
    {
        if (header == null)
        {
            // we have lost server
            ChangeState(SystemState.NotConnected);

            Gtk.Application.Invoke(delegate
            {
                MessagePopup(MessageType.Error,
                     ButtonsType.Ok, "Server lost",
                         "Server is down: disconnecting");
                cmdManager.Close();
            });
        }

        // if we have received a valid message
        if (header != null)
        {
#if DEBUG
            // print message content
            if (header.Length > 4)
            {
                Console.WriteLine("Bad header(" + header.Length + ")");
            }
#endif
            // Depending on message received (based on header), launch correponding action
            header = header.ToUpper();

            switch (header)
            {
                case DestijlCommandList.ANSWER_TIMEOUT:
                case DestijlCommandList.ANSWER_COM_ERROR:
                    Console.WriteLine("Communication lost with robot");
                    Gtk.Application.Invoke(delegate
                    {
                        MessagePopup(MessageType.Error, ButtonsType.Ok, "Robot lost", "Communication with robot lost !");
                    });

                    ChangeState(SystemState.ServerConnected);

                    break;
                case DestijlCommandList.ROBOT_BATTERY_LEVEL:
                    string batLevel = "";

                    switch (data[0])
                    {
                        case '2':
                            batLevel = "High";
                            break;
                        case '1':
                            batLevel = "Low";
                            break;
                        case '0':
                            batLevel = "Empty";
                            break;
                        default:
                            batLevel = "Invalid value";
                            break;
                    }

                    Gtk.Application.Invoke(delegate
                    {
                        labelBatteryLevel.Text = batLevel;
                    });

                    break;
                case DestijlCommandList.CAMERA_IMAGE:
                    imageReceivedCounter++;

                    byte[] image = new byte[2];
                    try
                    {
                        image = Convert.FromBase64String(data);
                    }
                    catch (FormatException)
                    {
                        badImageReceivedCounter++;
                        Console.WriteLine("Unable to convert from base64 ");
                    }

                    try
                    {
                        imageWidget.ShowImage(image);
                    }
                    catch (GLib.GException)
                    {
                        badImageReceivedCounter++;
#if DEBUG
                        Console.WriteLine("Bad Image: " + badImageReceivedCounter +
                                          " / " + imageReceivedCounter +
                                          " (" + badImageReceivedCounter * 100 / imageReceivedCounter + "%)");
#endif
                    }

                    break;
                case DestijlCommandList.CAMERA_POSITION:
                    imageWidget.Position = DestijlCommandManager.DecodePosition(data);

                    break;
                default:
                    Console.WriteLine("Untreated message from supervisor: " + header + ": " + data);

                    break;
            }
        }
    }

    /// <summary>
    /// Callback called by "quit" menu
    /// </summary>
    /// <param name="sender">Sender object</param>
    /// <param name="e">Event</param>
    protected void OnQuitActionActivated(object sender, EventArgs e)
    {
        Console.WriteLine("Bye bye 2");
        if (cmdManager != null) cmdManager.Close();
        this.Destroy();

        Application.Quit();
    }

    /// <summary>
    /// Callback called by "show log" menu
    /// </summary>
    /// <param name="sender">Sender object</param>
    /// <param name="e">Event</param>
    protected void OnShowLogWindowActionActivated(object sender, EventArgs e)
    {
        MessagePopup(MessageType.Info,
                     ButtonsType.Ok, "Info",
                     "Logger not yet implemented");
    }

    /// <summary>
    /// Callback called by "buttonServerConnection" button
    /// </summary>
    /// <param name="sender">Sender object</param>
    /// <param name="e">Event</param>
    protected void OnButtonServerConnectionClicked(object sender, EventArgs e)
    {
        DestijlCommandManager.CommandStatus statusCmd;

        // if we are currently connected
        if (buttonServerConnection.Label == "Disconnect")
        {
            // Change state to disconnect and close connection
            ChangeState(SystemState.NotConnected);
        }
        else // we are not currently connected to server
        {
            // if information about hostname or port are invalid, show a popup error
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

                // try to convert timout string value to double. If that failed, default to 100 ms
                try
                {
                    cmdManager.timeout = Convert.ToDouble(entryTimeout.Text);
                }
                catch (Exception)
                {
                    cmdManager.timeout = 100;
                    entryTimeout.Text = cmdManager.timeout.ToString();
                }

                // try to connect to givn server. 
                try
                {
                    status = cmdManager.Open(entryServerName.Text, Convert.ToInt32(entryServerPort.Text));
                }
                catch (Exception)
                {
                    Console.WriteLine("Something went wrong during connection");
                    return;
                }

                //if connection status is not ok, show an error popup
                if (status != true)
                {
                    MessagePopup(MessageType.Error,
                                 ButtonsType.Ok, "Error",
                                 "Unable to connect to server " + entryServerName.Text + ":" + Convert.ToInt32(entryServerPort.Text));
                }
                else // if we succed in connecting, open communication with robot
                {
                    Console.Write("Send command RobotOpenCom: ");
                    statusCmd = cmdManager.RobotOpenCom();
                    Console.WriteLine(statusCmd.ToString());

                    if (statusCmd == DestijlCommandManager.CommandStatus.Success)
                    {
                        ChangeState(SystemState.ServerConnected);
                    }
                    else // if communication with robot is not possible, show error
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

    /// <summary>
    /// Callback called when "buttonRobotactivation" is clicked
    /// </summary>
    /// <param name="sender">Sender object</param>
    /// <param name="e">Event</param>
    protected void OnButtonRobotActivationClicked(object sender, EventArgs e)
    {
        DestijlCommandManager.CommandStatus status;

        //if robot is not activated
        if (buttonRobotActivation.Label == "Activate")
        {
            // if a startup with watchdog is requested
            if (radioButtonWithWatchdog.Active)
            {
                status = cmdManager.RobotStartWithWatchdog();
            }
            else // startup without watchdog
            {
                status = cmdManager.RobotStartWithoutWatchdog();
            }

            // if status of command is ok, change state of system, enabling robot control
            if (status == DestijlCommandManager.CommandStatus.Success)
            {
                ChangeState(SystemState.RobotConnected);
            }
            else // if status is not ok, depending of error, show appropriate error
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
        else // If robot is already activated, request reset of robot
        {
            status = cmdManager.RobotReset();

            // if status of command is ok, change state of system, disabling robot control
            if (status == DestijlCommandManager.CommandStatus.Success)
            {
                ChangeState(SystemState.ServerConnected);
            }
            else // if status is not ok, depending of error, show appropriate error
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

    /// <summary>
    /// Callback called when user click on direction button
    /// </summary>
    /// <param name="sender">Sender button</param>
    /// <param name="e">Event</param>
    protected void OnButtonMouvClicked(object sender, EventArgs e)
    {
        // depending on button clicked, launch appropriate action
        if (sender == buttonRight)
        {
            cmdManager.RobotGoRight();
        }
        else if (sender == buttonLeft)
        {
            cmdManager.RobotGoLeft();
        }
        else if (sender == buttonForward)
        {
            cmdManager.RobotGoForward();
        }
        else if (sender == buttonDown)
        {
            cmdManager.RobotGoBackward();
        }
        else if (sender == buttonStop)
        {
            cmdManager.RobotStop();
        }
        else
        {
            MessagePopup(MessageType.Warning, ButtonsType.Ok, "Abnormal behavior", "Callback OnButtonMouvClicked called by unknown sender");
        }
    }

    /// <summary>
    /// Callback called when checkbutton for camera is clicked
    /// </summary>
    /// <param name="sender">Sender object</param>
    /// <param name="e">Event</param>
    protected void OnCheckButtonCameraOnClicked(object sender, EventArgs e)
    {
        // if camera is already active, switch it off
        if (!checkButtonCameraOn.Active)
        {
            if (cmdManager.CameraClose() != DestijlCommandManager.CommandStatus.Success)
            {
                Console.WriteLine("Error when closing camera: bad answer for supervisor or timeout");
            }
        }
        else // camera is not active, switch it on
        {
            badImageReceivedCounter = 0;
            imageReceivedCounter = 0;

            if (cmdManager.CameraOpen() != DestijlCommandManager.CommandStatus.Success)
            {
                Console.WriteLine("Error when opening camera: bad answer for supervisor or timeout");
                //checkButtonCameraOn.Active = false;
            }
        }
    }

    /// <summary>
    /// Callback called when checkbutton robot position is clicked
    /// </summary>
    /// <param name="sender">Sender object</param>
    /// <param name="e">Event</param>
    protected void OnCheckButtonRobotPositionClicked(object sender, EventArgs e)
    {
        // if server already send robot position, stop it
        if (!checkButtonRobotPosition.Active)
        {
            if (cmdManager.CameraStopComputePosition() != DestijlCommandManager.CommandStatus.Success)
            {
                Console.WriteLine("Error when stopping position reception: bad answer for supervisor or timeout");
            }
        }
        else // start reception of robot position
        {
            if (cmdManager.CameraComputePosition() != DestijlCommandManager.CommandStatus.Success)
            {
                Console.WriteLine("Error when starting getting robot position: bad answer for supervisor or timeout");

                //checkButtonRobotPosition.Active = false;
            }
        }

        imageWidget.showPosition = checkButtonRobotPosition.Active;
    }

    /// <summary>
    /// Show a popup asking user to tell if arena is correct or not 
    /// </summary>
    protected void DetectArena()
    {
        DestijlCommandManager.CommandStatus status;
        MessageDialog md = new MessageDialog(this, DialogFlags.DestroyWithParent,
                                             MessageType.Question, ButtonsType.YesNo, "Arena is correct ?");

        md.Title = "Check arena";

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

    /// <summary>
    /// Callback called when "Detect Arena" button is clicked
    /// </summary>
    /// <param name="sender">Sender object</param>
    /// <param name="e">Event</param>
    protected void OnButtonAskArenaClicked(object sender, EventArgs e)
    {
        // Send command to server for arean rendering
        if (cmdManager.CameraAskArena() != DestijlCommandManager.CommandStatus.Success)
        {
            MessagePopup(MessageType.Error,
                         ButtonsType.Ok, "Error",
                         "Error when asking for arena rendering");
            return;
        }

        // show popup and wait for user to say if arena is ok or not 
        DetectArena();
    }

    /// <summary>
    /// Callback function for FPS checkbutton
    /// </summary>
    /// <param name="sender">Sender object</param>
    /// <param name="e">unused paramter</param>
    protected void OnCheckButtonFPSToggled(object sender, EventArgs e)
    {
        imageWidget.ShowFPS = checkButtonFPS.Active;
    }
}
