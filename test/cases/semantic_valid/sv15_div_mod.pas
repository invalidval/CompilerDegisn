program sv15_div_mod;
var
    a, b, q, r, p: integer;
begin
    a := 17;
    b := 5;
    q := a div b;
    r := a mod b;
    p := q * b + r
end.
