namespace ProjectWizard;

using System;
using Avalonia;
using global::Avalonia.ReactiveUI;

internal static class Program
{
    [STAThread]
    public static int Main(string[] args)
    {
        return BuildAvaloniaApp()
            .StartWithClassicDesktopLifetime(args);
    }

    // Avalonia configuration, don't remove; also used by visual designer.
    private static AppBuilder BuildAvaloniaApp()
    {
        return AppBuilder.Configure<App>()
            .UsePlatformDetect()
            .LogToTrace()
            .UseReactiveUI();
    }
}
