#define MyAppName "GIMX Reborn-ffb"
#define MyAppVersion "8.1"
#define MyAppPublisher "GIMX Reborn-ffb"


[Setup]
AppId={{9b9d1546-7cad-45dc-a6c9-fc30c8f6ae10}
AppName=GIMX Reborn
AppVersion=8.1
AppVerName=GIMX Reborn 8.1
AppPublisher=GIMX Reborn

DefaultDirName={autopf}\GIMX Reborn-ffb
DefaultGroupName=GIMX Reborn-ffb
DisableProgramGroupPage=yes
PrivilegesRequired=admin

ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

OutputDir=Output
OutputBaseFilename=GIMX-Reborn-ffb-8.1-x64-Setup

Compression=lzma2
SolidCompression=yes
WizardStyle=modern
SetupLogging=yes
RestartIfNeededByRun=no

UninstallDisplayName=GIMX Reborn-ffb 8.1
UninstallDisplayIcon={app}\gimx-launcher.exe

[Tasks]
Name: "desktopicon"; Description: "Créer un raccourci sur le Bureau"; GroupDescription: "Raccourcis supplémentaires :"; Flags: unchecked

[Files]
Source: "setup\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\GIMX Reborn-ffb"; Filename: "{app}\gimx-launcher.exe"; WorkingDir: "{app}"
Name: "{group}\GIMX Config"; Filename: "{app}\gimx-config.exe"; WorkingDir: "{app}"
Name: "{group}\GIMX FPS Config"; Filename: "{app}\gimx-fpsconfig.exe"; WorkingDir: "{app}"
Name: "{group}\Désinstaller GIMX Reborn"; Filename: "{uninstallexe}"

Name: "{autodesktop}\GIMX Reborn-ffb"; Filename: "{app}\gimx-launcher.exe"; WorkingDir: "{app}"; Tasks: desktopicon

[Registry]
Root: HKCU; Subkey: "System\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\OEM\VID_046D&PID_C260"; ValueType: string; ValueName: "OEMName"; ValueData: "Logitech G HUB G29 Driving Force Racing Wheel (PS4)"

[Run]
Filename: "{app}\gimx-launcher.exe"; Description: "Lancer GIMX Reborn-ffb"; Flags: postinstall nowait skipifsilent
