program si16_arg_type_mismatch;
var
    a: integer;

procedure setval(x: integer);
begin
    a := x
end;

begin
    a := 0;
    setval(true)
end.
