import xml.etree.ElementTree as ET
from src.parser import Parser

if __name__ == "__main__":
    full_code = """
    class Main : Object {
        run [|
            x := self compute: 3 and: 2 and: 5.
            x := self plusOne: (self vysl).
            y := x asString.
        ]
        
        plusOne:
            [ :x | r := x plus: 1. ]
            
        compute:and:and:    
            [ :x :y :z | 
            a := x plus: y.
            _ := self vysl: a.
            
            _ := ((self vysl) greaterThen: 0)
                ifTrue: [|u := self vysl: 1.]
                ifFalse: [|].
            ]
    }
    """
    
    source_code = """
        class Main : Object {
        run [|
            x := self plusOne: (self vysl).
        ]
        
        plusOne:
            [ :x | r := x plus: 1. ]
    }
    """

    parser = Parser()
    ast = parser.parse(full_code)
    root = ast.to_xml()
    tree = ET.ElementTree(root)
    ET.indent(tree, space="  ") 
    xml_string = ET.tostring(root, encoding='unicode')

    print(xml_string)