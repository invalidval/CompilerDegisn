program test;
type
  person = record
    age: integer;
    score: real
  end;
var
  p: person;
  result: integer;

function getAge(p: person): integer;
begin
  getAge := p.age
end;

begin
  p.age := 25;
  p.score := 95.5;
  result := getAge(p);
  write(result)
end.
