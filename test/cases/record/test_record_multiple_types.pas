program test;
type
  point = record
    x, y: integer
  end;
  rect = record
    width, height: integer
  end;
var
  p: point;
  r: rect;
  area: integer;

begin
  p.x := 10;
  p.y := 20;

  r.width := 30;
  r.height := 40;

  area := r.width * r.height;
  write(p.x);
  write(p.y);
  write(area)
end.
