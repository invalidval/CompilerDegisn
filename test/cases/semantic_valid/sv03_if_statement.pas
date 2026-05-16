program sv03_if_statement;
var
    a, b, c: integer;
    flag: boolean;
begin
    a := 10;
    b := 20;
    flag := a < b;
    if flag then
        c := 1
    else
        c := 0;
    if a > b then
        c := 2;
    if not flag then
        c := 3
end.
