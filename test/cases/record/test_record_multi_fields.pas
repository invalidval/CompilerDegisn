program test;
type
  point = record
    x, y, z: integer
  end;
var
  p: point;

begin
  p.x := 1;
  p.y := 2;
  p.z := 3;
  write(p.x);
  write(p.y);
  write(p.z)
end.
