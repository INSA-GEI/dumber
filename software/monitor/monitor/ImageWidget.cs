//
//  ImageWidget.cs
//
//  Author:
//       Di MERCURIO Sébastien <dimercur@insa-toulouse.fr>
//
//  Copyright (c) 2019 INSA - DGEI
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

// 15/10/2019 dimercur
// Demande #43: Migrer le code lié à la gestion des images dans sa propre classe widget

using System;
using Gtk;
using Cairo;
using Gdk;

using System.Timers;

namespace monitor
{
    /// <summary>
    /// Class used for video display
    /// </summary>
    public class ImageWidget
    {
        /// <summary>
        /// Reference to GTK base widget
        /// </summary>
        private DrawingArea drawingArea;

        /// <summary>
        /// Pixbuffer used for displaying image
        /// </summary>
        private Pixbuf drawingareaCameraPixbuf;

        /// <summary>
        /// Indicate if position must be displayed or not
        /// </summary>
        public bool showPosition;

        /// <summary>
        /// Indicate if FPS must be displayed or not
        /// </summary>
        private bool showFPS;
        public bool ShowFPS
        {
            get
            {
                return showFPS;
            }

            set
            {
                showFPS = value;

                if (value == true)
                {
                    imageFPS = 0;
                    imageFPScounter = 0;

                    fpsTimer.Start();
                }
                else
                {
                    fpsTimer.Stop();

                    Refresh();
                }
            }
        }

        /// <summary>
        /// Counter used for FPS computation
        /// </summary>
        private int imageFPS = 0;
        private int imageFPScounter = 0;

        /// <summary>
        /// Timer for FPS request
        /// </summary>
        private System.Timers.Timer fpsTimer;

        /// <summary>
        /// Hold position to be displayed
        /// </summary>
        private DestijlCommandManager.Position position;
        public DestijlCommandManager.Position Position
        {
            get
            {
                return position;
            }

            set 
            {
                position = value; 
                Refresh();
            }
        }

        /// <summary>
        /// Request a refresh of drawing area
        /// </summary>
        private void Refresh() 
        {
            Gtk.Application.Invoke(delegate
            {
                drawingArea.QueueDraw();
            });
        }

        /// <summary>
        /// Initializes a new instance of ImageWidget class.
        /// </summary>
        /// <param name="area">Reference to GTK drawing area widget</param>
        public ImageWidget(DrawingArea area)
        {
            this.drawingArea = area;
            // create new timer for FPS , every 1s
            fpsTimer = new System.Timers.Timer(1000.0);
            fpsTimer.Elapsed += OnFpsTimerElapsed;

            showPosition = false;
            showFPS = false;

            drawingArea.ExposeEvent += OnDrawingAreaCameraExposeEvent;
        }

        /// <summary>
        /// Show an image from a ressource
        /// </summary>
        /// <param name="ressource">Ressource path</param>
        public void ShowImage(string ressource) 
        {
            drawingareaCameraPixbuf = Pixbuf.LoadFromResource("monitor.ressources.missing_picture.png");
            imageFPScounter++;

            Refresh();
        }

        /// <summary>
        /// Show an image from a byte array
        /// </summary>
        /// <param name="image">Byte array containing a valid image</param>
        public void ShowImage(byte[] image)
        {
            drawingareaCameraPixbuf = new Pixbuf(image);
            imageFPScounter++;

            Refresh();
        }

        /// <summary>
        /// Callback called when drawingarea need refresh
        /// </summary>
        /// <param name="o">Sender object</param>
        /// <param name="args">Expose arguments</param>
        protected void OnDrawingAreaCameraExposeEvent(object o, ExposeEventArgs args)
        {
            //Console.WriteLine("Event expose. Args = " + args.ToString());

            DrawingArea area = (DrawingArea)o;
            Gdk.Pixbuf displayPixbuf;
            int areaWidth, areaHeight;

            // Get graphic context for background
            Gdk.GC gc = area.Style.BackgroundGC(Gtk.StateType.Normal);

            // get size of drawingarea widget
            area.GdkWindow.GetSize(out areaWidth, out areaHeight);
            int width = drawingareaCameraPixbuf.Width;
            int height = drawingareaCameraPixbuf.Height;
            float ratio = (float)width / (float)height;

            // if widget is smaller than image, reduce it
            if (areaWidth <= width)
            {
                width = areaWidth;
                height = (int)(width / ratio);
            }

            // if image is smaller than widget, enlarge it
            if (width > areaWidth)
            {
                width = areaWidth;
            }

            if (height > areaHeight)
            {
                height = areaHeight;
            }

            //scale original picture and copy result in local pixbuf
            displayPixbuf = drawingareaCameraPixbuf.ScaleSimple(width, height, InterpType.Bilinear);

            // draw local pixbuff centered on drawingarea
            area.GdkWindow.DrawPixbuf(gc, displayPixbuf,
                                      0, 0,
                                      (areaWidth - displayPixbuf.Width) / 2,
                                      (areaHeight - displayPixbuf.Height) / 2,
                                      displayPixbuf.Width, displayPixbuf.Height,
                                      RgbDither.Normal, 0, 0);

            if (showPosition)
            {
                Cairo.Context cr = Gdk.CairoHelper.Create(area.GdkWindow);
                Cairo.Color textFontColor = new Cairo.Color(0.8, 0, 0);

                cr.SelectFontFace("Cantarell", FontSlant.Normal, FontWeight.Bold);
                cr.SetSourceColor(textFontColor);
                cr.SetFontSize(16);

                if (position != null)
                {
                    double space = 0.0;

                    string text = "Direction (" + position.direction.x.ToString("0.##") + " ; " + position.direction.y.ToString("0.##") + ")";
                    TextExtents te = cr.TextExtents(text);
                    cr.MoveTo(areaWidth - te.Width - 5,
                              areaHeight - te.Height - 5);
                    space = te.Height;
                    cr.ShowText(text);

                    text = "Centre (" + position.centre.x.ToString("0.##") + " ; " + position.centre.y.ToString("0.##") + ")";
                    te = cr.TextExtents(text);
                    cr.MoveTo(areaWidth - te.Width - 5,
                              areaHeight - te.Height - 5 - space - 5);
                    space = space + te.Height + 5;
                    cr.ShowText(text);

                    text = "Angle: " + position.angle.ToString("0.##");
                    te = cr.TextExtents(text);
                    cr.MoveTo(areaWidth - te.Width - 5,
                              areaHeight - te.Height - 5 - space - 5);
                    space = space + te.Height + 5;
                    cr.ShowText(text);

                    text = "ID: " + position.robotID;
                    te = cr.TextExtents(text);
                    cr.MoveTo(areaWidth - te.Width - 5,
                              areaHeight - te.Height - 5 - space - 5);

                    cr.ShowText(text);
                }
                else
                {
                    string text = "Position (NULL)";
                    TextExtents te = cr.TextExtents(text);
                    cr.MoveTo(areaWidth - te.Width - 5,
                              areaHeight - te.Height - 5);
                    
                    cr.ShowText(text);
                }

                ((IDisposable)cr.GetTarget()).Dispose();
                ((IDisposable)cr).Dispose();
            }

            if (showFPS)
            {
                Cairo.Context cr = Gdk.CairoHelper.Create(area.GdkWindow);
                Cairo.Color textFontColor = new Cairo.Color(0.8, 0, 0);

                cr.SelectFontFace("Cantarell", FontSlant.Normal, FontWeight.Bold);
                cr.SetSourceColor(textFontColor);
                cr.SetFontSize(16);

                string text = "FPS= " + imageFPS.ToString();
                TextExtents te = cr.TextExtents(text);
                cr.MoveTo(10, 10 + te.Height);
                cr.ShowText(text);

                ((IDisposable)cr.GetTarget()).Dispose();
                ((IDisposable)cr).Dispose();
            }
        }
    
        /// <summary>
        /// Timer used for FPS computation
        /// </summary>
        /// <param name="sender">Sender object</param>
        /// <param name="e">unused parameter</param>
        private void OnFpsTimerElapsed(object sender, ElapsedEventArgs e)
        {
            imageFPS = imageFPScounter;
            imageFPScounter = 0;

            Refresh();
        }
    }
}
