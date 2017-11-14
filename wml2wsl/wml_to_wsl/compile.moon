import INDENT, indent_space, ambigTagMap, actionTagMap, actionWrappersMap, actionMacroMap, toplevelMap from require"wml_to_wsl/config"

stringx = require"pl.stringx"
stringx.import!

moon = require"moon"

trim = (s) ->
    return (s\gsub("^%s*(.-)%s*$", "%1"))

build_compiler = ->
    env   = {}
    level = 0
    line_number  = 0

    isSymbol = false
    isInTableContext = false

    tagStack = {"toplevel"}
    popStack = (tagName) ->
        top = tagStack[#tagStack]
        if type(top) == "table"
            if top[1] != tagName
                -- print"Unbalanced WML! #{top[1]}: #{top[2].value} closed by #{tagName} at line #{line_number}"
                error"Unbalanced WML! #{top[1]}: #{top[2].value} closed by #{tagName} at line #{line_number}"
        elseif top != tagName
            -- print"Unbalanced WML! #{top} closed by #{tagName} at line #{line_number}"
            error"Unbalanced WML! #{top} closed by #{tagName} at line #{line_number}"
        table.remove(tagStack)
    isTopStack = (tagName) ->
        if type(tagStack[#tagStack]) == "table"
            return tagName == tagStack[#tagStack][1]
        else
            return tagName == tagStack[#tagStack]

    getTagType = (tagname) ->
        assert(tagname)

        context = tagStack[#tagStack]

        if actionWrappersMap[context]
            return "action"

        if actionWrappersMap[tagname]
            return "wrapper"

        switch context
            when "event", "case"
                if actionTagMap[tagname] or tagname == "event"
                    return "initial"
                else
                    return "key"
            when "toplevel"
                if toplevelMap[tagname]
                    return "action"
                if actionTagMap[tagname]
                    return "function"
                else
                    return "key"
            when "table"
                return "key"

        if ambigTagMap[tagname] and isInTableContext
            return "key"
        if ambigTagMap[tagname] and actionWrappersMap[context]
            return "action"
        if actionTagMap[tagname] and actionWrappersMap[context]
            return "action"
        if toplevelMap[tagname]
            return "action"

        return "key"

    compile_line = (ast) ->
        line_number += 1
        -- assert(ast)
        unless ast
            return "Couldn't parse line", 0
        assert(type(ast) == "table")

        old_level = level
        delete_empty_line = false
        line = ""

        ident = ->
            space = ""

            for i = 1, math.min(level, old_level)
                space ..= indent_space
            return space

        local compile_node

        compile_values = (node, make_list=true) ->
            first = true
            assert(node.type == "valueList")
            res = ""
            res ..= "{" if #node.value > 1 and make_list
            for value in *node.value
                res ..= ", " unless first
                first = false
                res ..= compile_node(value)
            res ..= "}" if #node.value > 1 and make_list
            return res

        compile_attribute = (node) ->
            assert(node.type == "attribute")
            line = ""
            first = true

            if tagStack[#tagStack] == "toplevel"
                line ..= "{"
                for i = 2, INDENT
                    line ..= " "
                table.insert(tagStack, "table")
                level += 1
            -- if isInTableContext
            --line ..= "{"
                -- isInTableContext = true
            if #node.keys == 1
                key_type = node.keys[1].type

                switch key_type
                    when "macro"
                        line ..= "[" .. compile_node(node.keys[1]) .. "]"
                    else
                        line ..= node.keys[1]
                line ..= ": "
                line ..= "{" if #node.value > 1

                line ..= compile_values(node.value)
                -- for value in *node.values
                --     line ..= ", " unless first
                --     first = false
                --     line ..= compile_node(value)
                line ..= "}" if #node.value > 1
            elseif #node.keys == #node.value.value
                for i, key in ipairs node.keys
                    line ..= ", " unless first
                    line ..= key .. ": " .. compile_node(node.value.value[i])
                    first = false
            else
                for i, key in ipairs node.keys
                    line ..= ", " unless first
                    line ..= key .. ": " .. compile_node(node.value.value[1]) .. "[#{i}]"
                    first = false

            return line

        compile_tag_start = (tagName, isAmend=false) ->
            assert(tagName)
            assert(tagName != "")
            res = ""

            tagType = getTagType(tagName)
            if tagType == "initial"
                level += 1
                res ..= "do: ->\n" .. ident! .. indent_space
                delete_empty_line = true
                table.insert(tagStack, "do")

            if tagType == "action" or tagType == "initial"
                first = true
                for part in *tagName\split('_')
                    res ..= '_' unless first
                    first = false
                    res ..= string.upper(string.sub(part, 1, 1)) .. string.sub(part, 2)
            else
                res ..= tagName

            switch tagType
                when "action"
                    res ..= "{"
                    isInTableContext = true
                when "key"
                    res ..= ": {"
                    isInTableContext = true
                when "wrapper"
                    isInTableContext = false
                    if tagName == "event"
                        res ..= ": {"
                    else
                        res ..= ": ->"
                when "initial"
                    isInTableContext = true
                    res ..= "{"
                when "script"
                    res ..= "->"
                when nil
                    error"TagType is nil"
                else
                    error"Unknown Tag Type" .. tagType

            res ..= "\n" .. ident! .. indent_space .. "amend: true" if isAmend

            level += 1
            table.insert(tagStack, tagName)
            return res

        compile_tag_end = (tagName) ->
            tagEnd = ""
            if tagName == "event"
                if tagStack[#tagStack] == 'do'
                    popStack("do")
                    level -= 1
            if tagName == "case"
                level -= 1
                popStack("do")
            level -= 1
            tagEnd ..= "}" unless actionWrappersMap[tagName]
            popStack(tagName)

            return tagEnd

        reduce_indentation = false
        compile_node = (node) ->

            unless node
                return ""
            -- assert(node)

            str = ""
            node_type = node.type
            -- unless node_type
                -- moon.p(node)
            -- assert(node_type)
            node_value = node.value
            switch node_type
                when "tagStart"
                    -- print tagStack[#tagStack][1]
                    if isTopStack"macro"
                        str ..= "{ "
                    str ..= compile_tag_start(node_value)
                when "strangeNumber"
                    str ..= '"' .. node_value .. '"'
                when "imagePath"
                    str ..= '"' .. node.path.value .. node.imageExtension .. '"'
                when "path"
                    str ..= '"' .. node_value .. '"'
                when "URL"
                    str ..= '"' .. node_value .. '"'
                when "emptyString"
                    str ..= ""
                when "id"
                    str ..= node_value\gsub(":", ".")
                when "lua"
                    str ..= "[[#{node_value}"
                    if node.closed
                        str ..= "]]"
                    else
                        table.insert(tagStack, "lua")
                when "valueList"
                    str ..= compile_values(node)
                when "bool"
                    switch node_value
                        when "yes", "true"
                            str ..= "true"
                        when "no", "false"
                            str ..= "false"
                        else
                            error"Unknown Boolean value #{node_value}"
                when "char"
                    str ..= "'" .. node_value .. "'"
                when "operator"
                    switch node_value
                        when "plus"
                            str ..= " + "
                        when "minus"
                            str ..= " - "
                        when "modulo"
                            str ..= " % "
                        when "div"
                            str ..= " / "
                        else
                            error"Unknown Operator: #{node_value}"
                when "Formula"
                    for formula_term in *node.value
                        str ..= compile_node(formula_term)
                when "string"
                    str ..= "_ " if node.translated

                    str_value = node_value\gsub("{", "#" .. "{")
                    str_value = str_value\gsub("%$", "#" .. "{")
                    str_value = str_value\gsub("|", "}")
                    str_value = str_value\gsub('\\', "\\\\")
                    str ..= '"' .. str_value
                    str ..= '"' if node.closed

                when "multiString"
                    str ..= '"' .. node_value
                when "tMultiString"
                    str ..= "_ \"" .. node_value
                when "endMultiString"
                    str ..= node_value unless node_value == '"' -- empty strings match the " ; this is a ugly workaround, fix the parser!
                    str ..= '"'
                when "number"
                    str ..= node_value
                when "variable"
                    --first = true
                    for i, var_frag in ipairs node_value
                        switch var_frag.type
                            when "macro"
                                if i == 1
                                    str ..= "Var(" .. compile_node(var_frag) .. ")"
                                    -- first = false
                                else
                                    str ..= "[" .. compile_node(var_frag) .. "]"
                            when "variableName"
                                str ..= "." unless i == 1
                                str ..= var_frag.value
                            when "arrayAccess"
                                switch var_frag.value.type
                                    when "number"
                                        var_frag.value.value += 1
                                str ..= "[" .. compile_node(var_frag.value) .. "]"
                when "variableName"
                    str ..= '"'
                    for i, var_frag in ipairs node_value
                        switch var_frag.type
                            when "macro"
                                str ..= '#' .. '{' .. compile_node(var_frag) .. "}"
                            when "variableName"
                                str ..= "." unless i == 1
                                str ..= var_frag.value
                            when "arrayAccess"
                                switch var_frag.value.type
                                    when "number"
                                        var_frag.value.value += 1
                                str ..= "[" .. compile_node(var_frag.value) .. "]"
                    str ..= '"'
                when "macro"

                    name = node.name
                    type = name.type

                    uses_exec = false
                    switch type
                        when "macro"
                            str ..= "EXEC("
                            uses_exec = true

                    str ..= compile_node(node.name)
                    table.insert(tagStack, {"macro", name})

                    last = #node.parameters
                    str ..= "(" if last != 0 and not uses_exec
                    str ..= ", " if uses_exec
                    for i, arg in ipairs node.parameters
                        str ..= compile_node(arg)
                        str ..= ", " if i != last
                    if node.closed and last != 0
                        str ..= ")"
                    str ..= "!" if last == 0 and (not env[node.name.value])

                    if node.closed
                        popStack"macro"
                    else
                        level += 1

                when "openArgument"
                    str ..= "{"
                    level += 1
                    table.insert(tagStack, "table")
                when "attribute"
                    str ..= "{"
                    str ..= compile_attribute(node)
                    str ..= "}"
                when "tagEmpty"
                    str ..= "{" .. compile_tag_start(node.value)
                    str ..= compile_tag_end(node.value) .. "}"
                when "stringChain"
                    -- moon.p(node)
                    first = true
                    for value in *node.value
                        str ..= " .. " unless first
                        str ..= compile_node(value)
                        first = false
                    str ..= " .." if node.value.open
                when "emptyArg"
                    str ..= "{}"
                when "macroChain"
                    for i, macro in ipairs node.macros
                        str ..= compile_node(macro)
                        str ..= ", " unless i == #node.macros
                else
                    -- moon.p(node)
                    error("node type unknown: " .. node_type)
            return str

        lineType = ast.type
        switch lineType

            when "do"
                line ..= "do: ->"
                level += 1
                table.insert(tagStack, "do")
            when "stringLine"
                if ast.endString
                    line ..= ast.endString
                if ast.closed
                    line ..= '"'
                if ast.value
                    line ..= " .. " .. compile_node(ast.value)
                if ast.macroClosed
                    line ..= ')'
                    popStack"macro"
                    level -= 1
            -- when "StringLine"
            --     line ..= compile_node(ast[1])
            --     line ..= " .. "
            --     line ..= compile_node(ast[2])
            when "emptyLine"
                line ..= ""
            when "commentLine"
                line ..= "--" .. ast.value\gsub("#", "-")
            when "luaLine"
                line ..= ast.value

                if ast.closed
                    line ..= "]]"
                    popStack"lua"
            when "lua"
                line ..= compile_node(ast)

            when "codeLine"

                codeLineType = ast.value.type

                switch codeLineType

                    when "lua"
                        line ..= compile_node(ast)
                    when "attribute"
                        line ..= compile_attribute(ast.value)

                    when "macroArguementLine"
                        -- moon.p(ast)
                        last = ""
                        for i, thing in ipairs ast.value.value
                            subtype = thing.type
                            switch subtype
                                when "closeMacro"
                                    popStack"macro"
                                    line ..= ")"
                                    level -= 1
                                    last = "closeMacro"
                                when "openMacro"
                                    line ..= "," if last == "closeMacro"
                                    table.insert(tagStack, "macro")
                                    line ..= "{"
                                    last = "openMacro"
                                when "closeArguement"
                                    line ..= "}"
                                    popStack"table"
                                    level -= 1
                                    last = "closeArguement"
                                when "openArgument"
                                    line ..= "," if last == "closeArguement"
                                    table.insert(tagStack, "table")
                                    line ..= "{"
                                    reduce_indentation = true
                                    last = "openArgument"
                    when "macroChain"
                        if tagStack[#tagStack] == "case"
                            delete_empty_line = true
                            line ..= "do: ->\n" .. ident! .. indent_space
                            level += 1
                            table.insert(tagStack, "do")
                        if tagStack[#tagStack] == "event"
                            delete_empty_line = true
                            line ..= "do: ->\n" .. ident! .. indent_space
                            level += 1
                            table.insert(tagStack, "do")
                        for i, macro in ipairs ast.value.macros
                            unless actionWrappersMap[tagStack[#tagStack]]
                                line ..= "<" unless tagStack[#tagStack] == "event"

                            line ..= compile_node(macro)
                            line ..= ", " unless i == #ast.value.macros



                    when "mapFileInclude"
                        -- ast.value.value ..= ".map"
                        line ..= 'map: ' .. compile_node(ast.value.value)


                    when "valueList"
                        line ..= compile_values(ast.value, false)
                    when "stringChain"
                        line ..= compile_node(ast)
                    when "endMultiString"
                        line ..= compile_node(ast)

                    when "ustring"
                        line ..= ast.value
                    when "number"
                        line ..= compile_node(ast)


                    when "tagEmpty"
                        line ..= compile_tag_start(ast.value.value)
                        line ..= compile_tag_end(ast.value.value)
                    when "tagStart"
                        line ..= compile_tag_start(ast.value.value)
                    when "tagEnd"
                        line ..= compile_tag_end(ast.value.value)
                        if ast.macro_closed
                            popStack"macro"
                            line ..= ')'
                            level -= 1
                    when "tagAmend"
                        line ..= compile_tag_start(ast.value.value, true)

                    when "define"
                        tableContext = true
                        tableContext = false if tagStack[#tagStack] == "toplevel"
                        tableContext = false if tagStack[#tagStack] == "do"
                        table.insert(tagStack, "define")
                        line ..= compile_node(ast.value.name)
                        if tableContext
                            line ..= ":"
                        else
                            line ..= " ="
                        line ..= " ("
                        if parameters = ast.value.parameters
                            for i, id in ipairs parameters
                                env[id.value.value] = true
                                line ..= compile_node(id.value)
                                line ..= ", " if i != #parameters

                        isSymbol = true if #ast.value.parameters == 0
                        line ..= ") ->"
                        unless actionMacroMap[ast.value.name.value]
                            line ..= " {"
                            isInTableContext = true
                            table.insert(tagStack, "table")
                        else
                            isInTableContext = false
                        level += 1
                    when "endDef"
                        -- assert(false)
                        if tagStack[#tagStack] == "table"
                            popStack"table"
                            line ..= '}'

                        popStack("define")

                        if isSymbol and not ast.value.value
                            line ..= "true"

                        level -= 1 unless (ast.value.value or isSymbol)
                        line ..= compile_node(ast.value.value) if ast.value.value
                        env = {}

                    when "unDef"
                        line ..= compile_node(ast.value.name) .. " = nil"


                    when "ifDef"
                        tableContext = actionWrappersMap[tagStack[#tagStack]]
                        unless tableContext
                            line ..= "<"
                        line ..= "if " .. compile_node(ast.value.value)
                        line ..= " then {" unless tableContext
                        level += 1
                        table.insert(tagStack, "ifDef")
                        unless tableContext
                            table.insert(tagStack, "table")
                    when "ifNDef"
                        tableContext = actionWrappersMap[tagStack[#tagStack]]
                        unless tableContext
                            line ..= "<"
                        line ..= "unless " .. compile_node(ast.value.value) .. "!"
                        line ..= " then {" unless tableContext
                        level += 1
                        table.insert(tagStack, "ifDef")
                        unless tableContext
                            table.insert(tagStack, "table")
                    when "ifHave"
                        line ..= "if have(" .. compile_node(ast.value.value) .. ")"
                        level += 1
                        table.insert(tagStack, "ifDef")
                    when "ifVer"
                        line ..= "if ver(" .. ast.value.left .. ", \"" .. ast.value.compareOperator .. "\", \"" .. ast.value.version .. "\")"
                        level += 1
                        table.insert(tagStack, "ifDef")
                    when "else"
                        if isInTableContext
                            line ..= "} else {"
                        else
                            line ..= "else"
                        level -= 1
                        reduce_indentation = true
                    when "endIf"
                        level -= 1
                        if isTopStack"table"
                            popStack"table"
                            line ..= "}"
                        popStack("ifDef")
                        -- line ..= "}" unless actionWrappersMap[tagStack[#tagStack]]

                    when "error"
                        line ..= "error(\""   .. ast.value.value .. "\")"
                    when "warning"
                        line ..= "warning(\"" .. ast.value.value .. "\")"
                    when "fileInclude"
                        line ..= "INCLUDE(" .. compile_node(ast.value.value) .. ")"

                if comment = ast.comment
                    comment = trim(comment) if #line == 0
                    line ..= ast.space .. "--" .. comment\gsub('#', '-')

                isSymbol = false unless ast.value.type == "define"

                line = ident! .. line

                if ast.value.type == "endDef"
                    level -= 1 if (ast.value.value or isSymbol)

            when nil
                line ..= "failed to parse this line" unless ast.comment
                -- error"ast.type is a nil value"
            else
                error"unknown type: #{lineType}"

        -- if #line != 0 and ast.type != "luaLine" and lineType != "stringLine"
            -- line = ident! .. line

        if ast.type == "macroArguementLine"
            level += 1 if ast.value.type == "closeOpenArguementLine"

        if ast.isLast and isTopStack"table"
            popStack"table"
            level -= 1
            unless ast.type == "emptyLine"
                line ..= "\n"
            line ..= "}"

        -- if ast.isLast
            -- unless isTopStack"toplevel"
                -- moon.p(tagStack)
                --error"Toplevel not reached."
                -- print "Let's have a look at the tagStack:"
                -- moon.p(tagStack)

        level += 1 if reduce_indentation

        return line, level, delete_empty_line
    return compile_line

return build_compiler
