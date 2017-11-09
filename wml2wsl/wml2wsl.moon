-- moon = require "moon"
lfs  = require "lfs"
-- lapp = require "pl.lapp"
path = require "pl.path"

-- args = lapp [[
-- Usage: wml2wsl SOURCE DEST
-- Transcompile Wesnoth's WML at SOURCE into Wesmere's WSL at DEST.
-- ]]

options =
    input_pattern: "(.*)%.cfg$"
    in_dir:  "../attic/data"
    out_dir: "../data"

lfs.rmdir(options.out_dir)
lfs.mkdir(options.out_dir)

in_dir  = options.in_dir
out_dir = options.out_dir

compile = (require "wml_to_wsl.init").transcompile_file

handle_dir = (dir) ->

    in_dir_path  = path.join(in_dir, dir)
    out_dir_path = out_dir .. "/" .. dir

    lfs.mkdir(out_dir_path)

    for node in lfs.dir(in_dir_path)
        local mode, node_path
        switch node
            when "..", "."
                continue

        match = node\match options.input_pattern

        node_path = path.join(in_dir_path, node)
        mode = lfs.attributes(node_path).mode

        switch mode
            when "file"
                match = node\match options.input_pattern
                if match
                    target_path = path.join(out_dir_path, "#{match}.wsl")
                    status, err = pcall(compile, node_path, target_path)
                    unless status
                        print "#{node_path}: #{err}"
            when "directory"
                handle_dir(path.join(dir, node))

handle_dir("")
