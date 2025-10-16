import xml.etree.ElementTree as ET
from src.lexer import Lexer
from src.parser import Parser
from src.semantic_analyzer import SemanticAnalyzer

if __name__ == "__main__":
    source_code = """
    class Main : Object {
        run [|
            y := [| ret := (self attr) greatherThan: 0.] whileTrue:
            [| r := ((self attr) asString) print.
                r := self attr: ((self attr) minus: 1).].
        ]
    }
    """
    
    lexer = Lexer()
    tokens = lexer.tokenize(source_code)
    
    for token in tokens:
        print(token)
    
    
    parser = Parser()
    ast = parser.parse(source_code)
    
    analyzer = SemanticAnalyzer(ast)
    try:
        analyzer.analyze()
        print("Semantic analysis successful!")
    except Exception as e:
        print(e)

    # print(ast)
    root = ast.to_xml()
    tree = ET.ElementTree(root)
    ET.indent(tree, space="  ") 
    xml_string = ET.tostring(root, encoding='unicode')
    
    print(xml_string)
