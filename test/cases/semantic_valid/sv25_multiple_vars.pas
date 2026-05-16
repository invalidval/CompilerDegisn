program sv25_multiple_vars;
var
    a, b, c, d, e: integer;
    x, y, z: real;
    p, q: boolean;
    ch1, ch2: char;
begin
    a := 1;
    b := a + 2;
    c := b * 3;
    d := c div 4;
    e := d mod 3;
    x := 1.5;
    y := x * 2.0;
    z := x + y;
    p := a < b;
    q := p and (c > d);
    ch1 := 'x';
    ch2 := 'y'
end.
