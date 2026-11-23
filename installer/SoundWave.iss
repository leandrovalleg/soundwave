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
; SoundWave look: dot-grid artwork (one file per Windows scaling level) + colours set in [Code]
WizardImageFile=art\wizard-164x314.bmp,art\wizard-246x459.bmp,art\wizard-328x628.bmp,art\wizard-410x797.bmp
WizardSmallImageFile=art\small-55x55.bmp,art\small-83x80.bmp,art\small-110x106.bmp,art\small-138x140.bmp
WizardImageStretch=no
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

[Code]
// SoundWave colours (Inno Setup colours are written $BBGGRR): light grey ground, white pages, black text, red accent
procedure InitializeWizard;
begin
  WizardForm.Color := $EFEFEF;
  WizardForm.MainPanel.Color := $FFFFFF;
  WizardForm.InnerPage.Color := $FFFFFF;
  WizardForm.WelcomePage.Color := $EFEFEF;
  WizardForm.FinishedPage.Color := $EFEFEF;
  WizardForm.PageNameLabel.Font.Color := $111111;
  WizardForm.PageDescriptionLabel.Font.Color := $8A8A8A;
  WizardForm.WelcomeLabel1.Font.Color := $2119D7;
  WizardForm.FinishedHeadingLabel.Font.Color := $2119D7;
end;
