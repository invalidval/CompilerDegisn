program sv06_for_downto_loop;
var
    i, product: integer;
begin
    product := 1;
    for i := 10 downto 1 do
        product := product * i
end.
