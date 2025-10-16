from src.lexer import Lexer
from src.parser import Parser
import xml.etree.ElementTree as ET


source_code = """
    class Main : Object {
        run [ |
            z := 'Ahoj\n \'\' Sve"te \ '.
        ]
    }
"""
lexer = Lexer()
tokens = lexer.tokenize(source_code)


for token in tokens:
    print(token)
    if token.type == 'STRING':
        print(repr(token.value)) # print the string using repr.
        
parser = Parser()
ast = parser.parse(source_code)
root = ast.to_xml()
tree = ET.ElementTree(root)
ET.indent(tree, space="  ") 
xml_string = ET.tostring(root, encoding='unicode')

print(xml_string)