namespace RemoteSessionApi.Data
{
    public class SessionDetail
    {
        public string SessionId { get; set; } = string.Empty;
        public string UUId { get; set; } = string.Empty;
        public string SerialNumber { get; set; } = string.Empty;
        public string MeetingId { get; set; }
    }
}
