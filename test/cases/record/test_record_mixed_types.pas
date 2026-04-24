program test;
type
  data = record
    a, b, c: integer;
    x, y: real;
    flag: boolean
  end;
var
  d: data;

begin
  d.a := 1;
  d.b := 2;
  d.c := 3;
  d.x := 1.5;
  d.y := 2.5;
  d.flag := true;

  write(d.a);
  write(d.b);
  write(d.c);
  write(d.x);
  write(d.y)
end.
