import ply.lex as lex
from src.lib.tokens import lexer_tokens

class Lexer:
    """Tokenizes SOL25 source code using PLY"""

    # List of token names
    tokens = lexer_tokens

    # Regular expression rules for simple tokens
    t_ASSIGN   = r':='
    t_COLON    = r':'
    t_DOT      = r'\.'
    t_PIPE     = r'\|'
    t_LPAREN   = r'\('
    t_RPAREN   = r'\)'
    t_LBRACE   = r'\{'
    t_RBRACE   = r'\}'
    t_LBRACKET = r'\['
    t_RBRACKET = r'\]'

    # Ignore whitespace
    t_ignore = ' \t\r'

    # Function for identifiers and keywords
    def t_IDENTIFIER(self, t):
        r'[a-zA-Z_][a-zA-Z0-9_]*'
        keywords = {
            'class': 'CLASS',
            'true': 'TRUE',
            'false': 'FALSE',
            'nil': 'NIL'
        }
        t.type = keywords.get(t.value, 'IDENTIFIER')  # Convert to keyword type if matched
        return t

    # String literals (single quotes)
    def t_STRING(self, t):
        r"'((?:[^'\\]|\\.)*)'"  # Explicitly handle escaped quotes
        t.value = t.value[1:-1]  # Strip surrounding quotes
        t.value = t.value.encode().decode('unicode_escape')  # Process escapes
        return t

    # Integer literals
    def t_INTEGER(self, t):
        r'\d+'
        t.value = int(t.value)  # Convert string to int
        return t

    # COMMENT (double quotes)
    def t_COMMENT(self, t):
        r'\"([^\\"]|\\.)*\"'
        t.value = t.value[1:-1]  # Remove surrounding quotes
        return t


    # Track line numbers
    def t_newline(self, t):
        r'\n+'
        t.lexer.lineno += len(t.value)

    # Error handling for invalid characters
    def t_error(self, t):
        print(f"Illegal character '{t.value[0]}' at line {t.lexer.lineno}")
        t.lexer.skip(1)
        raise KeyError("21") # Lexical Error

    def __init__(self):
        """Initialize the PLY lexer"""
        self._lexer = lex.lex(module=self)
        
    def input(self, source_code):
        """Sets the input text for the lexer."""
        self._lexer.input(source_code)
    
    def token(self):
        """Returns the next token from the input."""
        return self._lexer.token()

    def tokenize(self, source_code):
        """Tokenizes the given SOL25 source code"""
        self._lexer.input(source_code)
        return list(self._lexer)
