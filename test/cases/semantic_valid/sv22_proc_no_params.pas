program sv22_proc_no_params;
var
    counter: integer;

procedure increment;
begin
    counter := counter + 1
end;

begin
    counter := 0;
    increment;
    increment
end.
