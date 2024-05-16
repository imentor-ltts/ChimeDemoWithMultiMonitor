using AbhiSampleAppLauncher.Win32Apis;
using System.Diagnostics;
using System.Globalization;
using System.Security.Cryptography;
using System.Security.Principal;
using static AbhiSampleAppLauncher.Win32Apis.ADVAPI32;

namespace AbhiSampleAppLauncher
{
    internal class Program
    {
        static string exeFile = @"C:\Chime-Cpp\ars-remote-control\demo\Release\demo.exe";
        static string logFile = @"C:\ProgramData\AppLauncher.txt";
        private static Process? exeProcess;
        private const string _debugGuid = "f2b0a595-5ea8-471b-975f-12e70e0f3497";
        private static string host = "http://122.171.17.98:5000";
        
        static void logToFile(string logInfo)
        {
            File.AppendAllText(logFile, $"[{DateTime.Now.ToString("dd/MM/yyyy - hh:mm:ss")}]->{logInfo}\n");
        }
        static void Main(string[] args)
        {
            PROCESS_INFORMATION pInfo;
            try
            {
                if (!File.Exists(exeFile))
                {
                    logToFile("Remote control executable not found on target device.");
                    throw new FileNotFoundException("Remote control executable not found on target device.");
                }

                logToFile("Starting remote control");
                if (WindowsIdentity.GetCurrent().IsSystem) 
                {
                    var desktopName = "default";
                    var sessionId = Guid.NewGuid();
                    var accessKey = RandomGenerator.GenerateAccessKey();

                    var result = Win32Interop.CreateInteractiveSystemProcess(
                        exeFile +
                        $" --mode Unattended" +
                        $" --host {host}" +
                        $" --requester-name \"{"Phaniraj"}\"" +
                        $" --org-name \"{"Home"}\"" +
                        $" --org-id \"{AppConstants.DebugOrgId}\"" +
                        $" --session-id \"{sessionId}\"" +
                        $" --access-key \"{accessKey}\"",
                        desktopName, -1, false, true, out pInfo);
                    if (!result)
                    {
                        Console.WriteLine("Remote control failed to start on target device. Failed to start remote control.");
                        logToFile("Remote control failed to start on target device. Failed to start remote control.");
                    }
                    else
                    {
                        logToFile("Remote Control started on target device");
                        var process = Process.GetProcessById(pInfo.dwProcessId);
                        process.WaitForExit();
                    }
                }
                else
                {
                    var p_info = new ProcessStartInfo
                    {
                        UseShellExecute = true,
                        CreateNoWindow = false,
                        WindowStyle = ProcessWindowStyle.Normal,
                        FileName = exeFile
                    };
                    exeProcess = Process.Start(p_info);
                    if (exeProcess != null)
                    {
                        logToFile("Remote App has started");
                        exeProcess.WaitForExit();
                    }
                    logToFile("Launching Process has terminated, so current App is terminating");
                }
            }
            catch (Exception)
            {

                throw;
            }
        }
    }
}