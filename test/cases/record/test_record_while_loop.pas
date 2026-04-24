program test;
type
  point = record
    x, y: integer
  end;
var
  p: point;

begin
  p.x := 0;
  p.y := 0;

  while p.x < 5 do
  begin
    p.x := p.x + 1;
    p.y := p.y + 2;
    write(p.x);
    write(p.y)
  end
end.
