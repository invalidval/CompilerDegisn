program example(input, output);
const t = 's'; a = 1e6; {必须先声明const再声明var}
var x, y: integer; z : array [1..10, 2..8] of integer; u : integer;
function gcd(a, b: integer): integer;
begin
if b=0 then gcd:=a
else gcd:=gcd(b, a mod b)
end;
begin
x := 2+1;
z[2,2] := 5;
u := 1;
read(x, y);
write(gcd(x, y))
end.