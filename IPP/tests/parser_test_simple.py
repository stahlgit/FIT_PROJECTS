import xml.etree.ElementTree as ET
from src.parser import Parser
#            x := self compute: 3 and: 2 and: 5.

if __name__ == "__main__":
    source_code = """
    class Main : Object {
        run [|
            x := 3.
        ]
    }
    """
    parser = Parser()
    ast = parser.parse(source_code)
    root = ast.to_xml()
    tree = ET.ElementTree(root)
    ET.indent(tree, space="  ") 
    xml_string = ET.tostring(root, encoding='unicode')

    print(xml_string)