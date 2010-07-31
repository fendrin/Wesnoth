/*
* generated by Xtext
*/
grammar InternalWML;

options {
	superClass=AbstractInternalAntlrParser;
	
}

@lexer::header {
package org.wesnoth.parser.antlr.internal;

// Hack: Use our own Lexer superclass by means of import. 
// Currently there is no other way to specify the superclass for the lexer.
import org.eclipse.xtext.parser.antlr.Lexer;
}

@parser::header {
package org.wesnoth.parser.antlr.internal; 

import java.io.InputStream;
import org.eclipse.xtext.*;
import org.eclipse.xtext.parser.*;
import org.eclipse.xtext.parser.impl.*;
import org.eclipse.xtext.parsetree.*;
import org.eclipse.emf.ecore.util.EcoreUtil;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.xtext.parser.antlr.AbstractInternalAntlrParser;
import org.eclipse.xtext.parser.antlr.XtextTokenStream;
import org.eclipse.xtext.parser.antlr.XtextTokenStream.HiddenTokens;
import org.eclipse.xtext.parser.antlr.AntlrDatatypeRuleToken;
import org.eclipse.xtext.conversion.ValueConverterException;
import org.wesnoth.services.WMLGrammarAccess;

}

@parser::members {

 	private WMLGrammarAccess grammarAccess;
 	
    public InternalWMLParser(TokenStream input, IAstFactory factory, WMLGrammarAccess grammarAccess) {
        this(input);
        this.factory = factory;
        registerRules(grammarAccess.getGrammar());
        this.grammarAccess = grammarAccess;
    }
    
    @Override
    protected InputStream getTokenFile() {
    	ClassLoader classLoader = getClass().getClassLoader();
    	return classLoader.getResourceAsStream("org/wesnoth/parser/antlr/internal/InternalWML.tokens");
    }
    
    @Override
    protected String getFirstRuleName() {
    	return "WMLRoot";	
   	}
   	
   	@Override
   	protected WMLGrammarAccess getGrammarAccess() {
   		return grammarAccess;
   	}
}

@rulecatch { 
    catch (RecognitionException re) { 
        recover(input,re); 
        appendSkippedTokens();
    } 
}




// Entry rule entryRuleWMLRoot
entryRuleWMLRoot returns [EObject current=null] 
	:
	{ currentNode = createCompositeNode(grammarAccess.getWMLRootRule(), currentNode); }
	 iv_ruleWMLRoot=ruleWMLRoot 
	 { $current=$iv_ruleWMLRoot.current; } 
	 EOF 
;

// Rule WMLRoot
ruleWMLRoot returns [EObject current=null] 
    @init { EObject temp=null; setCurrentLookahead(); resetLookahead(); 
    }
    @after { resetLookahead(); 
    	lastConsumedNode = currentNode;
    }:
((
(
		{ 
	        currentNode=createCompositeNode(grammarAccess.getWMLRootAccess().getTagsWMLTagParserRuleCall_0_0(), currentNode); 
	    }
		lv_Tags_0_0=ruleWMLTag		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLRootRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode.getParent(), $current);
	        }
	        try {
	       		add(
	       			$current, 
	       			"Tags",
	        		lv_Tags_0_0, 
	        		"WMLTag", 
	        		currentNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	        currentNode = currentNode.getParent();
	    }

)
)
    |(
(
		{ 
	        currentNode=createCompositeNode(grammarAccess.getWMLRootAccess().getMacroCallsWMLMacroCallParserRuleCall_1_0(), currentNode); 
	    }
		lv_MacroCalls_1_0=ruleWMLMacroCall		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLRootRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode.getParent(), $current);
	        }
	        try {
	       		add(
	       			$current, 
	       			"MacroCalls",
	        		lv_MacroCalls_1_0, 
	        		"WMLMacroCall", 
	        		currentNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	        currentNode = currentNode.getParent();
	    }

)
)
    |(
(
		{ 
	        currentNode=createCompositeNode(grammarAccess.getWMLRootAccess().getMacroDefinesWMLMacroDefineParserRuleCall_2_0(), currentNode); 
	    }
		lv_MacroDefines_2_0=ruleWMLMacroDefine		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLRootRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode.getParent(), $current);
	        }
	        try {
	       		add(
	       			$current, 
	       			"MacroDefines",
	        		lv_MacroDefines_2_0, 
	        		"WMLMacroDefine", 
	        		currentNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	        currentNode = currentNode.getParent();
	    }

)
)
    |(
(
		{ 
	        currentNode=createCompositeNode(grammarAccess.getWMLRootAccess().getTextdomainsWMLTextdomainParserRuleCall_3_0(), currentNode); 
	    }
		lv_Textdomains_3_0=ruleWMLTextdomain		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLRootRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode.getParent(), $current);
	        }
	        try {
	       		add(
	       			$current, 
	       			"Textdomains",
	        		lv_Textdomains_3_0, 
	        		"WMLTextdomain", 
	        		currentNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	        currentNode = currentNode.getParent();
	    }

)
))*
;





// Entry rule entryRuleWMLTag
entryRuleWMLTag returns [EObject current=null] 
	:
	{ currentNode = createCompositeNode(grammarAccess.getWMLTagRule(), currentNode); }
	 iv_ruleWMLTag=ruleWMLTag 
	 { $current=$iv_ruleWMLTag.current; } 
	 EOF 
;

// Rule WMLTag
ruleWMLTag returns [EObject current=null] 
    @init { EObject temp=null; setCurrentLookahead(); resetLookahead(); 
    }
    @after { resetLookahead(); 
    	lastConsumedNode = currentNode;
    }:
(	'[' 
    {
        createLeafNode(grammarAccess.getWMLTagAccess().getLeftSquareBracketKeyword_0(), null); 
    }
(
(
		lv_plus_1_0=	'+' 
    {
        createLeafNode(grammarAccess.getWMLTagAccess().getPlusPlusSignKeyword_1_0(), "plus"); 
    }
 
	    {
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode, $current);
	        }
	        
	        try {
	       		set($current, "plus", true, "+", lastConsumedNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	    }

)
)?(
(
		lv_name_2_0=RULE_ID
		{
			createLeafNode(grammarAccess.getWMLTagAccess().getNameIDTerminalRuleCall_2_0(), "name"); 
		}
		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode, $current);
	        }
	        try {
	       		set(
	       			$current, 
	       			"name",
	        		lv_name_2_0, 
	        		"ID", 
	        		lastConsumedNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	    }

)
)	']' 
    {
        createLeafNode(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_3(), null); 
    }
((
(
		{ 
	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getTagsWMLTagParserRuleCall_4_0_0(), currentNode); 
	    }
		lv_Tags_4_0=ruleWMLTag		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode.getParent(), $current);
	        }
	        try {
	       		add(
	       			$current, 
	       			"Tags",
	        		lv_Tags_4_0, 
	        		"WMLTag", 
	        		currentNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	        currentNode = currentNode.getParent();
	    }

)
)
    |(
(
		{ 
	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getKeysWMLKeyParserRuleCall_4_1_0(), currentNode); 
	    }
		lv_Keys_5_0=ruleWMLKey		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode.getParent(), $current);
	        }
	        try {
	       		add(
	       			$current, 
	       			"Keys",
	        		lv_Keys_5_0, 
	        		"WMLKey", 
	        		currentNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	        currentNode = currentNode.getParent();
	    }

)
)
    |(
(
		{ 
	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getMacroCallsWMLMacroCallParserRuleCall_4_2_0(), currentNode); 
	    }
		lv_MacroCalls_6_0=ruleWMLMacroCall		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode.getParent(), $current);
	        }
	        try {
	       		add(
	       			$current, 
	       			"MacroCalls",
	        		lv_MacroCalls_6_0, 
	        		"WMLMacroCall", 
	        		currentNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	        currentNode = currentNode.getParent();
	    }

)
)
    |(
(
		{ 
	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getMacroDefinesWMLMacroDefineParserRuleCall_4_3_0(), currentNode); 
	    }
		lv_MacroDefines_7_0=ruleWMLMacroDefine		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode.getParent(), $current);
	        }
	        try {
	       		add(
	       			$current, 
	       			"MacroDefines",
	        		lv_MacroDefines_7_0, 
	        		"WMLMacroDefine", 
	        		currentNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	        currentNode = currentNode.getParent();
	    }

)
)
    |(
(
		{ 
	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getTextdomainsWMLTextdomainParserRuleCall_4_4_0(), currentNode); 
	    }
		lv_Textdomains_8_0=ruleWMLTextdomain		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode.getParent(), $current);
	        }
	        try {
	       		add(
	       			$current, 
	       			"Textdomains",
	        		lv_Textdomains_8_0, 
	        		"WMLTextdomain", 
	        		currentNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	        currentNode = currentNode.getParent();
	    }

)
))*	'[/' 
    {
        createLeafNode(grammarAccess.getWMLTagAccess().getLeftSquareBracketSolidusKeyword_5(), null); 
    }
(
(
		lv_endName_10_0=RULE_ID
		{
			createLeafNode(grammarAccess.getWMLTagAccess().getEndNameIDTerminalRuleCall_6_0(), "endName"); 
		}
		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode, $current);
	        }
	        try {
	       		set(
	       			$current, 
	       			"endName",
	        		lv_endName_10_0, 
	        		"ID", 
	        		lastConsumedNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	    }

)
)	']' 
    {
        createLeafNode(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_7(), null); 
    }
)
;





// Entry rule entryRuleWMLKey
entryRuleWMLKey returns [EObject current=null] 
	@init { 
		HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens("RULE_WS");
	}
	:
	{ currentNode = createCompositeNode(grammarAccess.getWMLKeyRule(), currentNode); }
	 iv_ruleWMLKey=ruleWMLKey 
	 { $current=$iv_ruleWMLKey.current; } 
	 EOF 
;
finally {
	myHiddenTokenState.restore();
}

// Rule WMLKey
ruleWMLKey returns [EObject current=null] 
    @init { EObject temp=null; setCurrentLookahead(); resetLookahead(); 
		HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens("RULE_WS");
    }
    @after { resetLookahead(); 
    	lastConsumedNode = currentNode;
    }:
((
(
		lv_name_0_0=RULE_ID
		{
			createLeafNode(grammarAccess.getWMLKeyAccess().getNameIDTerminalRuleCall_0_0(), "name"); 
		}
		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLKeyRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode, $current);
	        }
	        try {
	       		set(
	       			$current, 
	       			"name",
	        		lv_name_0_0, 
	        		"ID", 
	        		lastConsumedNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	    }

)
)	'=' 
    {
        createLeafNode(grammarAccess.getWMLKeyAccess().getEqualsSignKeyword_1(), null); 
    }
(
(
		{ 
	        currentNode=createCompositeNode(grammarAccess.getWMLKeyAccess().getValueWMLKeyValueParserRuleCall_2_0(), currentNode); 
	    }
		lv_value_2_0=ruleWMLKeyValue		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLKeyRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode.getParent(), $current);
	        }
	        try {
	       		add(
	       			$current, 
	       			"value",
	        		lv_value_2_0, 
	        		"WMLKeyValue", 
	        		currentNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	        currentNode = currentNode.getParent();
	    }

)
)+(
(
(
		lv_eol_3_1=RULE_EOL
		{
			createLeafNode(grammarAccess.getWMLKeyAccess().getEolEOLTerminalRuleCall_3_0_0(), "eol"); 
		}
		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLKeyRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode, $current);
	        }
	        try {
	       		set(
	       			$current, 
	       			"eol",
	        		lv_eol_3_1, 
	        		"EOL", 
	        		lastConsumedNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	    }

    |		lv_eol_3_2=RULE_SL_COMMENT
		{
			createLeafNode(grammarAccess.getWMLKeyAccess().getEolSL_COMMENTTerminalRuleCall_3_0_1(), "eol"); 
		}
		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLKeyRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode, $current);
	        }
	        try {
	       		set(
	       			$current, 
	       			"eol",
	        		lv_eol_3_2, 
	        		"SL_COMMENT", 
	        		lastConsumedNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	    }

)

)
))
;
finally {
	myHiddenTokenState.restore();
}





// Entry rule entryRuleWMLKeyValue
entryRuleWMLKeyValue returns [EObject current=null] 
	:
	{ currentNode = createCompositeNode(grammarAccess.getWMLKeyValueRule(), currentNode); }
	 iv_ruleWMLKeyValue=ruleWMLKeyValue 
	 { $current=$iv_ruleWMLKeyValue.current; } 
	 EOF 
;

// Rule WMLKeyValue
ruleWMLKeyValue returns [EObject current=null] 
    @init { EObject temp=null; setCurrentLookahead(); resetLookahead(); 
    }
    @after { resetLookahead(); 
    	lastConsumedNode = currentNode;
    }:
(
    { 
        currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getWMLValueParserRuleCall_0(), currentNode); 
    }
    this_WMLValue_0=ruleWMLValue
    { 
        $current = $this_WMLValue_0.current; 
        currentNode = currentNode.getParent();
    }

    |
    { 
        currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getWMLMacroCallParserRuleCall_1(), currentNode); 
    }
    this_WMLMacroCall_1=ruleWMLMacroCall
    { 
        $current = $this_WMLMacroCall_1.current; 
        currentNode = currentNode.getParent();
    }

    |
    { 
        currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getWMLLuaCodeParserRuleCall_2(), currentNode); 
    }
    this_WMLLuaCode_2=ruleWMLLuaCode
    { 
        $current = $this_WMLLuaCode_2.current; 
        currentNode = currentNode.getParent();
    }

    |
    { 
        currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getWMLArrayCallParserRuleCall_3(), currentNode); 
    }
    this_WMLArrayCall_3=ruleWMLArrayCall
    { 
        $current = $this_WMLArrayCall_3.current; 
        currentNode = currentNode.getParent();
    }
)
;





// Entry rule entryRuleWMLMacroCall
entryRuleWMLMacroCall returns [EObject current=null] 
	:
	{ currentNode = createCompositeNode(grammarAccess.getWMLMacroCallRule(), currentNode); }
	 iv_ruleWMLMacroCall=ruleWMLMacroCall 
	 { $current=$iv_ruleWMLMacroCall.current; } 
	 EOF 
;

// Rule WMLMacroCall
ruleWMLMacroCall returns [EObject current=null] 
    @init { EObject temp=null; setCurrentLookahead(); resetLookahead(); 
    }
    @after { resetLookahead(); 
    	lastConsumedNode = currentNode;
    }:
(	'{' 
    {
        createLeafNode(grammarAccess.getWMLMacroCallAccess().getLeftCurlyBracketKeyword_0(), null); 
    }
(
(
		lv_relative_1_0=	'~' 
    {
        createLeafNode(grammarAccess.getWMLMacroCallAccess().getRelativeTildeKeyword_1_0(), "relative"); 
    }
 
	    {
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode, $current);
	        }
	        
	        try {
	       		set($current, "relative", true, "~", lastConsumedNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	    }

)
)?(
(
		lv_name_2_0=RULE_ID
		{
			createLeafNode(grammarAccess.getWMLMacroCallAccess().getNameIDTerminalRuleCall_2_0(), "name"); 
		}
		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode, $current);
	        }
	        try {
	       		set(
	       			$current, 
	       			"name",
	        		lv_name_2_0, 
	        		"ID", 
	        		lastConsumedNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	    }

)
)((
(
(
		{ 
	        currentNode=createCompositeNode(grammarAccess.getWMLMacroCallAccess().getParamsWMLValueParserRuleCall_3_0_0_0(), currentNode); 
	    }
		lv_params_3_1=ruleWMLValue		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode.getParent(), $current);
	        }
	        try {
	       		add(
	       			$current, 
	       			"params",
	        		lv_params_3_1, 
	        		"WMLValue", 
	        		currentNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	        currentNode = currentNode.getParent();
	    }

    |		{ 
	        currentNode=createCompositeNode(grammarAccess.getWMLMacroCallAccess().getParamsWMLMacroParameterParserRuleCall_3_0_0_1(), currentNode); 
	    }
		lv_params_3_2=ruleWMLMacroParameter		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode.getParent(), $current);
	        }
	        try {
	       		add(
	       			$current, 
	       			"params",
	        		lv_params_3_2, 
	        		"WMLMacroParameter", 
	        		currentNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	        currentNode = currentNode.getParent();
	    }

)

)
)
    |(
(
		{ 
	        currentNode=createCompositeNode(grammarAccess.getWMLMacroCallAccess().getExtraMacrosWMLMacroCallParserRuleCall_3_1_0(), currentNode); 
	    }
		lv_extraMacros_4_0=ruleWMLMacroCall		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode.getParent(), $current);
	        }
	        try {
	       		add(
	       			$current, 
	       			"extraMacros",
	        		lv_extraMacros_4_0, 
	        		"WMLMacroCall", 
	        		currentNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	        currentNode = currentNode.getParent();
	    }

)
))*	'}' 
    {
        createLeafNode(grammarAccess.getWMLMacroCallAccess().getRightCurlyBracketKeyword_4(), null); 
    }
)
;





// Entry rule entryRuleWMLMacroParameter
entryRuleWMLMacroParameter returns [EObject current=null] 
	:
	{ currentNode = createCompositeNode(grammarAccess.getWMLMacroParameterRule(), currentNode); }
	 iv_ruleWMLMacroParameter=ruleWMLMacroParameter 
	 { $current=$iv_ruleWMLMacroParameter.current; } 
	 EOF 
;

// Rule WMLMacroParameter
ruleWMLMacroParameter returns [EObject current=null] 
    @init { EObject temp=null; setCurrentLookahead(); resetLookahead(); 
    }
    @after { resetLookahead(); 
    	lastConsumedNode = currentNode;
    }:
(	'(' 
    {
        createLeafNode(grammarAccess.getWMLMacroParameterAccess().getLeftParenthesisKeyword_0(), null); 
    }
(
(
(
		{ 
	        currentNode=createCompositeNode(grammarAccess.getWMLMacroParameterAccess().getParamWMLValueParserRuleCall_1_0_0(), currentNode); 
	    }
		lv_param_1_1=ruleWMLValue		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLMacroParameterRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode.getParent(), $current);
	        }
	        try {
	       		add(
	       			$current, 
	       			"param",
	        		lv_param_1_1, 
	        		"WMLValue", 
	        		currentNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	        currentNode = currentNode.getParent();
	    }

    |		{ 
	        currentNode=createCompositeNode(grammarAccess.getWMLMacroParameterAccess().getParamWMLTagParserRuleCall_1_0_1(), currentNode); 
	    }
		lv_param_1_2=ruleWMLTag		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLMacroParameterRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode.getParent(), $current);
	        }
	        try {
	       		add(
	       			$current, 
	       			"param",
	        		lv_param_1_2, 
	        		"WMLTag", 
	        		currentNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	        currentNode = currentNode.getParent();
	    }

    |		{ 
	        currentNode=createCompositeNode(grammarAccess.getWMLMacroParameterAccess().getParamWMLMacroCallParserRuleCall_1_0_2(), currentNode); 
	    }
		lv_param_1_3=ruleWMLMacroCall		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLMacroParameterRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode.getParent(), $current);
	        }
	        try {
	       		add(
	       			$current, 
	       			"param",
	        		lv_param_1_3, 
	        		"WMLMacroCall", 
	        		currentNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	        currentNode = currentNode.getParent();
	    }

    |		{ 
	        currentNode=createCompositeNode(grammarAccess.getWMLMacroParameterAccess().getParamWMLKeyParserRuleCall_1_0_3(), currentNode); 
	    }
		lv_param_1_4=ruleWMLKey		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLMacroParameterRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode.getParent(), $current);
	        }
	        try {
	       		add(
	       			$current, 
	       			"param",
	        		lv_param_1_4, 
	        		"WMLKey", 
	        		currentNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	        currentNode = currentNode.getParent();
	    }

)

)
)+	')' 
    {
        createLeafNode(grammarAccess.getWMLMacroParameterAccess().getRightParenthesisKeyword_2(), null); 
    }
)
;





// Entry rule entryRuleWMLLuaCode
entryRuleWMLLuaCode returns [EObject current=null] 
	:
	{ currentNode = createCompositeNode(grammarAccess.getWMLLuaCodeRule(), currentNode); }
	 iv_ruleWMLLuaCode=ruleWMLLuaCode 
	 { $current=$iv_ruleWMLLuaCode.current; } 
	 EOF 
;

// Rule WMLLuaCode
ruleWMLLuaCode returns [EObject current=null] 
    @init { EObject temp=null; setCurrentLookahead(); resetLookahead(); 
    }
    @after { resetLookahead(); 
    	lastConsumedNode = currentNode;
    }:
(
(
		lv_value_0_0=RULE_LUA_CODE
		{
			createLeafNode(grammarAccess.getWMLLuaCodeAccess().getValueLUA_CODETerminalRuleCall_0(), "value"); 
		}
		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLLuaCodeRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode, $current);
	        }
	        try {
	       		set(
	       			$current, 
	       			"value",
	        		lv_value_0_0, 
	        		"LUA_CODE", 
	        		lastConsumedNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	    }

)
)
;





// Entry rule entryRuleWMLArrayCall
entryRuleWMLArrayCall returns [EObject current=null] 
	:
	{ currentNode = createCompositeNode(grammarAccess.getWMLArrayCallRule(), currentNode); }
	 iv_ruleWMLArrayCall=ruleWMLArrayCall 
	 { $current=$iv_ruleWMLArrayCall.current; } 
	 EOF 
;

// Rule WMLArrayCall
ruleWMLArrayCall returns [EObject current=null] 
    @init { EObject temp=null; setCurrentLookahead(); resetLookahead(); 
    }
    @after { resetLookahead(); 
    	lastConsumedNode = currentNode;
    }:
(	'[' 
    {
        createLeafNode(grammarAccess.getWMLArrayCallAccess().getLeftSquareBracketKeyword_0(), null); 
    }
(
(
		{ 
	        currentNode=createCompositeNode(grammarAccess.getWMLArrayCallAccess().getValueWMLValueParserRuleCall_1_0(), currentNode); 
	    }
		lv_value_1_0=ruleWMLValue		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLArrayCallRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode.getParent(), $current);
	        }
	        try {
	       		add(
	       			$current, 
	       			"value",
	        		lv_value_1_0, 
	        		"WMLValue", 
	        		currentNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	        currentNode = currentNode.getParent();
	    }

)
)+	']' 
    {
        createLeafNode(grammarAccess.getWMLArrayCallAccess().getRightSquareBracketKeyword_2(), null); 
    }
)
;





// Entry rule entryRuleWMLMacroDefine
entryRuleWMLMacroDefine returns [EObject current=null] 
	:
	{ currentNode = createCompositeNode(grammarAccess.getWMLMacroDefineRule(), currentNode); }
	 iv_ruleWMLMacroDefine=ruleWMLMacroDefine 
	 { $current=$iv_ruleWMLMacroDefine.current; } 
	 EOF 
;

// Rule WMLMacroDefine
ruleWMLMacroDefine returns [EObject current=null] 
    @init { EObject temp=null; setCurrentLookahead(); resetLookahead(); 
    }
    @after { resetLookahead(); 
    	lastConsumedNode = currentNode;
    }:
(
(
		lv_name_0_0=RULE_DEFINE
		{
			createLeafNode(grammarAccess.getWMLMacroDefineAccess().getNameDEFINETerminalRuleCall_0(), "name"); 
		}
		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLMacroDefineRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode, $current);
	        }
	        try {
	       		set(
	       			$current, 
	       			"name",
	        		lv_name_0_0, 
	        		"DEFINE", 
	        		lastConsumedNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	    }

)
)
;





// Entry rule entryRuleWMLTextdomain
entryRuleWMLTextdomain returns [EObject current=null] 
	:
	{ currentNode = createCompositeNode(grammarAccess.getWMLTextdomainRule(), currentNode); }
	 iv_ruleWMLTextdomain=ruleWMLTextdomain 
	 { $current=$iv_ruleWMLTextdomain.current; } 
	 EOF 
;

// Rule WMLTextdomain
ruleWMLTextdomain returns [EObject current=null] 
    @init { EObject temp=null; setCurrentLookahead(); resetLookahead(); 
    }
    @after { resetLookahead(); 
    	lastConsumedNode = currentNode;
    }:
(
(
		lv_name_0_0=RULE_TEXTDOMAIN
		{
			createLeafNode(grammarAccess.getWMLTextdomainAccess().getNameTEXTDOMAINTerminalRuleCall_0(), "name"); 
		}
		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLTextdomainRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode, $current);
	        }
	        try {
	       		set(
	       			$current, 
	       			"name",
	        		lv_name_0_0, 
	        		"TEXTDOMAIN", 
	        		lastConsumedNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	    }

)
)
;





// Entry rule entryRuleWMLValue
entryRuleWMLValue returns [EObject current=null] 
	:
	{ currentNode = createCompositeNode(grammarAccess.getWMLValueRule(), currentNode); }
	 iv_ruleWMLValue=ruleWMLValue 
	 { $current=$iv_ruleWMLValue.current; } 
	 EOF 
;

// Rule WMLValue
ruleWMLValue returns [EObject current=null] 
    @init { EObject temp=null; setCurrentLookahead(); resetLookahead(); 
    }
    @after { resetLookahead(); 
    	lastConsumedNode = currentNode;
    }:
(
(
(
		lv_value_0_1=RULE_ID
		{
			createLeafNode(grammarAccess.getWMLValueAccess().getValueIDTerminalRuleCall_0_0(), "value"); 
		}
		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLValueRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode, $current);
	        }
	        try {
	       		set(
	       			$current, 
	       			"value",
	        		lv_value_0_1, 
	        		"ID", 
	        		lastConsumedNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	    }

    |		lv_value_0_2=RULE_STRING
		{
			createLeafNode(grammarAccess.getWMLValueAccess().getValueSTRINGTerminalRuleCall_0_1(), "value"); 
		}
		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLValueRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode, $current);
	        }
	        try {
	       		set(
	       			$current, 
	       			"value",
	        		lv_value_0_2, 
	        		"STRING", 
	        		lastConsumedNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	    }

    |		lv_value_0_3=	'+' 
    {
        createLeafNode(grammarAccess.getWMLValueAccess().getValuePlusSignKeyword_0_2(), "value"); 
    }
 
	    {
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLValueRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode, $current);
	        }
	        
	        try {
	       		set($current, "value", lv_value_0_3, null, lastConsumedNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	    }

    |		lv_value_0_4=	'~' 
    {
        createLeafNode(grammarAccess.getWMLValueAccess().getValueTildeKeyword_0_3(), "value"); 
    }
 
	    {
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLValueRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode, $current);
	        }
	        
	        try {
	       		set($current, "value", lv_value_0_4, null, lastConsumedNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	    }

    |		lv_value_0_5=RULE_ANY_OTHER
		{
			createLeafNode(grammarAccess.getWMLValueAccess().getValueANY_OTHERTerminalRuleCall_0_4(), "value"); 
		}
		{
	        if ($current==null) {
	            $current = factory.create(grammarAccess.getWMLValueRule().getType().getClassifier());
	            associateNodeWithAstElement(currentNode, $current);
	        }
	        try {
	       		set(
	       			$current, 
	       			"value",
	        		lv_value_0_5, 
	        		"ANY_OTHER", 
	        		lastConsumedNode);
	        } catch (ValueConverterException vce) {
				handleValueConverterException(vce);
	        }
	    }

)

)
)
;





RULE_LUA_CODE : '<<' ( options {greedy=false;} : . )*'>>';

RULE_DEFINE : '#define' ( options {greedy=false;} : . )*'#enddef';

RULE_TEXTDOMAIN : '#textdomain' ~(('\n'|'\r'))* ('\r'? '\n')?;

RULE_STRING : '"' ('\\' ('b'|'t'|'n'|'f'|'r'|'"'|'\''|'\\')|~(('\\'|'"')))* '"';

RULE_ID : ('a'..'z'|'A'..'Z'|'0'..'9'|'_'|',')+;

RULE_EOL : ('\r'|'\n')+;

RULE_WS : (' '|'\t')+;

RULE_ANY_OTHER : .;

RULE_SL_COMMENT : '#' ~(('\n'|'\r'))* ('\r'? '\n')?;


