program test;
type
  person = record
    age: integer;
    score: real
  end;
var
  p: person;

procedure modifyPerson(var p: person);
begin
  p.age := 30;
  p.score := 88.0
end;

begin
  p.age := 25;
  p.score := 95.5;
  write(p.age);
  modifyPerson(p);
  write(p.age)
end.
