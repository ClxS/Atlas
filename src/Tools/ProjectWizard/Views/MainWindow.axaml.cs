using System.Reactive.Disposables;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.ReactiveUI;
using Nephrite.Avalonia;
using Nephrite.Avalonia.Controls;
using ProjectWizard.ViewModels;
using ReactiveUI;
using ReactiveUI.Validation.Extensions;

namespace ProjectWizard.Views;

public partial class MainWindow : NephriteWindow<MainApplicationViewModel>
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
                    view => view.NameValidation.ValidationError)
                .DisposeWith(d);
            this.BindValidation(
                    this.ViewModel,
                    vm => vm.ProjectLocation,
                    view => view.LocationValidation.ValidationError)
                .DisposeWith(d);
            this.BindValidation(
                    this.ViewModel,
                    vm => vm.IncludeEngineSource,
                    view => view.SourceValidation.ValidationError)
                .DisposeWith(d);
            this.BindValidation(
                    this.ViewModel,
                    vm => vm.IncludeToolsSource,
                    view => view.SourceValidation.ValidationError)
                .DisposeWith(d);
        });
    }

    public ValidationAdornerBorder NameValidation => this.FindControl<ValidationAdornerBorder>("ProjectNameValidation");

    public ValidationAdornerBorder LocationValidation => this.FindControl<ValidationAdornerBorder>("ProjectLocationValidation");

    public ValidationAdornerBorder SourceValidation => this.FindControl<ValidationAdornerBorder>("IncludeSourceValidationError");

    private void InitializeComponent()
    {
        AvaloniaXamlLoader.Load(this);
    }

    private void BrowseButton_OnClick(object? sender, RoutedEventArgs e)
    {
        this.ViewModel?.BrowseProjectLocation();
    }
}

