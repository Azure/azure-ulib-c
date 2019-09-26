# cleanupMockable converts a single line containing a MOCKABLE(..., ...) into a C friendly function declaration.
function cleanupMockableLine([string]$mockableFunctionName) {
    $mockableFunctionName = $mockableFunctionName -replace "MOCKABLE_FUNCTION\(, ", ""
    $splitString = $mockableFunctionName -split ","

    if ($splitString.length -eq 2) {
        # Special case the scenario where the function takes no arguments.  The $splitString[1] will retain a closing ) but not the (), so tease this out.
        $prettyFunctionName = $splitString[0] + ($splitString[1] -replace "\);", "") + "();"
    }
    else {
        # Function takes arguments, which need to be marched through two by two (for type and parameter name)
        # First special case to get the function name and return type.
        $prettyFunctionName = $splitString[0] + $splitString[1] + "("

        for ($i = 2; $i -lt $splitString.Length; $i += 2) {
            $prettyFunctionName += $splitString[$i] + $splitString[$i + 1]

            if (($i + 2) -ne $splitString.Length) {
                $prettyFunctionName += ","
            }
        }
    }

    # There is no need to add an explicit ");" to end of string, as this came out as part of MOCKABLE process
    Write-Output $prettyFunctionName
}

# cleanupMockable converts a single line containing a MU_DEFINE_ENUM(..., ...) into a C friendly enumeration declaration.
function cleanupEnumLine([string]$enumName) {

    $enumRegex = 'MU_DEFINE_ENUM\(\s*?(\w*),([\s\w\W]*)\)'
    $regexReplace = 'typedef enum $1_TAG {$2} $1'
    $prettyEnumName = $enumName -replace $enumRegex, $regexReplace

    # There is no need to add an explicit ");" to end of string, as this came out as part of MOCKABLE process
    Write-Output $prettyEnumName
}

# cleanupMockablesInFile operates over a single header file, stripping any instances of MOCKABLE(...) and dealing with the MU_ENUM stuff.  Its final output is a cleaned up header
function cleanupCUtilMacrosInFile([string]$headerFileName) {
    $content = Get-Content $headerFileName
    $newLine = "`r`n"
    $functionText = ""
    $enumText = ""
    foreach ($line in $content) {
        if ($line -match "MOCKABLE_FUNCTION" -or $functionText -ne "") {
            if ($line -match [regex]::escape(");")) {
                $functionText = $functionText + $line.trim()
                Write-Output (cleanupMockableLine $functionText)
                $functionText = ""
            }
            else {
                $functionText = $functionText + $line.trim()
            }
            # If a MOCKABLE_FUNCTION is hit, invoke the helper that cleans up a specific line.
        }
        elseif ($line -match "MU_DEFINE_ENUM" -or $enumText -ne "") {
            if ($line -match [regex]::escape(");")) {
                $enumText = $enumText + $line + $newLine
                Write-Output (cleanupEnumLine $enumText)
                $enumText = ""
            }
            else {
                $enumText = $enumText + $line + $newLine
            }
        }
        else {
            # no MOCKABLE to process so just output line
            Write-Output $line
        }
    }
}

#Project directories to work with
$currentDir = $PSScriptRoot
$projectRoot = "$currentDir\.."
$incfiles = Get-ChildItem "$projectRoot\inc\*.h"
$configfiles = Get-ChildItem "$projectRoot\config\*.h"
$palfiles = Get-ChildItem "$projectRoot\pal\os\inc\*.h"
$incDir = "$projectRoot\doxy\inc"
$configDir = "$projectRoot\doxy\config"
$palDir = "$projectRoot\doxy\pal\os\inc"

Push-Location $projectRoot

# Create the output dir if it doesn't exist already
if ((Test-Path $incDir) -eq $false) {
    mkdir $incDir
}
if ((Test-Path $configDir) -eq $false) {
    mkdir $configDir
}
if ((Test-Path $palDir) -eq $false) {
    mkdir $palDir
}

# Enumerate through all files, running cleanupCUtilMacrosInFile and then a Set-Content to put into new file
foreach ($file in $incfiles) {
    $outputFile = Join-Path $incDir $file.name
    Write-Verbose ("Processing file {0} and outputting to {1}" -f $file.fullname, $outputFile)
    Set-Content -Path $outputFile (cleanupCUtilMacrosInFile $file.fullname)
}
foreach ($file in $configfiles) {
    $outputFile = Join-Path $configDir $file.name
    Write-Verbose ("Processing file {0} and outputting to {1}" -f $file.fullname, $outputFile)
    Set-Content -Path $outputFile (cleanupCUtilMacrosInFile $file.fullname)
}
foreach ($file in $palfiles) {
    $outputFile = Join-Path $palDir $file.name
    Write-Verbose ("Processing file {0} and outputting to {1}" -f $file.fullname, $outputFile)
    Set-Content -Path $outputFile (cleanupCUtilMacrosInFile $file.fullname)
}

Push-Location "docs/"

doxygen Doxyfile

Pop-Location

Remove-Item .\doxy -Force -Recurse

Pop-Location
