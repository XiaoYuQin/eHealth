﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

using System.IO;
using System.IO.Ports;

using System.Threading;
using System.Net;
using System.Net.Sockets;


namespace eHealth
{
    public partial class Form1 : Form
    {
        SerialPort com;
        bool isConnect;
       // delegate void HandleInterfaceUpdateDelegate(string text);  //委托，此为重点
       // HandleInterfaceUpdateDelegate interfaceUpdateHandle;
        Thread _readThread;
        bool _keepReading;

        String value = "0";
        String longitude = "0";
        String latitudeX = "0";

        bool isLocated = false;

        int mapflag = 0;

        private System.Timers.Timer timer = new System.Timers.Timer();

        public Form1()
        {
            InitializeComponent();
            Control.CheckForIllegalCrossThreadCalls = false;

            com = new SerialPort();
            isConnect = false;
            
            //AT+ST=1
            //AT+BT=123
            //AT+LO=1234
            //AT+LA=123

            timer.Elapsed += new System.Timers.ElapsedEventHandler(timer_Elapsed);
            timer.Enabled = true;

 
            Console.WriteLine("serial port");

            String[] str = SerialPort.GetPortNames();
            if (str == null)
            {
                MessageBox.Show("no port ", "error");
                return;
            }
            foreach (String s in SerialPort.GetPortNames())
            {
                System.Console.WriteLine(s);
                serialBox.Items.Add(s);
            }
            serialBox.SelectedIndex = 0;
            button1.Text = "closed";
           // interfaceUpdateHandle = new HandleInterfaceUpdateDelegate(UpdateTextBox);  //实例化委托对象
            //this.com.DataReceived += new System.IO.Ports.SerialDataReceivedEventHandler(this.OnDataRecivice);
            com.ReceivedBytesThreshold = 1;

    


        }
        private void OnDataRecivice(object sender ,SerialDataReceivedEventArgs e) {
            byte[] readBuffer = new byte[com.ReadBufferSize];
            com.Read(readBuffer, 0, readBuffer.Length);
            //this.Invoke(interfaceUpdateHandle, new string[] { Encoding.Unicode.GetString(readBuffer) });
        }
        private void button1_Click(object sender, EventArgs e)
        {
            if (isConnect == false)
            {
                try
                {
                    com.BaudRate = 9600;
                    Console.WriteLine(serialBox.Text);
                    com.PortName = serialBox.Text;
                    com.DataBits = 8;
                    com.Open();
                    isConnect = true;
                    _keepReading = true;
                    _readThread = new Thread(ReadPort);
                    _readThread.Start();
                    button1.Text = "opened";
                }
                catch (System.IO.IOException) {
                    button1.Text = "closed";
                    isConnect = false;
                    MessageBox.Show("SerialPort Open FAIL", "ERROR");                
                }
                catch (System.NullReferenceException){
                    button1.Text = "closed";
                    isConnect = false;
                    MessageBox.Show("Open SerialPort", "SerialPort FAIL");
                }
            }
            else { 
                
            }
        }

        /*private void button2_Click(object sender, EventArgs e)
        {
            com.WriteLine("1234");
            Console.WriteLine("1234");
        }*/
        private void UpdateTextBox(string text)
        {
            Console.WriteLine(text);
            
        }
        private void ReadPort() {
            while (_keepReading) {

                if (isConnect == true){

                    //byte[] readBuffer = new byte[com.ReadBufferSize + 1];
                    try
                    {

                        String SerialIn = com.ReadLine();
                        Console.WriteLine(SerialIn);

                        String sssss = SerialIn;
                        textBox1.Text = textBox1.Text + sssss + "\n\r";
                        textBox1.SelectionStart = textBox1.Text.Length - 1;
                        textBox1.ScrollToCaret();
                        //sssss = "AT+BT=60\r\n";
                        if (sssss.IndexOf("AT+ST=") == 0)
                        {
                            int i = sssss.IndexOf("AT+ST=") + 6;
                            int j = sssss.IndexOf("\r\n");
                            string strx = sssss.Substring(i, j - i + 2);
                            Console.WriteLine("ST = " + strx);
                            label8.Text = strx;
                        }
                        else if (sssss.IndexOf("AT+LO=") == 0)
                        {
                            int i = sssss.IndexOf("AT+LO=") + 6;
                            int j = sssss.IndexOf('\n');
                            string strx = sssss.Substring(i, sssss.Length - 6);
                            Console.WriteLine("LO = " + strx);
                            label6.Text = strx;
                            longitude = strx;
                            if (strx == "0")
                            {
                                label5.Text = "no locate";
                                this.label5.ForeColor = Color.Red;
                                isLocated = false;
                            }
                            else
                            {
                                label5.Text = "located";
                                this.label5.ForeColor = Color.Green;
                                isLocated = true;
                            }
                        }
                        else if (sssss.IndexOf("AT+LA=") == 0)
                        {
                            int i = sssss.IndexOf("AT+LA=") + 6;
                            int j = sssss.IndexOf('\n');
                            string strx = sssss.Substring(i, sssss.Length - 6);
                            Console.WriteLine("LA = " + strx);

   
                            label7.Text = "-"+strx;
                            latitudeX = strx;


           

                           /* mapflag++;
                            if (mapflag > 20)
                            {
                                mapflag = 0;
                                HttpPost();
                            }*/
                            //HttpPost();
                        }
                        else if (sssss.IndexOf("AT+BT=") == 0)
                        {
                            int i = sssss.IndexOf("AT+BT=") + 6;
                            int j = sssss.IndexOf('\n');
                            Console.WriteLine("j = " + j);
                            Console.WriteLine("i = " + i);
                            Console.WriteLine(sssss.Length);
                            string strx = sssss.Substring(i, sssss.Length - 6);
                            Console.WriteLine("BT = " + strx);

                            //   if (isLocated == true)
                            {
                                try
                                {
                                    int btInt = Int32.Parse(strx);
                                     //if (btInt < 150)
                                    {
                                        label10.Text = strx;
                                        value = strx;
                                        Console.WriteLine(btInt);
                                       // HttpPost();
                                    }
                                }
                                catch (System.FormatException)
                                {
                                    Console.WriteLine("conver error");
                                }

                            }
                        }


                        //HttpPost(value, longitude, latitude);

                    }
                    catch (TimeoutException) { }
                    catch (IOException) { }
                }
                else {
                    TimeSpan waitTime = new TimeSpan(0,0,0,50);
                    Thread.Sleep(waitTime);
                }
            }
        }
        




       /* private void button2_Click(object sender, EventArgs e)
        {
            //AT+ST=1
            //AT+BT=123
            //AT+LO=1234
            //AT+LA=123
            com.WriteLine("1234");
            Console.WriteLine("1234");
        }*/

        private void HttpPost()
        {
          
            try
            {
                int btInt = Int32.Parse(value);
                if (btInt == 0) return;
            }
            catch (System.FormatException)
            {
                Console.WriteLine("conver error");
                return;
            }
            


            String server = "ge2-6061.cloud.thingworx.com";

            String ss;
            ss = "POST /Thingworx/Things/eHealth/Services/eHealthService?appKey=d2560f02-2622-4a8a-a3fc-e806a8a99f7c&method=post&x-thingworx-session=true&heartBeat=";
            ss += value;//hrvData;
            ss += "&";
            ss += "longitude";
            ss += "=";
            ss += longitude;//gpsData.longitude;
            ss += "&";
            ss += "latitude";
            ss += "=";
            ss += "-";
            ss += latitudeX;//gpsData.latitude;
            ss += " HTTP/1.1\r\n";
            ss += "Host: ge2-4022.cloud.thingworx.com\r\n";
            ss += "Content-Type: text/html\r\n";
            ss += "\r\n";



            Console.WriteLine(ss);
            try
            {
                TcpClient tcpClient = new TcpClient();
                tcpClient.Connect(server, 80);

                NetworkStream ns = tcpClient.GetStream();


                if (ns.CanWrite)
                {
                    Byte[] sendBytes = Encoding.UTF8.GetBytes(ss);
                    ns.Write(sendBytes, 0, sendBytes.Length);
                }
                else
                {
                    MessageBox.Show("不能写入数据流", "终止", MessageBoxButtons.OK, MessageBoxIcon.Stop);
                    //Console.WriteLine("You cannot write data to this stream.");
                    tcpClient.Close();

                    // Closing the tcpClient instance does not close the network stream.
                    ns.Close();
                    return;
                }
                ns.Close();
                tcpClient.Close();
            }
            catch (SocketException) { }


        }

        private void button2_Click(object sender, EventArgs e)
        {
            if (isConnect == true) {
                _keepReading = false;
                com.Close();
                isConnect = false;
            }
            this.Close();
            /*com.WriteLine("1234");
            Console.WriteLine("1234");*/
        }
        void timer_Elapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            //模拟的做一些耗时的操作
            Console.WriteLine("1234");
            HttpPost();
            System.Threading.Thread.Sleep(1000000);
            System.Threading.Thread.Sleep(1000000);
        }


    }
}
