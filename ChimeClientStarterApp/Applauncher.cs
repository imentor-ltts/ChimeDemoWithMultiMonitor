using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ChimeClientStarterApp
{
    internal class Applauncher
    {
        static int Main(string[] args)
        {
            var logger = new FileLogger();
            
            //const string exePath = "I:\\Chime\\demo.exe";
            const string exePath = "C:\\ProgramData\\ARS\\demo.exe";
            //const string exePath = "C:\\Windows\\System32\\notepad.exe";
            //check how many args are passed to the EXE...
            if (args.Length > 0)
            {
                foreach (string arg in args)
                {
                    logger.Information(arg);
                }
            };
            var result = false;
            //If the arg is only 1...
            if (args.Length == 0)
            {
                logger.Information($"The EXE to start is : {exePath}");
                result = ApplicationLoader.StartProcessAndBypassUAC(exePath);
                if (result == false)
                {
                    logger.Information($"Failed to start the process: {exePath}");
                    return -1;
                }
                return 0;
            }
            else
            {
                Console.WriteLine("Taking Command line arg for running the specified EXE");
                Console.WriteLine(args[0]);
                result = ApplicationLoader.StartProcessAndBypassUAC(args[0]);
                if (result == false)
                {
                    logger.Information($"Failed to start the process: {args[0]}");
                    return -1;
                }
                logger.Information("Application is exiting in a clean manner");
                return 0;
            }
        }
    }
}
