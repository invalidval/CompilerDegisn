program test;
type
  vector = record
    x, y, z: real
  end;
var
  v1, v2: vector;
  sum: real;

begin
  v1.x := 1.0;
  v1.y := 2.0;
  v1.z := 3.0;

  v2.x := 4.0;
  v2.y := 5.0;
  v2.z := 6.0;

  sum := v1.x + v2.x + v1.y + v2.y + v1.z + v2.z;
  write(sum)
end.
