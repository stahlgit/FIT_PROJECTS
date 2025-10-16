# Documentation for SOL25 Parser Codebase

## **Design Philosophy**
The design philosophy was to linearly scan the code to do syntactic analysis and create abstract syntax tree, on which semantic analysis is then performed. If everything is done without problems, the ast will be printed in xml format. This implementation was heavily adapted to use the python library PLY. Instead of having specific XML generator that iterates whole AST, there is only 1 functio (to_xml), which iterates whole tree. Tree is specially design to coalign with xml representation. 

## **Internal Representation**

- **Lexer** - for lexical analysis using PLY 
- **Parser** - for syntax analysis using PLY and converting tokens into AST Nodes. 
- **SemanticAnalyzer** - for semnatic analysis, iterates through created AST

The internal representation revolves around the Abstract Syntax Tree (AST), which models the structure of the source code. Key classes include:
- **ASTNode**: The base class for all AST nodes.
- **ClassNode**: Represents a class definition with its name, parent class, and methods.
- **MethodNode**: Represents a method with its selector and body.
- **BlockNode**: Represents a block of code with parameters and statements.
- **AssignNode**: Represents variable assignments.
- **VarNode**: Represents variables used in expressions.
- **SendNode**: Represents method calls with arguments.

Each node has attributes to store relevant data and methods to convert itself into XML for output.

## **Specific Approach**
1. **Lexical Analysis**:
   - The `Lexer` class tokenizes the input source code using PLY (Python Lex-Yacc). It defines tokens such as identifiers, keywords, operators, and literals.
   - Error handling ensures illegal characters are identified and reported.

2. **Parsing**:
   - The `Parser` class uses a top-down approach with PLY's YACC module to parse the tokenized input. It constructs the AST based on grammar rules defined for SOL25. PLY exports there rules into file called `parser.out`, where you can find all the grammer rules automatically generated. 

3. **AST Nodes**:
   - AST nodes are specifically called as their XML counter part. So you can find node called ArgNode instead of ArgumentNode, but it contains all the necessary attributes. 
   - Class uses object-oriented programming principles to recursively branch, where occasionally overwrites it's method for specific handling. 

4. **Semantic Analysis**:
   - The `SemanticAnalyzer` validates the AST by checking for undefined variables, incorrect method arities, missing main classes or methods, and other semantic errors.
   - Symbol tables are used to track defined variables and methods within different scopes.

## **UML Diagram of Classes**
Below is the UML class diagram represented using Mermaid syntax:
```Mermaid
classDiagram
class ClassNode {
    +name : str
    +parent : str
    +methods : list
}
ClassNode --> ASTNode

class MethodNode {
    +selector : str
    +body : BlockNode
}
MethodNode --> ASTNode

class BlockNode {
    +parameters : list
    +statements : list
}
BlockNode --> ASTNode

class AssignNode {
    +target : VarNode
    +value : ExprNode
}
AssignNode --> ASTNode

class VarNode {
    +name : str
}
VarNode --> ASTNode

class SendNode {
    +target : ExprNode
    +selector : str
    +arguments : list
}
SendNode --> ASTNode

class Lexer {
    +tokens : list
    +input(source_code)
    +token()
    +tokenize(source_code)
}

class Parser {
    +parse(source_code)
}

class SemanticAnalyzer {
    +analyze()
}

Lexer --> Parser
Parser --> SemanticAnalyzer
Parser --> ASTNode
ASTNode --> Parser
```