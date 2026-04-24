program test;
type
  person = record
    age: integer;
    score: real
  end;
var
  people: array[1..3] of person;
  i: integer;

begin
  people[1].age := 20;
  people[1].score := 85.5;
  people[2].age := 25;
  people[2].score := 90.0;
  people[3].age := 30;
  people[3].score := 95.5;

  for i := 1 to 3 do
  begin
    write(people[i].age)
  end
end.
