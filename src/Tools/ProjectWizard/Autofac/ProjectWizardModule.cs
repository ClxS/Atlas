using Autofac;
using Autofac.Features.AttributeFilters;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Platform.Storage;
using Avalonia.VisualTree;
using Nephrite.Autofac;
using Nephrite.Avalonia.Autofac;
using Nephrite.UI.Autofac;
using ProjectWizard.ViewModels;
using ProjectWizard.Views;

namespace ProjectWizard.Autofac;

public class ProjectWizardModule : Module
{
    protected override void Load(ContainerBuilder builder)
    {
        builder.AddDomServices();
        builder.AddDefaultDocumentService();
        builder.AddCommands();
        builder.AddRegioning(true);
        builder.AddMenuService();
        builder.AddReactiveUIViewLocator();
        builder.RegisterMessengers();
        builder.RegisterFileSystem();

        RegisterAvalonia(builder);
        RegisterUI(builder);
    }

    private static void RegisterAvalonia(ContainerBuilder builder)
    {
        builder.Register<IStorageProvider>((c) =>
            (c.Resolve<MainWindow>().GetVisualRoot() as TopLevel)?.StorageProvider!);
    }

    private static void RegisterUI(ContainerBuilder builder)
    {
        builder.RegisterType<MainApplicationViewModel>().WithAttributeFiltering();
        builder.RegisterType<MainWindow>().SingleInstance().AsImplementedInterfaces().AsSelf();
    }
}
