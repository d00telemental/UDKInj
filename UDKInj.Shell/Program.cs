using System.ComponentModel;
using UDKInj;

using var process = new ProcessHandle(p => p.ProcessName == "UDK");
using var injector = new Injector(process, bDisposeProcess: false);

var module = new FileInfo(@"D:\Code\UDKInj\x64\Debug\UDKAgent.asi");

try
{
    injector.Inject(module);
    // This line should probably have an exception block of its own.
    injector.Call(module, "ExportListObjects");
    injector.Eject(module);
}
catch (Win32Exception Exception)
{
    Console.WriteLine($"Win32 error: {Exception.Message}");
}
