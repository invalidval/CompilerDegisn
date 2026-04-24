program test;
type
  stats = record
    count: integer;
    sum: real;
    avg: real
  end;
var
  s: stats;
  i: integer;

begin
  s.count := 0;
  s.sum := 0.0;

  for i := 1 to 5 do
  begin
    s.count := s.count + 1;
    s.sum := s.sum + i
  end;

  s.avg := s.sum / s.count;

  write(s.count);
  write(s.sum);
  write(s.avg)
end.
