program sv21_arr_nonzero_start;
var
    arr: array[3..9] of integer;
    i, sum: integer;
begin
    arr[3] := 1;
    arr[9] := 7;
    sum := 0;
    for i := 3 to 9 do
        sum := sum + arr[i]
end.
