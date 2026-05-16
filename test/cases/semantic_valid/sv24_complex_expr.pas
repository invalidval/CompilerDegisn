program sv24_complex_expr;
var
    a, b, c, d: integer;
    r: real;
    flag: boolean;
begin
    a := 1;
    b := 2;
    c := 3;
    d := 4;
    r := (a + b) * (c - d);
    d := a * b + c * d - a;
    flag := (a < b) and (c > d);
    r := a * b / c;
    d := a + b * (c - d) div 2
end.
