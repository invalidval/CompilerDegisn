program si04_proc_redefinition;

procedure greet;
begin
    write(1)
end;

procedure greet;
begin
    write(2)
end;

begin
    greet
end.
