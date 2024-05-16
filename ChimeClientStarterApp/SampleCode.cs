using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Threading.Tasks;

namespace ChimeClientStarterApp
{
    internal class SampleCode
    {
        static void Main(string[] args)
        {
            ILogger logger = new FileLogger();
            const string fileName = @"C:\ProgramData\ARS\demo.exe";
            Process currentProcess = Process.Start(fileName);
            if(currentProcess != null)
            {
                logger.LogInformation($"Current process {currentProcess.ProcessName} is running");
                currentProcess.WaitForExit();

            }
            Console.WriteLine("Waiting for the Demo to stop!!!");
            
        }
    }
}
