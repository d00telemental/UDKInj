using UDKInj;

using var process = new ProcessHandle(p => p.ProcessName == "UDK");
using var injector = new Injector(process, bDisposeProcess: false);

//var module = new FileInfo(@"D:\Code\LExSDKGenerator\x64\UDK-2015-02\LExSDKGenerator.dll");
var module = new FileInfo(@"D:\Code\UDKInj\x64\Debug\UDKAgent.asi");
injector.Inject(module);

