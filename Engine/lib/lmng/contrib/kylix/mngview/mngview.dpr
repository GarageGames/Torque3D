program mngview;

uses
  QForms,
  Main in 'Main.pas' {MainForm},
  libmng in '../libmng.pas';

{$E .bin}

{$R *.res}

begin
  Application.Initialize;
  Application.Title := 'mngview - libmng test-viewer in Kylix';
  Application.CreateForm(TMainForm, MainForm);
  Application.Run;
end.
