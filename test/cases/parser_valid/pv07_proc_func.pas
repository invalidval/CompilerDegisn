program pv07;

procedure p(x: integer);
begin
  x := x + 1
end;

function f(y: integer): integer;
begin
  f := y
end;

begin
  p(1);
  write(f(2))
end.
