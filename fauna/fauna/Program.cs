using System;
using System.Collections.Generic;
using System.Text;
using System.Net;
using System.Net.NetworkInformation;
using System.Net.Sockets;
using libusbK;

namespace fauna
{
    public enum LogType
    {
        Information,
        Warning,
        Error,
    }

    public class LogMessage
    {
        public string Message { get; set; }

        public string FileName { get; set; }

        public string FuncName { get; set; }
    }

    public static class Program
    {
        public static int BrewPort = 42760;

        public static TcpClient tcp;

        public static ulong LastAppId = 0;

        public static void Log(string LogText, LogType Type, bool NewLine = true)
        {
            Console.ForegroundColor = ConsoleColor.Gray;
            Console.Write("<");
            switch (Type)
            {
                case LogType.Error:
                    Console.ForegroundColor = ConsoleColor.Red;
                    Console.Write("Error");
                    break;
                case LogType.Information:
                    Console.ForegroundColor = ConsoleColor.Green;
                    Console.Write("Log");
                    break;
                case LogType.Warning:
                    Console.ForegroundColor = ConsoleColor.Yellow;
                    Console.Write("Warn");
                    break;
            }

            Console.ForegroundColor = ConsoleColor.Gray;
            Console.Write("> ");
            Console.ForegroundColor = ConsoleColor.White;
            if (NewLine) Console.WriteLine(LogText);
            else Console.Write(LogText);
        }

        public static uint NetRead32(NetworkStream strm)
        {
            var buf = new byte[4];
            strm.Read(buf, 0, 4);
            return BitConverter.ToUInt32(buf, 0);
        }
        public static ulong NetRead64(NetworkStream strm)
        {
            var buf = new byte[8];
            strm.Read(buf, 0, 8);
            return BitConverter.ToUInt64(buf, 0);
        }

        public static string NetReadString(NetworkStream strm)
        {
            var len = NetRead32(strm);
            var buf = new byte[len];
            strm.Read(buf, 0, (int)len);
            return Encoding.UTF8.GetString(buf).Trim('\0');
        }


        public static void HandleSentLogData()
        {
            NetworkStream nets = tcp.GetStream();
            if(nets.DataAvailable)
            {
                var headerbuf = new byte[4 + 8];
                nets.Read(headerbuf, 0, 4 + 8);

                var appid = BitConverter.ToUInt64(headerbuf, 0);
                var logtype = BitConverter.ToUInt32(headerbuf, 8);

                var log = NetReadString(nets);

                switch(logtype)
                {
                    case 1:
                        LogStdout(appid, log);
                        break;
                    case 2:
                        LogStderr(appid, log);
                        break;
                }
            }
        }

        public static void PreLog(ulong AppId)
        {
            Console.ForegroundColor = ConsoleColor.White;
            if (AppId != LastAppId)
            {
                LastAppId = AppId;
                Console.Write('[');
                Console.ForegroundColor = ConsoleColor.Cyan;
                Console.Write(string.Format("{0:X16}", AppId));
                Console.ForegroundColor = ConsoleColor.White;
                Console.WriteLine("]");
            }
        }

        public static void LogMsg(string Msg)
        {
            Console.ForegroundColor = ConsoleColor.Gray;
            Console.WriteLine(Msg);
        }

        public static void Log(ulong AppId, string Msg, string FileName, string FuncName, uint LineNo)
        {
            PreLog(AppId);
            Console.ForegroundColor = ConsoleColor.Cyan;
            Console.Write(FileName);
            Console.ForegroundColor = ConsoleColor.White;
            Console.Write(':');
            Console.ForegroundColor = ConsoleColor.Cyan;
            Console.Write(LineNo);
            Console.ForegroundColor = ConsoleColor.White;
            Console.Write(" (");
            Console.ForegroundColor = ConsoleColor.Cyan;
            Console.Write(FuncName);
            Console.ForegroundColor = ConsoleColor.White;
            Console.Write(") ");
            LogMsg(Msg);
        }

        public static void LogStdout(ulong AppId, string Msg)
        {
            PreLog(AppId);
            Console.ForegroundColor = ConsoleColor.White;
            Console.Write('(');
            Console.ForegroundColor = ConsoleColor.Cyan;
            Console.Write("stdout");
            Console.ForegroundColor = ConsoleColor.White;
            Console.Write(") ");
            LogMsg(Msg);
        }

        public static void LogStderr(ulong AppId, string Msg)
        {
            PreLog(AppId);
            Console.ForegroundColor = ConsoleColor.White;
            Console.Write('(');
            Console.ForegroundColor = ConsoleColor.Red;
            Console.Write("stderr");
            Console.ForegroundColor = ConsoleColor.White;
            Console.Write(") ");
            LogMsg(Msg);
        }

        public static void Main()
        {
            Console.Title = "Fauna - Biosphere's logging interface";
            try
            {
                Console.WriteLine();
                Console.Write("Console's IP address ");
                Console.ForegroundColor = ConsoleColor.Green;
                Console.Write("> ");
                Console.ForegroundColor = ConsoleColor.White;

                string ipaddr = Console.ReadLine();
                tcp = new TcpClient();
                Console.WriteLine();
                
                connectloop:
                try
                {
                    Console.WriteLine("Waiting for connection with 'flora' system process...");
                    tcp.Connect(ipaddr, 11762);
                }
                catch
                {
                    goto connectloop;
                }

                Console.ForegroundColor = ConsoleColor.Green;
                Console.WriteLine("Connection established.");
                Console.ForegroundColor = ConsoleColor.Gray;
                Console.WriteLine();

                while(true) HandleSentLogData();
            }
            catch (Exception ex)
            {
                Log("An error happened (" + ex.GetType().ToString() + "): " + ex.Message, LogType.Error);
                Console.ForegroundColor = ConsoleColor.Gray;
                Console.Write(" - ");
                Console.ForegroundColor = ConsoleColor.White;
                Console.WriteLine("Press any key to exit.");
                Console.ReadKey();
            }
        }
    }
}
