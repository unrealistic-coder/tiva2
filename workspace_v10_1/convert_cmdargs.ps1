$binsrcPath = "D:\html\binsrc\binsrc.exe"   # Replace with the actual path to binsrc.exe
# param( [Parameter(Mandatory=$true)] [string]$i, [Parameter(Mandatory=$true)] [string]$o )
$inputFolder = $args[0]  # Replace with the actual folder path containing files
$outputFolder = $args[1] # Replace with the path where you want the output files

# Create output folder if it doesn't exist
if (-not (Test-Path $outputFolder)) {
    New-Item -ItemType Directory -Force -Path $outputFolder
}

# Loop through each file in the input folder (and its subfolders)
Get-ChildItem -Path $inputFolder -Recurse | ForEach-Object {
    if ($_ -is [System.IO.FileInfo]) {  # Ensure it's a file, not a directory
        $inputFile = $_.FullName
        $outputFile = Join-Path $outputFolder ($_.BaseName +"_" + $_.Extension.Substring(1) + ".h")  # Modify output file name if needed
        $array_name = ($_.BaseName + "_" + $_.Extension.Substring(1))
        # Run binsrc tool with input and output files
        Write-Host "Processing $inputFile..."
        & $binsrcPath $inputFile $outputFile $array_name
    }
}

Write-Host "Processing complete!"
