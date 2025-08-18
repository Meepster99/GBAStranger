// Made by mono21400

using System.Text;
using System;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using UndertaleModLib.Util;
using System.Linq;

EnsureDataLoaded();

string fntFolder = GetFolder(FilePath) + "ExportData/Export_Fonts" + Path.DirectorySeparatorChar;
TextureWorker worker = new TextureWorker();
Directory.CreateDirectory(fntFolder);
List<string> input = new List<string>();

foreach (var x in Data.Fonts)
{
    input.Add(x.Name.ToString().Replace("\"", ""));
}

string[] arrayString = input.ToArray();

await DumpFonts();
worker.Cleanup();

await StopProgressBarUpdater();
HideProgressBar();
ScriptMessage("Export Complete.\n\nLocation: " + fntFolder);


string GetFolder(string path)
{
    return Path.GetDirectoryName(path) + Path.DirectorySeparatorChar;
}

async Task DumpFonts()
{
    await Task.Run(() => Parallel.ForEach(Data.Fonts, DumpFont));
}

void DumpFont(UndertaleFont font)
{
    if (arrayString.Contains(font.Name.ToString().Replace("\"", "")))
    {
        worker.ExportAsPNG(font.Texture, fntFolder + font.Name.Content + ".png");
        using (StreamWriter writer = new StreamWriter(fntFolder + "glyphs_" + font.Name.Content + ".csv"))
        {
            writer.WriteLine(font.DisplayName + ";" + font.EmSize + ";" + font.Bold + ";" + font.Italic + ";" + font.Charset + ";" + font.AntiAliasing + ";" + font.ScaleX + ";" + font.ScaleY);

            foreach (var g in font.Glyphs)
            {
                writer.WriteLine(g.Character + ";" + g.SourceX + ";" + g.SourceY + ";" + g.SourceWidth + ";" + g.SourceHeight + ";" + g.Shift + ";" + g.Offset);
            }
        }
    }

    IncrementProgressParallel();
}

