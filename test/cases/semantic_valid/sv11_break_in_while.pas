program sv11_break_in_while;
var
    i: integer;
begin
    i := 0;
    while i < 100 do
    begin
        i := i + 1;
        if i = 50 then
            break
    end
end.
