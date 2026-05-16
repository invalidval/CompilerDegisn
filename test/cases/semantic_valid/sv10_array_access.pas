program sv10_array_access;
var
    arr: array[1..10] of integer;
    i: integer;
begin
    arr[1] := 5;
    arr[5] := 10;
    i := arr[1] + arr[5]
end.
