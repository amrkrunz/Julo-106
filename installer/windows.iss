[Setup]
AppName=J-106 Synth
AppVersion=1.0.0
AppPublisher=J106 Project
DefaultDirName={commoncf64}\VST3
DisableDirPage=yes
DisableProgramGroupPage=yes
CreateAppDir=no
PrivilegesRequired=admin
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
OutputDir=..\dist
OutputBaseFilename=J-106-Setup-Windows
Compression=lzma2
SolidCompression=yes

[Files]
Source: "..\stage\J-106.vst3\*"; DestDir: "{commoncf64}\VST3\J-106.vst3"; Flags: recursesubdirs createallsubdirs ignoreversion
