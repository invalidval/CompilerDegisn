program test;
type
  point = record
    x, y: integer
  end;
var
  p1, p2: point;

function addPoints(a, b: point): integer;
begin
  addPoints := a.x + a.y + b.x + b.y
end;

begin
  p1.x := 10;
  p1.y := 20;
  p2.x := 30;
  p2.y := 40;
  write(addPoints(p1, p2))
end.
