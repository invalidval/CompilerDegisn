program test;
type
  person = record
    age: integer;
    score: real
  end;
var
  people: array[1..3] of person;
  i: integer;
  totalAge: integer;
  avgScore: real;

begin
  people[1].age := 20;
  people[1].score := 80.0;
  people[2].age := 25;
  people[2].score := 90.0;
  people[3].age := 30;
  people[3].score := 100.0;

  totalAge := 0;
  avgScore := 0.0;

  for i := 1 to 3 do
  begin
    totalAge := totalAge + people[i].age;
    avgScore := avgScore + people[i].score
  end;

  avgScore := avgScore / 3.0;

  write(totalAge)
end.
