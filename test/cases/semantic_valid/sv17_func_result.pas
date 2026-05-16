program sv17_func_result;
var
    n, result: integer;

function factorial(x: integer): integer;
begin
    if x <= 1 then
        factorial := 1
    else
        factorial := x * factorial(x - 1)
end;

begin
    n := 5;
    result := factorial(n)
end.
