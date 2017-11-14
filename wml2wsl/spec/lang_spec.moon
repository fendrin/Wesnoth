wml2wsl = require"wml_to_wsl/init"

dir  = require"pl.dir"
file = require"pl.file"
path = require"pl.path"

describe = describe
it       = it

options =
    in_dir:  "spec/inputs"
    out_dir: "spec/outputs"
    input_sh_pattern: "*.cfg"
    input_reg_exp: "(.*)%.cfg$"
    output_ext: "moon"
    diff:
        tool: "git diff --no-index --color"


diff_file = (a_fname, b_fname) ->
    unless path.exists(b_fname)
        assert(false, "File not found: #{b_fname}")
    diff_cmd = "#{options.diff.tool} #{a_fname} #{b_fname}"
    return io.popen(diff_cmd, "r")\read"*a"


describe "input tests", ->

    abs_in_dir = path.abspath(options.in_dir)

    inputs = dir.getallfiles(options.in_dir, options.input_sh_pattern)
    for name in *inputs

        relpath = path.relpath(path.abspath(name), abs_in_dir)
        it "##{relpath}", ->
            tmp_file_path = os.tmpname!
            wml2wsl.transcompile_file(name, tmp_file_path)

            match = relpath\match options.input_reg_exp
            out_file_path = "#{options.out_dir}/#{match}.#{options.output_ext}"

            diff_out = diff_file(tmp_file_path, out_file_path)
            if diff_out != ''
                assert(false, diff_out)
            else
                file.delete(tmp_file_path)
