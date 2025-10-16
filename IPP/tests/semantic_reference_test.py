import xml.etree.ElementTree as ET
from src.parser import Parser
from src.semantic_analyzer import SemanticAnalyzer

if __name__ == "__main__":
    source_code = """
    class Main : Object {
        run [|
            a := A new.
            b := [ :arg | y := self attr: arg. z := self.].
            c := a foo: b.
        ]
    }
    
    class A : Object {
        foo: [:x| 
            u := x value: 1.
        ]
        }
    """
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
