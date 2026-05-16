program sv14_relational_ops;
var
    a, b: integer;
    x, y: real;
    r1, r2, r3, r4, r5, r6, r7: boolean;
begin
    a := 5;
    b := 10;
    x := 3.0;
    y := 5.0;
    r1 := a < b;
    r2 := a > b;
    r3 := a <= b;
    r4 := a >= b;
    r5 := a = b;
    r6 := a <> b;
    r7 := x < y
end.
