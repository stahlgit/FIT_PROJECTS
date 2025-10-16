import xml.etree.ElementTree as ET
from src.parser import Parser

if __name__ == "__main__":
    source_code = """
        class Main : Object {
        run [|
            x := self plusOne: (self vysl).
        ]
        
        plusOne:
            [ :x | r := x plus: 1. ]
    }
    """
    
    try:
        parser = Parser()
        ast = parser.parse(source_code)
        root = ast.to_xml()
        tree = ET.ElementTree(root)
        ET.indent(tree, space="  ") 
        xml_string = ET.tostring(root, encoding='unicode')

        print(xml_string)
    except ValueError as e:
        print(e)