program sv08_func_call;
var
    a, b, result: integer;

function add(x, y: integer): integer;
begin
    add := x + y
end;

begin
    a := 3;
    b := 5;
    result := add(a, b)
end.
