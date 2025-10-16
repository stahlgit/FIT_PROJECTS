import xml.etree.ElementTree as ET
from src.parser import Parser
from src.semantic_analyzer import SemanticAnalyzer

if __name__ == "__main__":
    source_code = """
    class Main : Object {
        run [|
            b1 := [| a := String read. _ := a print. ].
            b2 := [:x | _ := x plus: 1.].
            b3 := [:x:y| val := x value: y.].
            _ := b1 value.
            c := b3 value: b2 value: 3.
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
    
    #print(xml_string)
