; GIMX Reborn-ffb 8.1 - Script Inno Setup 7 (x64)

#define MyAppName "GIMX Reborn-ffb"
#define MyAppId "{9b9d1546-7cad-45dc-a6c9-fc30c8f6ae10}"
#define MyAppVersion "8.1"
#define MyAppPublisher "GIMX Reborn-ffb"
#define MyAppURL "https://gimx.fr"

#define MyApp1 "GIMX Reborn-ffb"
#define MyAppExeName1 "gimx-launcher.exe"
#define MyApp2 "GIMX Config"
#define MyAppExeName2 "gimx-config.exe"
#define MyApp3 "GIMX FPS Config"
#define MyAppExeName3 "gimx-fpsconfig.exe"

; Versions et identifiants pilotes optionnels (dossier tools\)
#ifndef UsbdkVersion
  #define UsbdkVersion "1.0.22"
#endif
#ifndef UsbdkAppIdx64
  #define UsbdkAppIdx64 "{6D4A6ED0-CF41-4615-A4B3-BDA018C3C1CD}"
#endif
#ifndef SilabsCP210xAppId
  #define SilabsCP210xAppId "C9C3E5CCB43EEF685DD0E2BB4263DDC88C9B3834"
#endif

[Setup]
AppId={{#MyAppId}
AppName=GIMX Reborn
AppVersion={#MyAppVersion}
AppVerName=GIMX Reborn {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}

DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
PrivilegesRequired=admin

ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

OutputDir=Output
OutputBaseFilename=GIMX-Reborn-ffb-{#MyAppVersion}-x64-Setup

Compression=lzma2
SolidCompression=yes
WizardStyle=modern
SetupLogging=yes
RestartIfNeededByRun=no
#if FileExists("LICENSE")
LicenseFile=LICENSE
#endif

UninstallDisplayName={#MyAppName} {#MyAppVersion}
UninstallDisplayIcon={app}\{#MyAppExeName1}

[Languages]
#if FileExists(CompilerPath + "Languages\French.isl")
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
#endif
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "setup\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "tools\drivers\*"; DestDir: "{app}\tools\drivers\"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist
Source: "tools\CP210x_VCP_Windows\*"; Excludes: "*x86*"; DestDir: "{app}\tools\CP210x_VCP_Windows"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist
Source: "tools\usbdk\UsbDk_{#UsbdkVersion}_x64.msi"; DestDir: "{app}\tools\usbdk\"; Flags: ignoreversion skipifsourcedoesntexist

[Icons]
Name: "{group}\{#MyApp1}"; Filename: "{app}\{#MyAppExeName1}"; WorkingDir: "{app}"
Name: "{group}\{#MyApp2}"; Filename: "{app}\{#MyAppExeName2}"; WorkingDir: "{app}"
Name: "{group}\{#MyApp3}"; Filename: "{app}\{#MyAppExeName3}"; WorkingDir: "{app}"
Name: "{group}\Désinstaller GIMX Reborn"; Filename: "{uninstallexe}"

Name: "{autodesktop}\{#MyApp1}"; Filename: "{app}\{#MyAppExeName1}"; WorkingDir: "{app}"; Tasks: desktopicon
Name: "{autodesktop}\{#MyApp2}"; Filename: "{app}\{#MyAppExeName2}"; WorkingDir: "{app}"; Tasks: desktopicon
Name: "{autodesktop}\{#MyApp3}"; Filename: "{app}\{#MyAppExeName3}"; WorkingDir: "{app}"; Tasks: desktopicon

[Registry]
; Noms OEM DirectInput / joy.cpl (HKLM pour compatibilité PrivilegesRequired=admin)
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\OEM\VID_046D&PID_C202"; ValueType: string; ValueName: "OEMName"; ValueData: "Logitech WingMan Formula (Yellow) (USB)"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\OEM\VID_046D&PID_C20E"; ValueType: string; ValueName: "OEMName"; ValueData: "Logitech WingMan Formula GP"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\OEM\VID_046D&PID_C293"; ValueType: string; ValueName: "OEMName"; ValueData: "Logitech WingMan Formula Force GP USB"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\OEM\VID_046D&PID_C294"; ValueType: string; ValueName: "OEMName"; ValueData: "Logitech Driving Force USB"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\OEM\VID_046D&PID_C295"; ValueType: string; ValueName: "OEMName"; ValueData: "Logitech MOMO Force USB"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\OEM\VID_046D&PID_C298"; ValueType: string; ValueName: "OEMName"; ValueData: "Logitech Driving Force Pro USB"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\OEM\VID_046D&PID_C299"; ValueType: string; ValueName: "OEMName"; ValueData: "Logitech G25 Racing Wheel USB"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\OEM\VID_046D&PID_C29A"; ValueType: string; ValueName: "OEMName"; ValueData: "Logitech Driving Force GT USB"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\OEM\VID_046D&PID_C29B"; ValueType: string; ValueName: "OEMName"; ValueData: "Logitech G27 Racing Wheel USB"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\OEM\VID_046D&PID_C24F"; ValueType: string; ValueName: "OEMName"; ValueData: "Logitech G29 Driving Force Racing Wheel USB"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\OEM\VID_046D&PID_C260"; ValueType: string; ValueName: "OEMName"; ValueData: "Logitech G HUB G29 Driving Force Racing Wheel (PS4)"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\OEM\VID_046D&PID_CA03"; ValueType: string; ValueName: "OEMName"; ValueData: "Logitech MOMO Racing USB"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\OEM\VID_046D&PID_CA04"; ValueType: string; ValueName: "OEMName"; ValueData: "Logitech Racing Wheel USB"; Flags: uninsdeletekey

[Run]
Filename: "msiexec.exe"; Parameters: "/i ""{app}\tools\usbdk\UsbDk_{#UsbdkVersion}_x64.msi"" /qn"; Check: ShouldInstallUsbDk; Flags: runascurrentuser runhidden
Filename: "{app}\tools\CP210x_VCP_Windows\CP210xVCPInstaller_x64.exe"; Parameters: "/Q /SE"; Check: ShouldInstallCP210x; Flags: runascurrentuser runhidden
Filename: "{sys}\pnputil.exe"; Parameters: "-i -a arduino.inf"; WorkingDir: "{app}\tools\drivers"; Check: HasArduinoDriver; Flags: runascurrentuser runhidden 64bit
Filename: "{app}\{#MyAppExeName1}"; Description: "Lancer {#MyAppName}"; Flags: postinstall nowait skipifsilent

[Code]
function GetNumber(var temp: String): Integer;
var
  part: String;
  pos1: Integer;
begin
  if Length(temp) = 0 then
  begin
    Result := -1;
    Exit;
  end;
  pos1 := Pos('.', temp);
  if (pos1 = 0) then
  begin
    Result := StrToIntDef(temp, 0);
    temp := '';
  end
  else
  begin
    part := Copy(temp, 1, pos1 - 1);
    temp := Copy(temp, pos1 + 1, Length(temp));
    Result := StrToIntDef(part, 0);
  end;
end;

function CompareInner(var temp1, temp2: String): Integer;
var
  num1, num2: Integer;
begin
  num1 := GetNumber(temp1);
  num2 := GetNumber(temp2);
  if (num1 = -1) or (num2 = -1) then
  begin
    Result := 0;
    Exit;
  end;
  if (num1 > num2) then
    Result := 1
  else if (num1 < num2) then
    Result := -1
  else
    Result := CompareInner(temp1, temp2);
end;

function CompareVersion(str1, str2: String): Integer;
var
  temp1, temp2: String;
begin
  temp1 := str1;
  temp2 := str2;
  Result := CompareInner(temp1, temp2);
end;

function GetHKLM(isNative: Boolean): Integer;
begin
  if isNative then
  begin
    if IsWin64 then
      Result := HKLM64
    else
      Result := HKLM32;
  end
  else
    Result := HKLM;
end;

function GetUninstallRegKey(appId: String; isInno: Boolean): String;
begin
  Result := 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\' + appId;
  if isInno then
    Result := Result + '_is1';
end;

function AppInstalled(isNative, isInno: Boolean; appId: String): Boolean;
begin
  Result := RegKeyExists(GetHKLM(isNative), GetUninstallRegKey(appId, isInno));
end;

function ShouldInstallUsbDk(): Boolean;
begin
  Result := IsWin64 and
            FileExists(ExpandConstant('{app}\tools\usbdk\UsbDk_{#UsbdkVersion}_x64.msi')) and
            not AppInstalled(True, False, '{#UsbdkAppIdx64}');
end;

function ShouldInstallCP210x(): Boolean;
begin
  Result := IsWin64 and
            FileExists(ExpandConstant('{app}\tools\CP210x_VCP_Windows\CP210xVCPInstaller_x64.exe')) and
            not AppInstalled(True, False, '{#SilabsCP210xAppId}');
end;

function HasArduinoDriver(): Boolean;
begin
  Result := IsWin64 and
            FileExists(ExpandConstant('{app}\tools\drivers\arduino.inf'));
end;

function InitializeSetup(): Boolean;
var
  oldVersion: String;
  uninstaller: String;
  ErrorCode: Integer;
  compareResult: Integer;
  appId: String;
  appName: String;
  appVersion: String;
begin
  appId := '{#MyAppId}';
  appName := '{#MyAppName}';
  appVersion := '{#MyAppVersion}';

  if not AppInstalled(False, True, appId) then
  begin
    Result := True;
    Exit;
  end;

  oldVersion := '';
  RegQueryStringValue(GetHKLM(False), GetUninstallRegKey(appId, True), 'DisplayVersion', oldVersion);
  compareResult := CompareVersion(oldVersion, appVersion);
  if (compareResult = 0) then
  begin
    if (MsgBox(appName + ' ' + oldVersion + ' est déjà installé. Voulez-vous le réparer maintenant ?',
      mbConfirmation, MB_YESNO) = IDNO) then
    begin
      Result := False;
      Exit;
    end;
  end
  else
  begin
    if (MsgBox(appName + ' ' + oldVersion + ' est installé. Voulez-vous le mettre à jour vers ' + appVersion + ' ?',
      mbConfirmation, MB_YESNO) = IDNO) then
    begin
      Result := False;
      Exit;
    end;
  end;

  if RegQueryStringValue(GetHKLM(False), GetUninstallRegKey(appId, True), 'UninstallString', uninstaller) then
  begin
    uninstaller := RemoveQuotes(uninstaller);
    if not ShellExec('runas', uninstaller, '/SILENT', '', SW_HIDE, ewWaitUntilTerminated, ErrorCode) or (ErrorCode <> 0) then
    begin
      MsgBox('Échec de la désinstallation de ' + appName + ' ' + oldVersion + '. Redémarrez Windows et relancez l''installateur.',
        mbError, MB_OK);
      Result := False;
      Exit;
    end;
  end;

  Result := True;
end;

function FileReplaceDevices(const FileName: string): Boolean;
var
  MyFile: TStrings;
  MyText: string;
  WriteFile: Boolean;
begin
  MyFile := TStringList.Create;
  try
    Result := True;
    try
      MyFile.LoadFromFile(FileName);
      MyText := MyFile.Text;
      WriteFile := False;

      // Normalisation des noms de volants branchés avant installation de Logitech Gaming Software
      WriteFile := WriteFile or (StringChangeEx(MyText, '"WingMan Formula"',            '"Logitech WingMan Formula (Yellow) (USB)"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"WingMan Formula GP"',         '"Logitech WingMan Formula GP"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"WingMan Formula Force GP"',   '"Logitech WingMan Formula Force GP USB"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"Logitech Driving Force"',     '"Logitech Driving Force USB"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"Logitech Driving Force EX"',  '"Logitech Driving Force USB"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"Logitech Formula Force EX"',  '"Logitech Driving Force USB"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"Logitech Driving Force RX"',  '"Logitech Driving Force USB"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"Logitech MOMO Force"',        '"Logitech MOMO Force USB"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"Logitech Driving Force Pro"', '"Logitech Driving Force Pro USB"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"G25 Racing Wheel"',           '"Logitech G25 Racing Wheel USB"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"Driving Force GT"',           '"Logitech Driving Force GT USB"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"G27 Racing Wheel"',           '"Logitech G27 Racing Wheel USB"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"G29 Driving Force Racing Wheel"', '"Logitech G29 Driving Force Racing Wheel USB"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"Logitech MOMO Racing "',      '"Logitech MOMO Racing USB"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"Logitech Racing Wheel"',      '"Logitech Racing Wheel USB"', True) > 0);

      // Normalisation des noms japonais (Logicool)
      WriteFile := WriteFile or (StringChangeEx(MyText, '"Logicool WingMan Formula (Yellow) (USB)"', '"Logitech WingMan Formula (Yellow) (USB)"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"Logicool WingMan Formula GP"',             '"Logitech WingMan Formula GP"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"Logicool WingMan Formula Force GP USB"',   '"Logitech WingMan Formula Force GP USB"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"Logicool Driving Force EX USB"',           '"Logitech Driving Force EX USB"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"Logicool Formula Force EX USB"',           '"Logitech Formula Force EX USB"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"Logicool Driving Force RX USB"',           '"Logitech Driving Force RX USB"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"Logicool Driving Force USB"',              '"Logitech Driving Force USB"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"Logicool MOMO Force USB"',                 '"Logitech MOMO Force USB"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"Logicool Driving Force Pro USB"',          '"Logitech Driving Force Pro USB"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"Logicool G25 Racing Wheel USB"',           '"Logitech G25 Racing Wheel USB"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"Logicool Driving Force GT USB"',           '"Logitech Driving Force GT USB"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"Logicool G27 Racing Wheel USB"',           '"Logitech G27 Racing Wheel USB"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"Logicool G29 Driving Force Racing Wheel USB"', '"Logitech G29 Driving Force Racing Wheel USB"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"Logicool MOMO Racing USB"',                '"Logitech MOMO Racing USB"', True) > 0);
      WriteFile := WriteFile or (StringChangeEx(MyText, '"Logicool Racing Wheel USB"',               '"Logitech Racing Wheel USB"', True) > 0);

      if WriteFile then
      begin
        MyFile.Text := MyText;
        MyFile.SaveToFile(FileName);
      end;
    except
      Result := False;
    end;
  finally
    MyFile.Free;
  end;
end;

procedure ListFiles(const Directory: string; Files: TStringList);
var
  FindRec: TFindRec;
begin
  Files.Clear;
  if FindFirst(Directory + '*.xml', FindRec) then
  try
    repeat
      if (FindRec.Attributes and FILE_ATTRIBUTE_DIRECTORY) = 0 then
        Files.Add(FindRec.Name);
    until not FindNext(FindRec);
  finally
    FindClose(FindRec);
  end;
end;

procedure FixConfigWheelNames();
var
  MyDir: string;
  MyFiles: TStringList;
  i: Integer;
begin
  MyDir := ExpandConstant('{userappdata}') + '\gimx\config\';
  if not DirExists(MyDir) then
    Exit;

  MyFiles := TStringList.Create;
  try
    ListFiles(MyDir, MyFiles);
    for i := 0 to MyFiles.Count - 1 do
      FileReplaceDevices(MyDir + MyFiles[i]);
  finally
    MyFiles.Free;
  end;
end;

procedure DisableLogitechCombinedPedals();
var
  BaseKey: string;
  Profiles: array[0..13] of string;
  i: Integer;
begin
  BaseKey := 'Software\Logitech\Gaming Software\GlobalDeviceSettings\';
  Profiles[0]  := 'WingManFormula2';
  Profiles[1]  := 'WingManFormulaGT';
  Profiles[2]  := 'WingManFormulaGP';
  Profiles[3]  := 'MomoForce';
  Profiles[4]  := 'MOMORacing';
  Profiles[5]  := 'DrivingForce';
  Profiles[6]  := 'RacingWheel';
  Profiles[7]  := 'DrivingForcePro';
  Profiles[8]  := 'DrivingForceEX';
  Profiles[9]  := 'FormulaForceRX';
  Profiles[10] := 'DrivingForceRX';
  Profiles[11] := 'G25';
  Profiles[12] := 'G27';
  Profiles[13] := 'DrivingForceGT';

  for i := 0 to 13 do
    RegWriteDWordValue(HKEY_CURRENT_USER, BaseKey + Profiles[i], 'CombinedPedalsEnable', 0);
end;

procedure RestartLWEMon();
var
  ErrorCode: Integer;
  LweMonPath: String;
begin
  LweMonPath := ExpandConstant('{pf64}\Logitech\Gaming Software\LWEMon.exe');
  if not FileExists(LweMonPath) then
    LweMonPath := ExpandConstant('{pf32}\Logitech\Gaming Software\LWEMon.exe');

  if FileExists(LweMonPath) then
  begin
    Exec('taskkill.exe', '/f /im LWEMon.exe', '', SW_HIDE, ewWaitUntilTerminated, ErrorCode);
    ShellExecAsOriginalUser('', LweMonPath, '/noui', '', SW_HIDE, ewNoWait, ErrorCode);
  end;
end;

function CreateRestorePoint(sDescription: String): Boolean;
var
  SWbemLocator: Variant;
  WMIService: Variant;
  SystemRestore: Variant;
  ErrCode: Integer;
begin
  try
    WizardForm.StatusLabel.Caption := 'Création du point de restauration système...';
    SWbemLocator := CreateOleObject('WbemScripting.SWbemLocator');
    WMIService := SWbemLocator.ConnectServer('', 'root\default');
    SystemRestore := WMIService.Get('SystemRestore');
    ErrCode := SystemRestore.CreateRestorePoint(sDescription, 0, 100);
    Result := (ErrCode = 0);
  except
    Result := False;
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssInstall then
  begin
    CreateRestorePoint('Installed GIMX Reborn-ffb');
  end
  else if CurStep = ssPostInstall then
  begin
    DisableLogitechCombinedPedals();
    FixConfigWheelNames();
    RestartLWEMon();
  end;
end;
