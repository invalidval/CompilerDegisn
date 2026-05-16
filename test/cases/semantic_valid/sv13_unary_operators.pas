program sv13_unary_operators;
var
    a, b: integer;
    x: real;
    flag: boolean;
begin
    a := -1;
    b := -a;
    x := -3.14;
    flag := true;
    flag := not flag;
    a := not 0;
    b := not a
end.
