using System.Reactive.Disposables;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.ReactiveUI;
using ProjectWizard.ViewModels;
using ReactiveUI;
using ReactiveUI.Validation.Extensions;

namespace ProjectWizard.Views;

public partial class MainWindow : ReactiveWindow<MainApplicationViewModel>
{
    public MainWindow()
    {
        InitializeComponent();
        this.AttachDevTools();
        this.WhenActivated(d =>
        {
            this.BindValidation(
                    this.ViewModel,
                    vm => vm.ProjectName,
                    view => view.NameValidation.Text)
                .DisposeWith(d);
            this.BindValidation(
                    this.ViewModel,
                    vm => vm.ProjectLocation,
                    view => view.LocationValidation.Text)
                .DisposeWith(d);
        });
    }

    public TextBlock NameValidation => this.FindControl<TextBlock>("ProjectNameValidation");

    public TextBlock LocationValidation => this.FindControl<TextBlock>("ProjectLocationValidation");

    private void InitializeComponent()
    {
        AvaloniaXamlLoader.Load(this);
    }

    private void BrowseButton_OnClick(object? sender, RoutedEventArgs e)
    {
        this.ViewModel?.BrowseProjectLocation();
    }
}

