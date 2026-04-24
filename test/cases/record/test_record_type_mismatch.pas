program test;
type
  person = record
    age: integer;
    score: real
  end;
var
  p: person;

begin
  p.age := 25.5;  { Error: type mismatch, age is integer }
  write(p.age)
end.
