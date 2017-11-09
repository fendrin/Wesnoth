compile_line = require"wml_to_wsl.compile"
parse_line   = require"wml_to_wsl.parse"

transcompile_line = (line, parser, compiler, last = false) ->
    return compiler(parser(line, last))

transcompile_file = (input, output) ->

    compiler = compile_line!
    parser   = parse_line!

    input_file  = io.open(input,  "r")
    output_file = io.open(output, "w")

    lineList = {}

    level = 0
    for line in input_file\lines!
        table.insert(lineList, "#{line}")

    input_file\close!

    write_empty = false
    for i, line in ipairs lineList
        code  = nil
        level = nil

        status, code, level, delete_empty_line = pcall(transcompile_line, line, parser, compiler, i == #lineList)
        unless status
            print"#{input}: #{code}"
            output_file\write("err: #{input} - #{code}" .. '\n')
        else
            unless delete_empty_line
                output_file\write('\n') if write_empty
            if code == ""
                write_empty = true
            else
                write_empty = false
                output_file\write(code .. '\n')

    output_file\close!

    if level != 0 and level != nil
        error"Toplevel not reached, #{input} closed at level #{level}"

{
    :transcompile_file
}
