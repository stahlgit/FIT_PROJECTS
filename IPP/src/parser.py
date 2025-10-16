import ply.yacc as yacc #TOP DOWN PARSER
from src.lexer import Lexer
from src.lib.ast_nodes import *
from src.lib.tokens import lexer_tokens

class Parser:
    """Parses SOL25 """
    
    tokens = lexer_tokens
    
    def __init__(self):
        """Initialize the lexer and parser."""
        self.lexer = Lexer()
        self._parser = yacc.yacc(module=self)
        
    def parse(self, source_code):
        """Parse the given SOL25 source code."""
        return self._parser.parse(source_code, lexer=self.lexer)
    
    """Grammar rules for SOL25"""
    def p_program(self, p):
        """program : class_list"""
        p[0] = ProgramNode(p[1]) 
        
    def p_class_list(self, p):
        """class_list : class_list class
                        | class"""
        if len(p) == 3:
            p[0] = p[1] + [p[2]] # append classes to existing list
        else:
            p[0] = [p[1]] # create list with single class

    def p_class(self, p):
        """class : CLASS IDENTIFIER COLON IDENTIFIER LBRACE method_list RBRACE"""
        p[0] = ClassNode(p[2], p[4], p[6])

    def p_method_list(self, p):
        """method_list : method_list method
                        | method"""
        if len(p) == 3:
            p[0] = p[1] + [p[2]] # append methods to existing list
        else:
            p[0] = [p[1]] # create list with single method

    def p_method(self, p):
        """method : IDENTIFIER block
                    | method_name block"""
        p[0] = MethodNode(p[1], p[2]) # MethodNode(ID, BlockNode)
    
    def p_method_name(self, p):
        """method_name : IDENTIFIER COLON
                        | method_name IDENTIFIER COLON"""
        if len(p) == 3:
            p[0] = p[1] + p[2] # ID + COLON
        else:
            # recursive call
            p[0] = p[1] + p[2] + p[3] # Append selector: method_name + ID + COLON
        
    def p_param_list(self, p):
        """param_list : param_list COLON IDENTIFIER
                        | COLON IDENTIFIER
                        | empty"""
        if len(p) == 4:
            order=len(p[1]) + 1
            p[0] = p[1] + [ParameterNode(p[3], order)] # Append parameter
        elif len(p) == 3:
            p[0] = [ParameterNode(p[2], 1)] # Single parameter
        else:
            p[0] = [] # No parameters

    def p_block(self, p):
        """block : LBRACKET block_params PIPE statement_list RBRACKET"""
        order = 1
        for stmt in p[4]:
            if isinstance(stmt, AssignNode):
                stmt.order = order
                order += 1 # Increment order for each statement
        p[0] = BlockNode(p[2], p[4]) # BlockNode(Parameters, Statements)
        
    def p_block_params(self, p):
        """block_params : param_list
                        | empty"""
        p[0] = p[1] # pass parameter list to block_params

    def p_statement_list(self, p):
        """statement_list : statement_list statement
                            | statement
                            | empty"""
        if len(p) == 3:
            p[0] = p[1] + [p[2]]
        else:
            p[0] = [p[1]] if p[1] is not None else []
        
    def p_statement(self, p):
        """statement : assignment"""
        p[0] = p[1]

    def p_assignment(self, p):
        """assignment : IDENTIFIER ASSIGN expression DOT"""
        p[0] = AssignNode(VarNode(p[1]), ExprNode(p[3]))

    def p_expression(self, p):
        """expression : primary
                    | instruction"""
        p[0] = p[1]
        
    def p_instruction(self, p):
        """instruction : primary IDENTIFIER COLON primary    
                    | instruction IDENTIFIER COLON primary 
                    | primary IDENTIFIER
                    | instruction IDENTIFIER COLON paren_expression
                    | primary IDENTIFIER COLON paren_expression
                    | paren_expression IDENTIFIER COLON primary
                    | paren_expression IDENTIFIER 
                    | paren_expression"""
        if len(p) == 5:
            
            # Chained or new message with argument
            if isinstance(p[1], SendNode):
                # Existing message chain: append new argument
                prev = p[1]
                selector = f"{prev.selector}{p[2]}:"
                new_order = len(prev.arguments) + 1
                args = prev.arguments + [ArgNode(ExprNode(p[4]), new_order)]
                p[0] = SendNode(prev.target, selector, args)
            else:
                # New message with argument: order = 1 
                selector = f"{p[2]}:"
                args = [ArgNode(p[4], 1)]
                p[0] = SendNode(p[1], selector, args)
        else:
            # Unary message (no arguments)
            selector = f"{p[2]}"
            p[0] = SendNode(p[1], selector, [])
            
    def p_primary(self, p):
        """primary : INTEGER
                    | IDENTIFIER
                    | TRUE
                    | FALSE
                    | STRING
                    | NIL
                    | block
                    | concatenated_string"""
        if len(p) == 4:
            p[0] = p[2]
        elif isinstance(p[1], int):
            p[0] = LiteralNode("Integer", p[1])
        elif isinstance(p[1], str):
            if p[1] == "true":
                p[0] = LiteralNode("Boolean", True)
            elif p[1] == "false":
                p[0] = LiteralNode("Boolean", False)
            elif p[1] == "nil":
                p[0] = LiteralNode("Nil", None)
            else:
                if p[1] == "Integer":
                    p[0] = ExprNode(LiteralNode("class", p[1])) # by example from documenation
                else:
                    p[0] = ExprNode(VarNode(p[1]))
        else:
            p[0] = p[1]  # BlockNode

    def p_concatenated_string(self, p):
        """concatenated_string : STRING STRING
                            | concatenated_string STRING"""
        if len(p) == 3:
            # Concatenate strings
            p[0] = LiteralNode("String", p[1] + p[2])
        else:
            p[0] = LiteralNode("String", p[1])


    def p_paren_expression(self, p):
        """paren_expression : LPAREN expression RPAREN"""
        p[0] = ParenthesesNode(ExprNode(p[2]))
            
    def p_empty(self, p):
        """empty :"""
        p[0] = None

    def p_error(self, p):
        if p:
            raise SyntaxError(f"Syntax error at '{p.value}', line {p.lineno}")
        else:
            raise SyntaxError("Syntax error at end of input")
            
