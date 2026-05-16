program sv23_nested_if;
var
    score: integer;
    grade: integer;
begin
    score := 85;
    if score >= 90 then
        grade := 1
    else
        if score >= 80 then
            grade := 2
        else
            if score >= 70 then
                grade := 3
            else
                grade := 0
end.
