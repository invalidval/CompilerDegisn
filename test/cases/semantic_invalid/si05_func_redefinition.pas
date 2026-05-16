program si05_func_redefinition;

function f(x: integer): integer;
begin
    f := x
end;

function f(x: integer): integer;
begin
    f := x + 1
end;

begin
    write(f(1))
end.
