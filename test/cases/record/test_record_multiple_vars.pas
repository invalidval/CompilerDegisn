program test;
type
  student = record
    id: integer;
    name: char;
    score: real;
    passed: boolean
  end;
var
  s1, s2, s3: student;

begin
  s1.id := 1;
  s1.name := 'A';
  s1.score := 95.5;
  s1.passed := true;

  s2.id := 2;
  s2.name := 'B';
  s2.score := 88.0;
  s2.passed := true;

  s3.id := 3;
  s3.name := 'C';
  s3.score := 65.5;
  s3.passed := false;

  write(s1.id);
  write(s1.score);
  write(s2.id);
  write(s2.score);
  write(s3.id);
  write(s3.score)
end.
