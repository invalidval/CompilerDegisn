program test;
type
  counter = record
    value: integer;
    step: integer
  end;
var
  c: counter;
  i: integer;

begin
  c.value := 0;
  c.step := 5;

  for i := 1 to 10 do
  begin
    c.value := c.value + c.step;
    write(c.value)
  end
end.
