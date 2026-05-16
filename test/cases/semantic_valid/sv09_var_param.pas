program sv09_var_param;
var
    x: integer;

procedure inc(var n: integer);
begin
    n := n + 1
end;

begin
    x := 0;
    inc(x)
end.
