program sv07_proc_call;
var
    a, b: integer;

procedure swap(var x, y: integer);
var
    tmp: integer;
begin
    tmp := x;
    x := y;
    y := tmp
end;

begin
    a := 1;
    b := 2;
    swap(a, b)
end.
