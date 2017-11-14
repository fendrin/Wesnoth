--
--
-- WML2WSL is a line by line cross-compiler.
-- Thus the parser processes each line seperately.
-- This enables a more easy line by line translation, but gives the compiler more of a challenge.
--
--

moon = require"moon"

lpeg = require"lpeg"
{
    :R, :S,  :V,  :P  -- Paterns
    :C, :Ct, :Cg, :Cc -- Captures
} = lpeg
L = lpeg.luversion and lpeg.L or (v) -> #v


build_parser = ->

    lineNumber = 0

    context = "wml"

    Type = (type_name) ->
        Cg(Cc(type_name), "type")

    Value = (pat) ->
        Cg(pat, "value")

    Space     = S" \t"^0
    SomeSpace = S" \t"^1

    -- a symbol
    sym  = (chars) -> Space * chars
    -- a symbol that doesn't accept whitespace before it
    symx = (chars) -> chars

    Break = P"\r"^-1 * P"\n"
    Stop  = Break + -1

    AlphaNum  = R "az", "AZ", "09", "__", "$$", "{{", "}}"
    NameV     = R("az", "AZ") * AlphaNum^0
    Name      = C NameV
    Char      = R"az" + S","

    Num = P"0x" * R("09", "af", "AF")^1 * (S"uU"^-1 * S"lL"^2)^-1 +
        R"09"^1 * (S"uU"^-1 * S"lL"^2) +
        (
            R"09"^1 * (P"." * R"09"^1)^-1 +
            P"." * R"09"^1
        ) * (S"eE" * P"-"^-1 * R"09"^1)^-1


    wml = {
        "Line"

        MapFileInclude: sym"map_data" * sym"=" * sym'"' * sym'{' * Value(V"Path") * sym'}' * sym'"' * Type"mapFileInclude"

        Comment: sym"#" * C((1 - S"\r\n")^0)
        Number: Ct(Space * (Value(S"+-"^-1 * Num^1) * Type"number" ) * ( #(SomeSpace + S",})") + L(Stop) ))
        Boolean: Ct( (Value(C(sym"yes") + C(sym"true") + C(sym"no") + C(sym"false")) * (#(SomeSpace + S",})") + L(Stop)) ) * Type"bool" )
        StrangeNumber: Ct(Space * (Value( sym"=" * S"+-"^-1 * Num^0) * Type"strangeNumber" ) * ( #(SomeSpace + S",})") + L(Stop) ))

        Character: Ct(Value(Char) * Type"char") --* L(Stop)

        VariableNameV: (P(1) - S" ,|{}})[]$")^1
        VariableName: Ct(Value(C(V"VariableNameV")) * Type"variableName") * sym"|"^-1
        ArrayAccess: Ct(Value(symx"[" * Space *( V"Formula" + Ct(Value(C(Num^1)) * Type"number") + V"Macro" + V"Variable")* sym"]") * Type"arrayAccess")
        Variable: sym"$" * Ct(Value(Ct Cg((V"VariableName" + V"Macro") * V"ArrayAccess"^-1 *
            (sym"." * (V"VariableName" + V"Macro") * V"ArrayAccess"^-1)^0 )) * Type"variable")
        VariableNameThing: Ct(Value(Ct Cg((V"VariableName" + V"Macro") * V"ArrayAccess"^-1 *
            (sym"." * V"VariableName" * V"ArrayAccess"^-1)^1 )) * Type"variableName")

        Condition: V"Expression" * sym"=" * V"Expression"
        If: sym"if(" * V"Condition" * sym")"

        -- Expression: V"Term" * V"MoreTerms"^-1
        -- MoreTerms: sym"+" * Cc"plus" * Space * V"Term" * Space * V"MoreTerms"^-1
        -- Term: V"Variable" + V"Number" --+ sym"(" * V"Expression" * sym")"
        -- AddExp: V"Expression" * sym"+" * Space * V"Expression"
        -- MulExp: V"Expression" * sym"*" * Space * V"Expression"
        Plus:   Ct(sym"+" * Value(Cc"plus")   * Type"operator" * Space)
        Minus:  Ct(sym"-" * Value(Cc"minus")  * Type"operator" * Space)
        Modulo: Ct(sym"%" * Value(Cc"modulo") * Type"operator" * Space)
        Div:    Ct(sym"/" * Value(Cc"div")    * Type"operator" * Space)
        Mul:    sym"*" * Cc"mul"  * Space
        Expression: (V"Plus" + V"Minus" + V"Modulo")^-1 * V"Term" * ( (V"Plus" + V"Minus" + V"Modulo") * V"Term")^0
        Term:   V"Factor" * ((V"Mul" + V"Div") * V"Factor")^0
        Factor: V"Variable" + V"Number" --+ sym"(" * V"Expression" * sym")"
        Formula: Ct(Value(sym"$(" * Space * Ct(V"Expression") * sym")") * Type"Formula")

        Lua: sym"<<" * Ct(Value((P(1) - S">>")^0) * Type"lua" * ((sym">>") * Cg(Cc"true", "closed") )^-1) / (ast) ->
            unless ast.closed
                context = "lua"
            return ast

        -- EmptyLine:   Space * #(V"Comment")^-1 * L(Stop) / (table) -> { type: "emptyLine" }
        EmptyLine:   Space * L(Stop) / (table) -> { type: "emptyLine" }
        -- CommentLine: Ct(Value(C(symx"#" * P(1)^0 * L(Stop)) - V"PreProc") * Type"commentLine")
        CommentLine: Ct( ( (sym"#" * (Value(C(P(1)^0 * L(Stop))))) - V"PreProc") * Type"commentLine")
        CodeLine:    ( ( Ct(V"MapFileInclude") + Ct(V"Attribute") + Ct(V"MacroChain") + Ct(V"FileInclude") + (V"MacroArguementLine") + Ct(V"Tag") + Ct(V"PreProc") + Ct(V"ValueList") + V"Value") ) / (cfg) ->
            -- moon.p(cfg)
            return cfg
        Line:        (V"WML2WSL" + V"Lua" + V"EmptyLine" + V"CommentLine" + Ct(Value(V"CodeLine") * Type"codeLine" * Cg(C(Space), "space") * Cg(V"Comment", "comment")^-1) ) * Space * L(Stop)

        FileName:    (P(1) - S'."$ /}{,[]#~')^1
        Path:        Ct(Value(C(sym"~"^-1 * S"/"^0 * (V"FileName" + V"Macro") * (sym"/" * V"FileName")^0 * sym"/"^-1 * (P"." * V"FileName")^-1)) * Type"path")
        ImagePath:   Ct(Cg(V"Path", "path") * Cg(C(symx"~" * NameV * symx"(" * (P(1) - S")")^0 * symx")")^1, "imageExtension") * Type"imagePath")
        FileInclude: sym"{" * Value(V"Path") * sym"}" * Type"fileInclude"

        URL: Ct(Value(C(NameV * (symx"." * NameV)^2 * symx":" * V"Number")) * Type"URL")

        -- MacroArguementLine: V"TagEnd" * sym")" -- * Cg(Ct(Cg(SomeSpace * (V"Argument"))^0),"parameters") * (sym"}" * Cg(Cc"true", "closed"))^-1
        MacroArguementLine:  Ct(Value( Ct(Cg((V"CloseArguement" + V"OpenArgument" + V"OpenMacro" + V"CloseMacro") * Space)^1) ) * Type"macroArguementLine") / (cfg) ->
            -- moon.p(cfg)
            return cfg

        CloseMacro:     Ct(sym"}" * Type"closeMacro")
        OpenMacro:      Ct(sym"{" * Type"openMacro")
        CloseArguement: Ct(sym")" * Type"closeArguement")
        OpenArgument:   Ct(sym"(" * Type"openArgument")

        AltQuotedString: (Ct ( ((sym"_" * Space * Cg(Cc"true", "translated"))^-1 ) * sym'<<' * Value(C( (P(1) - S'<<')^0) ) * ( sym'>>' * Cg(Cc"true", "closed") )^-1 * Type"string")) / (ast) ->
            unless ast.closed
                context = "altString"
            return ast
        QuotedString: (Ct ( ((sym"_" * Space * Cg(Cc"true", "translated"))^-1 ) * sym'"' * Value(C( (P(1) - S'"')^0) ) * ( sym'"' * Cg(Cc"true", "closed") )^-1 * Type"string")) / (ast) ->
            unless ast.closed
                context = "string"
            return ast
        AnyString:      (V"Macro" * (#(SomeSpace + S"+ }") + L(Stop))) + V"QuotedString" + V"AltQuotedString" + V"Variable" + Stop * Ct(Type"emptyString")
        StringChain:    Ct(Value( (Ct( V"AnyString" * (sym"+" * Space * (V"AnyString" + Cg(Cc"true", "open")))^0))) * Type"stringChain")

        QuotedStringClean: (Ct ( ((symx"_" * Cg(Cc"true", "translated"))^-1 ) * symx'"' * Value(C( (P(1) - S'"')^0) ) * ( symx'"' * Cg(Cc"true", "closed") )^-1 * Type"string")) / (ast) ->
            unless ast.closed
                context = "string"
            return ast

        Anything: C(P(1)^1)
        UStringV: (P(1) - S'"\#=(), ')^1
        UString:  Space * Ct(Value(V"UStringV" * (SomeSpace * V"UStringV")^0 ) * Cg(Cc"true", "closed") * Type"string")
        -- SString:        Ct(Value((P(1) - S'" {}()=')^1 - V"Number")          * Cg(Cc"true", "closed") * Type"string")

        simpleString:  (P(1) - S' {}')^1
        SString_sowas: V"simpleString" * (Cc'#' * symx"{" * V"Macro" * sym"}")^-1 * V"simpleString"^-1
        SString:       Ct(Value(V"SString_sowas" - V"Number") * Cg(Cc"true", "closed") * Type"string")

        ValueClean:     V"StrangeNumber" + V"Variable" + V"Boolean" + V"Number" + V"QuotedStringClean" + V"SString"
        AttributeClean: Space * V"KeyListClean" * symx"=" * Value(Ct(V"ValueListClean")) * Type"attribute"
        KeyListClean:   Cg(Ct( (V"Macro" + Name) * (symx"," * Name)^0), "keys") * #(sym"=")
        ValueListClean: Value(Cg(Ct(V"ValueClean" * (symx"," * V"ValueClean")^0))) * Type"valueList"

        AttributeName: C((R "az", "AZ", "09", "__")^1)
        Attribute: Space * V"KeyList" * sym"=" * Space * Value(Ct(V"ValueList")) * Type"attribute"
        KeyList:   Cg(Ct( (V"MacroClosed" + V"AttributeName") * (sym"," * Name)^0),"keys") * #(sym"=")
        Value:     V"StrangeNumber" + V"Boolean" + V"Number" + V"URL" + V"ImagePath" + V"VariableNameThing" + (V"MacroClosed" * (#(SomeSpace + S",#") + L(Stop)) ) + V"Lua" +
            V"Formula" + V"Variable" + V"StringChain" + V"QuotedString" + V"UString" + V"Path" + V"Character"

        ValueList: Value(Cg(Ct(V"Value" * (sym"," * Space * V"Value")^0 ))) * Type"valueList"

        TagName:  (R "az", "__", "09")^1
        Tag:      (V"TagEmpty") + V"TagStart" + (V"TagEnd") + V"TagAmend"
        TagStart:  sym"["  * Value(V"TagName") * symx"]" * Type"tagStart"
        TagEnd:   (sym"[/" * Value(V"TagName") * symx"]" * Type"tagEnd") --q* (P(1) - S"#")^0
        -- TagEnd: Ct( Value(sym"[/" * C(V"TagName") * symx"]") * ( sym")" * Cg(Cc"true", "arguement_closed") * V"MacroParameter" * (sym"}" * Cg(Cc"true", "macro_closed"))^-1 )^-1 * Type"tagEnd")
        -- TagEnd:   Ct( Value(sym"[/" * C(V"TagName") * symx"]") * ( sym")" * Cg(Cc"true", "arguement_closed") * V"MacroParameter" * (sym"}" * Cg(Cc"true", "macro_closed"))^-1 ) * Type"tagEnd")
        TagEmpty: Value(sym"["   * C(V"TagName") * symx"]"  * Space * "[/" * V"TagName" * sym"]") * Type"tagEmpty"
        TagAmend: Value(sym"[+"  * C(V"TagName") * symx"]") * Type"tagAmend"

        -- EnclosedArgument: sym"(" * (Ct(V"Tag") + Ct(V"Attribute") + Ct(V"ValueList") + V"Variable" + Ct(V"TagEmpty") + Ct(V"MacroChain") + V"Macro") * (sym")" * Cg(Cc"true", "closed"))^-1
        EnclosedArgument: sym"(" * (Ct(V"Tag") + Ct(V"Attribute") + Ct(V"MacroChain") + Ct(V"ValueList") + V"Variable" + Ct(V"TagEmpty") + V"Macro") * (sym")" * Cg(Cc"true", "closed"))

        Argument:  Ct(sym"()" * Type"emptyArg") + Ct(V"AttributeClean") + Ct(V"ValueListClean") + V"QuotedStringClean" + V"Boolean" + V"Variable"  + V"Macro" +
            Ct(V"SString") + V"Number" --+ V"EnclosedArgument" --+ V"OpenArgument"

        -- MacroArguementLine: sym')' * sym'}'
            --Cg(Ct(Cg(SomeSpace * (V"Argument"))^0),"parameters")^-1 * (sym")" * Cg(Cc"true", "table_closed"))^-1 * (sym"}" * Cg(Cc"true", "macro_closed"))^-1 * Type"macroArguementLine"

        MacroName: Ct(Value(C R("AZ", "__", "::", "09", "xx", "--")^1) * Type"id")
        -- Macro:     Ct(sym"{" * ( Cg(V"MacroName" + V"Macro","name")) * Cg(Ct(Cg(SomeSpace * (V"Argument"))^0),"parameters") * (sym"}" * Cg(Cc"true", "closed"))^-1 * Type("macro") )

        -- ArgumentParentheses: (P(1) - S" ") * Space
        -- ArgumentSome:        (P(1) - S')') * P")"

        MacroParameter: Cg(Ct(Cg(SomeSpace * (
            (#(P"(" * (P(1) - S')')^0 * P")") * V"EnclosedArgument") +
            (#(P(1) - S" ")^0 * V"Argument") +
            (P"(" * ( (Ct( ((V"TagStart" + V"Attribute") * Cg(Cc"true", "param_open")) + Type"openArgument") )))
            ) )^0),"parameters") *
            (sym"}" * Cg(Cc"true", "closed"))^-1

        Macro: Ct(sym"{" * Cg(V"MacroName" + V"Macro", "name") *
            Cg(Ct(Cg(SomeSpace * (
            (sym"(" * sym")" * Ct(Type"emptyArg")) +
            (#(P"(" * (P(1) - S')')^0 * P")") * V"EnclosedArgument") +
            (Space * P"(" * ( (Ct( ((V"TagStart" + V"Attribute") * Cg(Cc"true", "param_open")) + Type"openArgument") ))) +
            (#(P(1) - S" (")^1 * V"Argument")
            -- (P"(" * Ct(Cg(Cc"true", "paramOpen")) * Ct(V"TagStart")^-1 )
            ) )^0),"parameters") *
            (sym"}" * Cg(Cc"true", "closed"))^-1 * Type("macro") )

        MacroClosed: Ct(sym"{" * ( Cg(V"MacroName" + V"Macro","name")) * Cg(Ct(Cg(SomeSpace * (V"Argument"))^0),"parameters") * (sym"}" * Cg(Cc"true", "closed"))    * Type("macro") )
        MacroChain: (Cg(Ct(V"Macro" * (Space * V"Macro")^0), "macros")) * Type"macroChain"

        WML2WSL: Ct(sym"#" * sym"wml2wsl: do" * Space * Type"do")

        PreProc: V"Define" + V"Undef" + V"EndIf" + V"Else" + V"IfDef" + V"EndDef" + V"IfNDef" + V"Error" + V"Warning" + V"IfHave" + V"IfVer"
        Compare: C(sym">=" + sym"<=" + sym"<" + sym">" + sym"==")
        IfHave:  sym"#ifhave" * SomeSpace * Value(V"Path")          * Type"ifHave"
        IfDef:   sym"#ifdef"  * SomeSpace * Value(V"MacroName")     * Type"ifDef"
        IfNDef:  sym"#ifndef" * SomeSpace * Value(V"MacroName")     * Type"ifNDef"
        Else:    sym"#else"                                         * Type"else"
        EndIf:   sym"#endif"                                        * Type"endIf"
        -- EndDef:  #((P(1) - S"#")^0 * P"#enddef") * Ct( ((Value(Ct V"ValueList")) + Space) * sym"#enddef" * Type"endDef" )
        -- EndDef:  sym"#enddef" * Type"endDef"
        EndDef:  (Value(Ct V"ValueList")^-1 * sym"#enddef" * Type"endDef")
        -- EndDef:  (V"Value"^-1 * sym"#enddef" * Type"endDef") / (cfg) ->
            -- moon.p(cfg)
            -- return cfg
        Define:  sym"#define" * SomeSpace * (Cg(V"MacroName", "name") * Cg(Ct((SomeSpace * Ct(Value(V"MacroName") * Type"Name"))^0 ),"parameters") ) * Type"define"
        Undef:   sym"#undef"  * SomeSpace *  Cg(V"MacroName", "name") * Type"unDef"
        Version: C(Num^1 * (sym"+")^0)
        IfVer:   sym"#ifver"  * SomeSpace * (Cg(Name, "left") * SomeSpace * Cg(V"Compare", "compareOperator") * SomeSpace * Cg(V"Version", "version")) * Type"ifVer"
        IfNVer:  sym"#ifnver" * SomeSpace * (Cg(Name, "left") * SomeSpace * Cg(V"Compare", "compareOperator") * SomeSpace * Cg(V"Version", "version")) * Type"ifNVer"

        Error:   Value(sym"#error"   * SomeSpace * V"Anything") * Type"error"
        Warning: Value(sym"#warning" * SomeSpace * V"Anything") * Type"warning"
    }

    stringLine = {
        "line"

        QuotedString: (Ct ( ((sym"_" * Space * Cg(Cc"true", "translated"))^-1 ) * sym'"' * Value(C( (P(1) - S'"')^0) ) * ( sym'"' * Cg(Cc"true", "closed") )^-1 * Type"string")) / (ast) ->
            unless ast.closed
                context = "string"
            return ast
        AnyString:   V"QuotedString"
        StringChain: Ct(Value( Cg (Ct((sym"+" * Space * (V"AnyString"))^1))) * Type"stringChain")

        line: Ct( (Cg((P(1) - S'"')^0, "endString")) * (sym'"' * Cg(Cc"true", "closed"))^-1 * Value(V"StringChain")^-1 * ( sym'}' * Cg(Cc"true", "macroClosed") )^-1 * Type"stringLine") / (ast) ->
            -- moon.p(ast)
            if ast.closed --
                unless ast.value
                    -- moon.p(ast)
                    context = "wml"

            if ast.value
                if ast.value.value[#ast.value.value].closed
                    context = "wml"
            return ast
    }

    altStringLine = {
        "line"

        QuotedString: (Ct ( ((sym"_" * Space * Cg(Cc"true", "translated"))^-1 ) * sym'<<' * Value(C( (P(1) - S'>>')^0) ) * ( sym'>>' * Cg(Cc"true", "closed") )^-1 * Type"string")) / (ast) ->
            unless ast.closed
                context = "altString"
            return ast
        AnyString:   V"QuotedString"
        StringChain: Ct(Value( Cg (Ct((sym"+" * Space * (V"AnyString"))^1))) * Type"stringChain")

        line: Ct( (Cg((P(1) - S'>>')^0, "endString")) * (sym'>>' * Cg(Cc"true", "closed"))^-1 * Value(V"StringChain")^-1 * ( sym'}' * Cg(Cc"true", "macroClosed") )^-1 * Type"stringLine") / (ast) ->
            if ast.closed
                context = "wml"
            return ast
    }

    luaLine = {
        "line"
        line: Ct(Value((P(1) - S">>")^0 ) * Type"luaLine" * (sym">>" * Cg(Cc"true", "closed"))^-1 ) / (ast) ->
            if ast.closed
                context = "wml"
            return ast
    }


    parse = (line, last) ->

        lineNumber += 1

        local debug, show_match_trace
        -- debug = true
        -- show_match_trace = true

        g = switch context
            when "wml"
                wml
            when "lua"
                luaLine
            when "string"
                stringLine
            when "altString"
                altStringLine
            else
                print "unknown context: #{context}"
                assert(false)

        ast = P(g)\match(line)
        -- moon.p(ast)
        -- unless ast
            -- sowas = true
            -- error("Error parsing (#{context}) line #{lineNumber}: #{line}")
            -- debug = true
            -- show_match_trace = true
        -- elseif last
            -- ast.isLast = true

        ast.isLast = last if ast


        if debug
            print("################# " .. line .. " #################")
            if show_match_trace
                grammar = require('pegdebug').trace(g)
                print(lpeg.match(P(grammar), line))
            if ast
                moon.p(ast)
            else
                error("Error parsing (#{context}) line: #{line}")



        return ast

    return parse

return build_parser
