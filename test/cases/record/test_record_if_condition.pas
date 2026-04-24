program test;
type
  person = record
    age: integer;
    score: real;
    active: boolean
  end;
var
  p: person;

begin
  p.age := 25;
  p.score := 85.5;
  p.active := true;

  if p.active then
  begin
    if p.score > 90.0 then
      write(1)
    else
      write(0)
  end
  else
    write(-1)
end.
