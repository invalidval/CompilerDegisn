program gcd_demo;
var a, b: integer;

function gcd(x, y: integer): integer;
begin
  if y = 0 then
    gcd := x
  else
    gcd := gcd(y, x mod y)
end;

begin
  a := 12;
  b := 18;
  write(gcd(a, b));
end.
