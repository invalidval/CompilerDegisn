program test;
type
  person = record
    age: integer;
    score: real
  end;
var
  p: person;
begin
  p.age := 25;
  write(p.age)
end.
