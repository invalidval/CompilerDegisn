program test;
type
  person = record
    age: integer;
    age: real  { Error: duplicate field name }
  end;
var
  p: person;

begin
  p.age := 25;
  write(p.age)
end.
