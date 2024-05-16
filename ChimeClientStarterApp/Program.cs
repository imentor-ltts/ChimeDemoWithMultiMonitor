using System;
using System.Runtime.InteropServices;
using System.ComponentModel;
using System.Security.Principal;
using ChimeClientStarterApp;
using Microsoft.Extensions.Logging;
using System.Diagnostics;
using System.Configuration;

class Program
{
    
    [StructLayout(LayoutKind.Sequential)]
    public struct SECURITY_ATTRIBUTES
    {
        public int nLength;
        public IntPtr lpSecurityDescriptor;
        public int bInheritHandle;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct STARTUPINFO
    {
        public int cb;
        public string lpReserved;
        public string lpDesktop;
        public string lpTitle;
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
        public int dwProcessId;
        public int dwThreadId;
    }

    public enum SECURITY_IMPERSONATION_LEVEL
    {
        SecurityAnonymous,
        SecurityIdentification,
        SecurityImpersonation,
        SecurityDelegation
    }

    public enum TOKEN_TYPE
    {
        TokenPrimary = 1,
        TokenImpersonation
    }

    [Flags]
    public enum CreationFlags
    {
        CREATE_BREAKAWAY_FROM_JOB = 0x01000000,
        CREATE_DEFAULT_ERROR_MODE = 0x04000000,
        CREATE_NEW_CONSOLE = 0x00000010,
        CREATE_NEW_PROCESS_GROUP = 0x00000200,
        CREATE_NO_WINDOW = 0x08000000,
        CREATE_PROTECTED_PROCESS = 0x00040000,
        CREATE_PRESERVE_CODE_AUTHZ_LEVEL = 0x02000000,
        CREATE_SEPARATE_WOW_VDM = 0x00000800,
        CREATE_SHARED_WOW_VDM = 0x00001000,
        CREATE_SUSPENDED = 0x00000004,
        CREATE_UNICODE_ENVIRONMENT = 0x00000400,
        DEBUG_ONLY_THIS_PROCESS = 0x00000002,
        DEBUG_PROCESS = 0x00000001,
        DETACHED_PROCESS = 0x00000008,
        EXTENDED_STARTUPINFO_PRESENT = 0x00080000
    }

    [DllImport("advapi32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
    public static extern bool CreateProcessAsUser(
        IntPtr hToken,
        string lpApplicationName,
        string lpCommandLine,
        ref SECURITY_ATTRIBUTES lpProcessAttributes,
        ref SECURITY_ATTRIBUTES lpThreadAttributes,
        bool bInheritHandles,
        CreationFlags dwCreationFlags,
        IntPtr lpEnvironment,
        string lpCurrentDirectory,
        ref STARTUPINFO lpStartupInfo,
        out PROCESS_INFORMATION lpProcessInformation
    );

    [DllImport("advapi32.dll", SetLastError = true)]
    public static extern bool DuplicateTokenEx(
        IntPtr hExistingToken,
        uint dwDesiredAccess,
        ref SECURITY_ATTRIBUTES lpThreadAttributes,
        SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
        TOKEN_TYPE TokenType,
        out IntPtr phNewToken
    );

    [DllImport("kernel32.dll", SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool CloseHandle(IntPtr hObject);

    static void Main(string[] args)
    {
        var logger = new FileLogger();
        var exePath = ConfigurationManager.AppSettings["exePath"];
        
        // Obtain the primary token of the current user
        WindowsIdentity windowsIdentity = WindowsIdentity.GetCurrent();
        IntPtr userToken = windowsIdentity.Token;

        IntPtr hTokenDuplicate = IntPtr.Zero;
        try
        {
            SECURITY_ATTRIBUTES sa = new SECURITY_ATTRIBUTES();
            sa.nLength = Marshal.SizeOf(sa);

            // Duplicate the primary token for impersonation
            if (!DuplicateTokenEx(userToken, 0, ref sa,
                                   SECURITY_IMPERSONATION_LEVEL.SecurityImpersonation,
                                   TOKEN_TYPE.TokenPrimary, out hTokenDuplicate))
            {
                logger.LogError(Marshal.GetLastWin32Error().ToString());
                throw new Win32Exception(Marshal.GetLastWin32Error());
            }

            STARTUPINFO si = new STARTUPINFO();
            si.cb = Marshal.SizeOf(si);

            PROCESS_INFORMATION pi = new PROCESS_INFORMATION();
            logger.LogInformation($"EXE Path: {exePath}");
            // Create the process as the user
            if (!CreateProcessAsUser(hTokenDuplicate, exePath, null,
                                     ref sa, ref sa, false, CreationFlags.CREATE_NEW_CONSOLE,
                                     IntPtr.Zero, null, ref si, out pi))
            {
                logger.LogError(Marshal.GetLastWin32Error().ToString());
                throw new Win32Exception(Marshal.GetLastWin32Error());
            }
            logger.LogInformation($"Process created with PID: {pi.dwProcessId}");
            Process process = Process.GetProcessById(pi.dwProcessId);
            if(process != null) {
                logger.LogInformation($"Waiting for the Process {process.ProcessName} to exit");
                process.WaitForExit();
            }
            if(process == null)
            {
                logger.LogInformation("Demo Process is closed");
            }
        }
        catch (Exception ex)
        {
            logger.LogError(ex.ToString());
        }
        finally
        {
            if (hTokenDuplicate != IntPtr.Zero)
                CloseHandle(hTokenDuplicate);
        }
        Console.WriteLine("Press any key to exit");
        Console.ReadKey();
        logger.LogInformation("Process is exiting with Exit Code 0");
    }
}

