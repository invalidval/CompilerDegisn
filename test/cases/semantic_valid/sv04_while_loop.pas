program sv04_while_loop;
var
    i, sum: integer;
begin
    i := 1;
    sum := 0;
    while i <= 10 do
    begin
        sum := sum + i;
        i := i + 1
    end
end.
