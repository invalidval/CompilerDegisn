program sv12_break_in_for;
var
    i, found: integer;
begin
    found := 0;
    for i := 1 to 100 do
    begin
        if i = 50 then
        begin
            found := i;
            break
        end
    end
end.
