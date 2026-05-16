program si15_arg_count_mismatch;
var
    a: integer;

procedure add(x, y: integer);
begin
    a := x + y
end;

begin
    a := 0;
    add(1)
end.
