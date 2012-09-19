program mngview;

uses
  Forms,
  Main in 'Main.pas' {MainForm},
  libmng in '..\libmng.pas';

{$R *.RES}

begin
  Application.Initialize;
  Application.CreateForm(TMainForm, MainForm);
  Application.Run;
end.
