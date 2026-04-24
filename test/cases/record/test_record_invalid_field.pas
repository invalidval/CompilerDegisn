program test;
type
  person = record
    age: integer;
    score: real
  end;
var
  p: person;

begin
  p.name := 'John';  { Error: field 'name' does not exist }
  write(p.age)
end.
