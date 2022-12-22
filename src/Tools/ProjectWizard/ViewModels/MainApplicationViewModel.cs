using System;
using System.IO;
using System.Reactive.Disposables;
using System.Threading.Tasks;
using System.Windows.Input;
using Avalonia.Platform.Storage;
using ReactiveUI;
using ReactiveUI.Validation.Abstractions;
using ReactiveUI.Validation.Contexts;
using ReactiveUI.Validation.Extensions;
using Serilog;

namespace ProjectWizard.ViewModels;

public class MainApplicationViewModel : ReactiveObject, IActivatableViewModel, IValidatableViewModel
{
    private readonly Lazy<IStorageProvider> storageProvider;
    private bool includeEngineSource;
    private bool includeToolsSource;
    private bool useStandaloneAssetsProject = true;
    private bool addEditorIntegrationProject = true;
    private bool createGitRepository = true;
    private bool useComplexProjectLayout = true;
    private string projectName;
    private string projectDescription;
    private string projectLocation;

    public MainApplicationViewModel(Lazy<IStorageProvider> storageProvider)
    {
        this.storageProvider = storageProvider;
        this.Activator = new ViewModelActivator();
        this.ValidationContext = new ValidationContext();
        this.CreateProject = ReactiveCommand.Create(this.DoCreateProject, this.IsValid());

        this.WhenActivated(d =>
        {
            this.ValidationRule(
                vm => vm.ProjectName,
                name => !string.IsNullOrEmpty(name),
                "Project Name must be specified")
                .DisposeWith(d);
            this.ValidationRule(
                    vm => vm.ProjectLocation,
                    name => !string.IsNullOrEmpty(name),
                    "Project Location must be specified")
                .DisposeWith(d);
            this.ValidationRule(
                    vm => vm.ProjectLocation,
                    name => name != null && !File.Exists(name) && (!Directory.Exists(name) || (Directory.GetFiles(name).Length == 0 && Directory.GetDirectories(name).Length == 0)),
                    "Project Location must be an empty or non-existing folder")
                .DisposeWith(d);
        });
    }

    public ViewModelActivator Activator { get; }

    public ValidationContext ValidationContext { get; }

    public ICommand CreateProject { get; private set; }

    public bool IncludeEngineSource
    {
        get => includeEngineSource;
        set => this.RaiseAndSetIfChanged(ref includeEngineSource, value);
    }

    public bool IncludeToolsSource
    {
        get => includeToolsSource;
        set => this.RaiseAndSetIfChanged(ref includeToolsSource, value);
    }

    public bool UseStandaloneAssetsProject
    {
        get => useStandaloneAssetsProject;
        set => this.RaiseAndSetIfChanged(ref useStandaloneAssetsProject, value);
    }

    public bool UseComplexProjectLayout
    {
        get => useComplexProjectLayout;
        set => this.RaiseAndSetIfChanged(ref useComplexProjectLayout, value);
    }

    public bool AddEditorIntegrationProject
    {
        get => addEditorIntegrationProject;
        set => this.RaiseAndSetIfChanged(ref addEditorIntegrationProject, value);
    }

    public bool CreateGitRepository
    {
        get => createGitRepository;
        set => this.RaiseAndSetIfChanged(ref createGitRepository, value);
    }

    public string ProjectName
    {
        get => projectName;
        set => this.RaiseAndSetIfChanged(ref projectName, value);
    }

    public string ProjectDescription
    {
        get => projectDescription;
        set => this.RaiseAndSetIfChanged(ref projectDescription, value);
    }

    public string ProjectLocation
    {
        get => projectLocation;
        set => this.RaiseAndSetIfChanged(ref projectLocation, value);
    }

    public async Task BrowseProjectLocation()
    {
        var provider = this.storageProvider.Value;
        if (!provider.CanPickFolder)
        {
            return;
        }

        var result = await provider.OpenFolderPickerAsync(new FolderPickerOpenOptions()
        {
            AllowMultiple = false
        });

        if (result.Count > 0)
        {
            if (result[0].TryGetUri(out var uri))
            {
                this.ProjectLocation = uri.OriginalString;
            }
        }
    }

    public void DoCreateProject()
    {
        if (!this.ValidationContext.IsValid)
        {
            return;
        }
    }
}
