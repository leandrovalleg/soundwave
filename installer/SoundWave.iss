; SoundWave installer (Inno Setup). Built in CI from dist/.
[Setup]
AppName=SoundWave
AppVersion=0.1.0
AppPublisher=SoundWave
DefaultDirName={autopf}\SoundWave
DefaultGroupName=SoundWave
OutputBaseFilename=SoundWave-Setup
SetupIconFile=..\brand\icon.ico
UninstallDisplayIcon={app}\SoundWave.exe
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
WizardStyle=modern
LicenseFile=..\LICENSE
Compression=lzma2
SolidCompression=yes

[Languages]
Name: "es"; MessagesFile: "compiler:Languages\Spanish.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"
Name: "vst3"; Description: "Instalar los plugins VST3 de SoundWave (también para Ableton y otros DAW)"

[Files]
Source: "..\dist\SoundWave\SoundWave.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\dist\VST3\*"; DestDir: "{commoncf64}\VST3"; Flags: ignoreversion recursesubdirs createallsubdirs; Tasks: vst3
Source: "..\LICENSE"; DestDir: "{app}"
Source: "..\dist\LAME-LICENSE.txt"; DestDir: "{app}"; Flags: skipifsourcedoesntexist

[Icons]
Name: "{group}\SoundWave"; Filename: "{app}\SoundWave.exe"
Name: "{autodesktop}\SoundWave"; Filename: "{app}\SoundWave.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\SoundWave.exe"; Description: "{cm:LaunchProgram,SoundWave}"; Flags: nowait postinstall skipifsilent
