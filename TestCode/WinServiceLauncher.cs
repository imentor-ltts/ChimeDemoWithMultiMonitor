using System;
using System.Collections.Generic;
using System.Linq;
using System.ServiceProcess;
using System.Text;
using System.Threading.Tasks;

//I will create a new class that starts the service which internally starts the demo.exe. 
//Explain what needs to be done for integration. If possible document it and share it.
//
namespace TestCode
{
    /// <summary>
    /// A Class to start the Windows Service
    /// </summary>
    public static class WinServiceLauncher
    {
        /// <summary>
        /// Function that will start the RemoteLauncherService which will internally start demo.exe
        /// </summary>
        public static void StartService(string meetingId)
        {
            try
            {
                ServiceController service = new ServiceController("RemoteLauncherService");
                if (service == null)
                {
                    Console.WriteLine("Could not find the matching service");
                    return;
                }
                if ((service.Status == ServiceControllerStatus.Stopped) || (service.Status == ServiceControllerStatus.StopPending))
                {
                    service.Start(new string[] { meetingId });
                    Console.WriteLine("Service has started");
                }
                else
                {
                    Console.WriteLine("Service has started already, please stop the service and run this App again");
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                Console.WriteLine("Please install the service and try it.");
            }
        }
    }
}
