public enum WindowsSessionType
{
    Console = 1,
    RDP = 2
}

public class WindowsSession
{
    public uint Id { get; set; }

    public string Name { get; set; } = string.Empty;
    public WindowsSessionType Type { get; set; }
    public string Username { get; set; } = string.Empty;
}
