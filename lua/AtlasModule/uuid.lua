premake.override(_G, "externalproject", function(base, name)
    base(name)
    uuid(os.uuid(name))
end)