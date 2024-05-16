
using System.Security.Cryptography;

public class RandomGenerator
{
    private const string AllowableCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIGKLMNOPQRSTUVWXYZ0123456789";

    public static string GenerateString(int length)
    {
        var bytes = RandomNumberGenerator.GetBytes(length);
        return new string(bytes.Select(x => AllowableCharacters[x % AllowableCharacters.Length]).ToArray());
    }

    public static string GenerateAccessKey()
    {
        return GenerateString(64);
    }
}

public class AppConstants
{
    public const string DefaultProductName = "ARS RemoteControl";
    public const string DefaultPublisherName = "Siemens Healthineers";
    public const string DebugOrgId = "e8f4ad87-4a4b-4da1-bcb2-1788eaeb80e8";
    public const int EmbeddedDataBlockLength = 256;
    public const long MaxUploadFileSize = 100_000_000;
    public const double ScriptRunExpirationMinutes = 30;
    public const string ApiKeyHeaderName = "X-Api-Key";
    public const string ExpiringTokenHeaderName = "X-Expiring-Token";

#pragma warning disable IDE0230 // Use UTF-8 string literal
    public static byte[] EmbeddedImmySignature { get; } = new byte[] { 73, 109, 109, 121, 66, 111, 116, 32, 114, 111, 99, 107, 115, 32, 116, 104, 101, 32, 115, 111, 99, 107, 115, 32, 117, 110, 116, 105, 108, 32, 116, 104, 101, 32, 101, 113, 117, 105, 110, 111, 120, 33 };
#pragma warning restore IDE0230 // Use UTF-8 string literal
}
