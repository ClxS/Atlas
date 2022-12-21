using Autofac;
using Avalonia.Markup.Xaml;
using Nephrite.Avalonia;
using ProjectWizard.Autofac;
using ProjectWizard.ViewModels;

namespace ProjectWizard;

public class App : ReactiveNephriteApplication<MainApplicationViewModel>
{
    protected override bool CanConfigure()
    {
        return true;
    }

    protected override bool SupportsConfigureWindow()
    {
        return false;
    }

    protected override bool SupportsSplashWindow()
    {
        return false;
    }

    protected override bool Configure(ContainerBuilder builder)
    {
        builder.RegisterModule<ProjectWizardModule>();
        return true;
    }

    public override void Initialize()
    {
        AvaloniaXamlLoader.Load(this);
    }
}


