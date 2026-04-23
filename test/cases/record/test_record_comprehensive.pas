program test;
type
  person = record
    age: integer;
    score: real;
    initial: char
  end;
var
  p1, p2: person;
begin
  p1.age := 25;
  p1.score := 95.5;
  p1.initial := 'A';

  p2.age := 30;
  p2.score := 88.0;
  p2.initial := 'B';

  write(p1.age);
  write(p1.score);
  write(p2.age);
  write(p2.score)
end.
