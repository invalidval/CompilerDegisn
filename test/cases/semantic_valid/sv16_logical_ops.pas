program sv16_logical_ops;
var
    p, q, r: boolean;
begin
    p := true;
    q := false;
    r := p and q;
    r := p or q;
    r := (p and q) or (not p);
    r := not (p and q)
end.
