using System.Diagnostics;
using System.ServiceProcess;

//When Blazor App makes the call to the API, the API should have the relevant rights to start the process in that machine. Or else the Logic does not fit as any Windows Service 


namespace TestCode
{
    internal class Program
    {

        static void RunProgram()
        {
            ChimeAppLauncherDll.ChimeLauncher chime = new ChimeAppLauncherDll.ChimeLauncher();
            chime.LaunchChimeApp("C:\\Chime-Cpp\\ars-remote-control\\demo\\Release\\demo.exe", "801445");
        }
        //Issue: This program works only on Admin mode
        static void Main(string[] args)
        {
            RunProgram();
            //WinServiceLauncher.StartService(args[0]);
        }
    }
}