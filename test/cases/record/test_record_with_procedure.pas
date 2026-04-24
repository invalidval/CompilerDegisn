program test;
type
  point = record
    x, y: integer
  end;
var
  p: point;

procedure resetPoint;
begin
  p.x := 0;
  p.y := 0
end;

begin
  p.x := 10;
  p.y := 20;
  write(p.x);
  write(p.y);

  resetPoint;

  write(p.x);
  write(p.y)
end.
