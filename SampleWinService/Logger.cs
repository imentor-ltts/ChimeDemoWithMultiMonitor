namespace ChimeAppLauncherWinService
{
    /// <summary>
    /// Class to log the messages
    /// </summary>
    internal static class Logger
    {
        public static string FileName { get; set; } = "AppServiceLog.txt";
        public static string LogFolderPath { get; set; } = @"C:\ProgramData\ChimeLogs";
        /// <summary>
        /// Message to log
        /// </summary>
        /// <param name="message">The Message to log</param>
        /// <param name="args">Optional Args to be passed</param>
        ///
        
        public static void LogMessage(string message)
        {
            string time = DateTime.Now.ToString("dd-MM-yyyy hh:mm:ss");
            string messageFormat = $"[{time}]: {message}\n";
            if(!Directory.Exists(LogFolderPath))
            {
                Directory.CreateDirectory(LogFolderPath);
            }
            string file = Path.Combine(LogFolderPath, FileName);
            File.AppendAllText(file, messageFormat);
        }
    }
}
