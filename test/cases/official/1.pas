program example(input, output);
const t = `10086`; a = 1e6; {必须先声明const再声明var}
var x, y: integer; z : array [1..10, 2..8] of integer;
function gcd(a, b: integer): integer;
begin
if b=0 then gcd:=a
else gcd:=gcd(b, a mod b)
end;
begin
z[2,3] := 5;
read(x, y);
write(gcd(x, y))
end.