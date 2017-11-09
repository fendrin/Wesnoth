# wsl coding conventions

## UPPER_CASE_WITH_UNDERSCORES: multi arguement functions

## Mixed_Case_With_Underscores: single table arguement functions
The former ToplevelWML and ActionWML tags.

## Variable Names


# wml2wsl
Transcompiles Wesnoth's WML (.cfg) files into WSL.

## Comments
Since wml2wsl compiles the information into the output file at the same line as its origin,
comments can be perserved and keep their meaning in respect to the context.

## WML
WML tags transform into lua function calls with the tag's contents as single table arguement.

## Lua Tables

## Precondissions

* Lua5.x

* Only Balanced WML
* WMLLINT + WMLINDENT
* Macro Names must be all uppercase like in mainline
* Macro Definitions might need to be replaced

## Translating

### WML Tag

#### ToplevelWML

#### ActionWML
Those tags translate into function calls with the tag's contents as single table arguement.
But they begin with a capital letter.
This change has been made to avoid naming conflicts with variables.

### WML Attributes

Attributes transform into table

### MacroDefinition

### MacroInclusion

###
