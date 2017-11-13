wml2wsl = require"wml_to_wsl/init"
describe = describe

dir = require"pl.dir"
file = require"pl.file"

options =
    in_dir:  "spec/inputs"
    out_dir: "spec/outputs"
    input_pattern: "*.cfg"
    old_one: "(.*)%.cfg$"
    output_ext: ".moon"
    diff:
        tool: "git diff --no-index --color" --color-words"
    lint:
        tool: "moonc -l"

lint_file = (file_path) ->
    return io.popen(options.lint.tool .. " #{file_path}")

diff_file = (a_fname, b_fname) ->
    out = io.popen(options.diff.tool .. " " .. a_fname .. " " .. b_fname, "r")\read "*a"
    if options.diff.filter
        out = options.diff.filter out
    out

inputs = for input_file in *dir.getallfiles(options.in_dir, options.input_pattern)
    input_file

path = require"pl.path"
abs_in_dir = path.abspath(options.in_dir)

describe "input tests", ->

    for name in *inputs

        relpath = path.relpath(path.abspath(name), abs_in_dir)
        file = require"pl.file"

        it "#" .. relpath, ->
            tmp_file_path = os.tmpname!

            match = relpath\match options.old_one
            wml2wsl.transcompile_file(name, tmp_file_path)

            out_file_path = options.out_dir .. "/" .. match .. ".moon"
            -- lint_out = lint_file(out_file_path)

            -- print lint_out


            diff_out = diff_file(tmp_file_path, out_file_path)

            if diff_out != ''
                --print diff_out
                assert(false, diff_out)
            else
                -- lint_file
                file.delete(tmp_file_path)
