using System;
using System.IO;
using System.IO.Compression;
using System.Threading.Tasks;

namespace ProjectWizard.Services;

public static class ProjectCreator
{
    private const string SimpleProjectPath =
        @"C:\Users\c-j-1\Documents\Projects\Fayre\src\Atlas\src\Tools\ProjectWizard\Templates\SimpleWithSource.zip";

    private const string ComplexProjectPath =
        @"C:\Users\c-j-1\Documents\Projects\Fayre\src\Atlas\src\Tools\ProjectWizard\Templates\ComplexWithSource.zip";

    public static async Task CreateProjectAsync(ProjectCreateArguments args)
    {
        await using var fs = File.OpenRead(args.UseComplexProjectLayout ? ComplexProjectPath : SimpleProjectPath);
        using ZipArchive archive = new(fs, ZipArchiveMode.Read);
    }

    public record ProjectCreateArguments(string ProjectName, Uri OutputDirectory)
    {
        public bool InitializeGitRepository { get; init; }

        public bool UseComplexProjectLayout { get; init; }
    }
}
