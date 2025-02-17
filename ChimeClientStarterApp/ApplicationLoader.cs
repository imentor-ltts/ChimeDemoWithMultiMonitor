﻿//using Microsoft.Extensions.Logging;
//using System.ComponentModel;
//using System.Diagnostics;
//using System.Runtime.InteropServices;
//using System.Security;

//namespace ChimeClientStarterApp
//{
//    internal class ApplicationLoader
//    {
//        public static  ILogger _logger;

//        public static void SetLogger(ILogger logger)
//        {
//            _logger = logger;
//        }
//        [StructLayout(LayoutKind.Sequential)]
//        public struct SECURITY_ATTRIBUTES
//        {
//            public int Length;
//            public IntPtr lpSecurityDescriptor;
//            public bool bInheritHandle;
//        }
//        [StructLayout(LayoutKind.Sequential)]
//        public struct STARTUPINFO
//        {
//            public int cb;
//            public String lpReserved;
//            public String lpDesktop;
//            public String lpTitle;
//            public uint dwX;
//            public uint dwY;
//            public uint dwXSize;
//            public uint dwYSize;
//            public uint dwXCountChars;
//            public uint dwYCountChars;
//            public uint dwFillAttribute;
//            public uint dwFlags;
//            public short wShowWindow;
//            public short cbReserved2;
//            public IntPtr lpReserved2;
//            public IntPtr hStdInput;
//            public IntPtr hStdOutput;
//            public IntPtr hStdError;
//        }

//        [StructLayout(LayoutKind.Sequential)]
//        public struct PROCESS_INFORMATION
//        {
//            public IntPtr hProcess;
//            public IntPtr hThread;
//            public uint dwProcessId;
//            public uint dwThreadId;
//        }
//        private enum TOKEN_TYPE : int
//        {
//            TokenPrimary = 1,
//            TokenImpersonation = 2
//        }
//        private enum SECURITY_IMPERSONATION_LEVEL : int
//        {
//            SecurityAnonymous = 0,
//            SecurityIdentification = 1,
//            SecurityImpersonation = 2,
//            SecurityDelegation = 3,
//        }


//        public const int TOKEN_DUPLICATE = 0x0002;
//        public const uint MAXIMUM_ALLOWED = 0x2000000;
//        public const int CREATE_NEW_CONSOLE = 0x00000010;
//        public const int IDLE_PRIORITY_CLASS = 0x40;
//        public const int NORMAL_PRIORITY_CLASS = 0x20;
//        public const int HIGH_PRIORITY_CLASS = 0x80;
//        public const int REALTIME_PRIORITY_CLASS = 0x100;
//        public static uint ProcessID = 0;

//        [DllImport("kernel32.dll", SetLastError = true)] private static extern bool CloseHandle(IntPtr hSnapshot);
//        [DllImport("kernel32.dll")] static extern uint WTSGetActiveConsoleSessionId();
//        [DllImport("advapi32.dll", EntryPoint = "CreateProcessAsUser", SetLastError = true, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)] public extern static bool CreateProcessAsUser(IntPtr hToken, String lpApplicationName, String lpCommandLine, ref SECURITY_ATTRIBUTES lpProcessAttributes, ref SECURITY_ATTRIBUTES lpThreadAttributes, bool bInheritHandle, int dwCreationFlags, IntPtr lpEnvironment, String lpCurrentDirectory, ref STARTUPINFO lpStartupInfo, out PROCESS_INFORMATION lpProcessInformation);
//        [DllImport("kernel32.dll")] static extern bool ProcessIdToSessionId(uint dwProcessId, ref uint pSessionId);
//        [DllImport("advapi32.dll", EntryPoint = "DuplicateTokenEx")] public extern static bool DuplicateTokenEx(IntPtr ExistingTokenHandle, uint dwDesiredAccess, ref SECURITY_ATTRIBUTES lpThreadAttributes, int TokenType, int ImpersonationLevel, ref IntPtr DuplicateTokenHandle);
//        [DllImport("kernel32.dll")] static extern IntPtr OpenProcess(uint dwDesiredAccess, bool bInheritHandle, uint dwProcessId);
//        [DllImport("advapi32", SetLastError = true), SuppressUnmanagedCodeSecurityAttribute] static extern bool OpenProcessToken(IntPtr ProcessHandle, int DesiredAccess, ref IntPtr TokenHandle);


//        public static bool StartProcessAndBypassUAC(String applicationName)
//        {
//            uint winlogonPid = 0;
//            IntPtr hUserTokenDup = IntPtr.Zero, hPToken = IntPtr.Zero, hProcess = IntPtr.Zero;
//            var procInfo = new PROCESS_INFORMATION();
//            var dwSessionId = WTSGetActiveConsoleSessionId();
//            var processes = Process.GetProcessesByName("winlogon");
//            foreach (var p in processes)
//            {
//                if ((uint)p.SessionId == dwSessionId)
//                {
//                    winlogonPid = (uint)p.Id;
//                }
//            }


//            hProcess = OpenProcess(MAXIMUM_ALLOWED, false, winlogonPid);
//            if (!OpenProcessToken(hProcess, TOKEN_DUPLICATE, ref hPToken))
//            {
//                _logger.Log(LogLevel.Error, "Failed to get the Token");
//                _logger.LogInformation("Failed to get the token");
//                CloseHandle(hProcess);
//                return false;
//            }


//            var sa = new SECURITY_ATTRIBUTES();
//            sa.Length = Marshal.SizeOf(sa);
//            if (!DuplicateTokenEx(hPToken, MAXIMUM_ALLOWED, ref sa, (int)SECURITY_IMPERSONATION_LEVEL.SecurityIdentification, (int)TOKEN_TYPE.TokenPrimary, ref hUserTokenDup))
//            {
//                _logger.Log(LogLevel.Error, "Failed to get the Duplicate token in DuplicateTokenEx function");
//                _logger.LogInformation("Failed to get the Duplicate token in DuplicateTokenEx function");
//                CloseHandle(hProcess);
//                CloseHandle(hPToken);
//                return false;
//            }


//            var si = new STARTUPINFO();
//            si.cb = (int)Marshal.SizeOf(si);
//            si.lpDesktop = "winsta0\\default";
//            var dwCreationFlags = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE;
//            var result = false;
//            try
//            {
//                result = CreateProcessAsUser(hUserTokenDup, applicationName, "801445", ref sa, ref sa, false, dwCreationFlags, IntPtr.Zero, null, ref si, out procInfo);
//                if (!result)
//                {
//                    _logger.LogError($"Failed to start the process: {applicationName} at the CreateProcessAsUser API");
//                    throw new Win32Exception(Marshal.GetLastWin32Error());
//                }
//            }
//            catch(Exception ex)
//            {
//                Console.WriteLine(ex.Message);
//                _logger.LogError($"Failed to start the process {applicationName}");
//                _logger.LogError(ex.ToString());
//            }

//            try
//            {
//                Console.WriteLine("Process ID: " + procInfo.dwProcessId.ToString());
//                Process process = Process.GetProcessById((int)(procInfo.dwProcessId));
//                ProcessID = procInfo.dwProcessId;
//                if (process != null)
//                {
//                    Console.WriteLine(DateTime.Now.ToString() + " Waiting for process to exit");
//                    try
//                    {
//                        _logger.LogInformation("waiting for the created process to exit");
//                        process.WaitForExit();
//                    }
//                    catch (Exception ex)
//                    {
//                        _logger.LogError(ex.Message);                     
//                    }
//                    Console.WriteLine(DateTime.Now.ToString() + " Process exited");
//                }
//            }
//            catch (Exception ex)
//            {
//                Console.WriteLine(ex.ToString());
//                _logger.LogError(ex.ToString() );   
//            }


//            CloseHandle(procInfo.hProcess);
//            CloseHandle(procInfo.hThread);
//            CloseHandle(hProcess);
//            CloseHandle(hPToken);
//            CloseHandle(hUserTokenDup);
//            return result;
//        }

//        public static void Teminate(string applicationName)
//        {
//            Process process = Process.GetProcessById((int)ProcessID);
//            if (process != null)
//            {
//                process.Kill();
//            }

//        }
//    }
//}

using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Security;
using System.Linq;
public class ApplicationLoader
{
    [StructLayout(LayoutKind.Sequential)]
    public struct SECURITY_ATTRIBUTES
    {
        public int Length;
        public IntPtr lpSecurityDescriptor;
        public bool bInheritHandle;
    }
    [StructLayout(LayoutKind.Sequential)]
    public struct STARTUPINFO
    {
        public int cb;
        public String lpReserved;
        public String lpDesktop;
        public String lpTitle;
        public uint dwX;
        public uint dwY;
        public uint dwXSize;
        public uint dwYSize;
        public uint dwXCountChars;
        public uint dwYCountChars;
        public uint dwFillAttribute;
        public uint dwFlags;
        public short wShowWindow;
        public short cbReserved2;
        public IntPtr lpReserved2;
        public IntPtr hStdInput;
        public IntPtr hStdOutput;
        public IntPtr hStdError;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct PROCESS_INFORMATION
    {
        public IntPtr hProcess;
        public IntPtr hThread;
        public uint dwProcessId;
        public uint dwThreadId;
    }
    private enum TOKEN_TYPE : int
    {
        TokenPrimary = 1,
        TokenImpersonation = 2
    }
    private enum SECURITY_IMPERSONATION_LEVEL : int
    {
        SecurityAnonymous = 0,
        SecurityIdentification = 1,
        SecurityImpersonation = 2,
        SecurityDelegation = 3,
    }


    public const int TOKEN_DUPLICATE = 0x0002;
    public const uint MAXIMUM_ALLOWED = 0x2000000;
    public const int CREATE_NEW_CONSOLE = 0x00000010;
    public const int CREATE_UNICODE_ENVIRONMENT = 0x00000400;
    public const int IDLE_PRIORITY_CLASS = 0x40;
    public const int NORMAL_PRIORITY_CLASS = 0x20;
    public const int HIGH_PRIORITY_CLASS = 0x80;
    public const int REALTIME_PRIORITY_CLASS = 0x100;
    public static uint ProcessID = 0;

    [DllImport("kernel32.dll", SetLastError = true)] private static extern bool CloseHandle(IntPtr hSnapshot);
    [DllImport("kernel32.dll")] static extern uint WTSGetActiveConsoleSessionId();
    [DllImport("advapi32.dll", EntryPoint = "CreateProcessAsUser", SetLastError = true, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)] public extern static bool CreateProcessAsUser(IntPtr hToken, String lpApplicationName, String lpCommandLine, ref SECURITY_ATTRIBUTES lpProcessAttributes, ref SECURITY_ATTRIBUTES lpThreadAttributes, bool bInheritHandle, int dwCreationFlags, IntPtr lpEnvironment, String lpCurrentDirectory, ref STARTUPINFO lpStartupInfo, out PROCESS_INFORMATION lpProcessInformation);
    [DllImport("kernel32.dll")] static extern bool ProcessIdToSessionId(uint dwProcessId, ref uint pSessionId);
    [DllImport("advapi32.dll", EntryPoint = "DuplicateTokenEx")] public extern static bool DuplicateTokenEx(IntPtr ExistingTokenHandle, uint dwDesiredAccess, ref SECURITY_ATTRIBUTES lpThreadAttributes, int TokenType, int ImpersonationLevel, ref IntPtr DuplicateTokenHandle);
    [DllImport("kernel32.dll")] static extern IntPtr OpenProcess(uint dwDesiredAccess, bool bInheritHandle, uint dwProcessId);
    [DllImport("advapi32", SetLastError = true), SuppressUnmanagedCodeSecurityAttribute] static extern bool OpenProcessToken(IntPtr ProcessHandle, int DesiredAccess, ref IntPtr TokenHandle);


    public static bool StartProcessAndBypassUAC(String applicationName)
    {
        uint winlogonPid = 0;
        IntPtr hUserTokenDup = IntPtr.Zero, hPToken = IntPtr.Zero, hProcess = IntPtr.Zero;
        var procInfo = new PROCESS_INFORMATION();
        var dwSessionId = WTSGetActiveConsoleSessionId();
        var processes = Process.GetProcessesByName("winlogon");
        foreach (var p in processes)
        {
            if ((uint)p.SessionId == dwSessionId)
            {
                winlogonPid = (uint)p.Id;
            }
        }


        hProcess = OpenProcess(MAXIMUM_ALLOWED, false, winlogonPid);
        if (!OpenProcessToken(hProcess, TOKEN_DUPLICATE, ref hPToken))
        {
            CloseHandle(hProcess);
            return false;
        }


        var sa = new SECURITY_ATTRIBUTES();
        sa.Length = Marshal.SizeOf(sa);
        if (!DuplicateTokenEx(hPToken, MAXIMUM_ALLOWED, ref sa, (int)SECURITY_IMPERSONATION_LEVEL.SecurityIdentification, (int)TOKEN_TYPE.TokenPrimary, ref hUserTokenDup))
        {
            CloseHandle(hProcess);
            CloseHandle(hPToken);
            return false;
        }


        var si = new STARTUPINFO();
        si.cb = (int)Marshal.SizeOf(si);
        si.lpDesktop = "winsta0\\default";
        var dwCreationFlags = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT;
        var result = CreateProcessAsUser(hUserTokenDup, null, applicationName, ref sa, ref sa, false, dwCreationFlags, IntPtr.Zero, null, ref si, out procInfo);


        try
        {
            Console.WriteLine("Process ID: " + procInfo.dwProcessId.ToString());
            Process process = Process.GetProcessById((int)(procInfo.dwProcessId));
            ProcessID = procInfo.dwProcessId;
            if (process != null)
            {
                Console.WriteLine(DateTime.Now.ToString() + " Waiting for process to exit");
                process.WaitForExit();
                Console.WriteLine(DateTime.Now.ToString() + " Process exited");
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine(ex.ToString());
        }


        CloseHandle(procInfo.hProcess);
        CloseHandle(procInfo.hThread);
        CloseHandle(hProcess);
        CloseHandle(hPToken);
        CloseHandle(hUserTokenDup);
        return result;
    }


    public static void Execute(string applicationName)
    {
        StartProcessAndBypassUAC(applicationName);
    }

    public static void Teminate(string applicationName)
    {
        Process process = Process.GetProcessById((int)ProcessID);
        if (process != null)
        {
            process.Kill();
        }

    }
}
