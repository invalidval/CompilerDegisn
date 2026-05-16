program sv26_array_multidim;
var
    matrix: array[1..3, 1..3] of integer;
    i, j, sum: integer;
begin
    for i := 1 to 3 do
        for j := 1 to 3 do
            matrix[i, j] := i * j;
    sum := 0;
    for i := 1 to 3 do
        sum := sum + matrix[i, i]
end.
