program si03_param_redefinition;
var
    a: integer;

procedure test(x: integer; x: boolean);
begin
    a := x
end;

begin
    a := 0;
    test(1, true)
end.
