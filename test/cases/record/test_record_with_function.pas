program test;
type
  person = record
    age: integer;
    score: real
  end;
var
  p: person;
  maxAge: integer;

function getMax(a, b: integer): integer;
begin
  if a > b then
    getMax := a
  else
    getMax := b
end;

begin
  p.age := 25;
  p.score := 95.5;
  maxAge := getMax(p.age, 30);
  write(maxAge)
end.
