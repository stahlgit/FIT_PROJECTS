import xml.etree.ElementTree as ET
from src.parser import Parser
from src.semantic_analyzer import SemanticAnalyzer

if __name__ == "__main__":
    source_code = """
    class Main : Object {
        run [|
            x := self compute: 3 and: 2 and: 5.
            c := true.
        ]
        
        compute:and:and:    
            [ :x :y :z | 
            a := x plus: y.
            _ := self vysl: a.
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
    
    # print(xml_string)
