import xml.etree.ElementTree as ET

class ASTNode:
    """Base class for AST nodes."""
    def __repr__(self) -> str:
        attributes = ", ".join(f"{key}={value}" for key, value in self.__dict__.items())
        return f"{self.__class__.__name__}({attributes})"
    
    def _get_xml_tag(self) -> str:
        """Returns the XML tag name, removing 'Node' suffix and converting to lowercase."""
        return self.__class__.__name__.replace("Node", "").lower()
    
    def _is_attribute(self, key) -> bool:
        """Determines which fields should be stored as XML attributes instead of child elements.
        Override in subclasses for more customization."""
        return key in {"name", "selector", "order", "parent", "arity", "language", "value", "class"} 
    
    def to_xml(self, parent=None) -> ET.Element:
        """Converts the node to an XML element.
        Args:
            parent: The parent XML element to attach this node to.
                    If None, a new XML element is created.
        Returns:
            an ElementTree Element representing node in XML format.
        """
        
        # Creates an XML element with the tag name of the class
        element = ET.Element(self._get_xml_tag()) 
        # Maps attribute names to XML element names
        key_map = {"class_name": "class"}
        
        # Iterates over all attributes of the current node
        for key, value in self.__dict__.items():
            #  Map key to a different name 
            key = key_map.get(key, key) 
            # If value is another ASTNode, recursively convert it to XML
            if isinstance(value, ASTNode):
                value.to_xml(element)
            # if value is a list, handle each item separately
            elif isinstance(value, list):
                for item in value:
                    if isinstance(item, ASTNode):
                        # If the item is an ASTNode, convert it to XML recursively
                        item.to_xml(element)
                    else:
                        # create a child element for non-AST list items
                        ET.SubElement(element, key).text = str(item)    
            elif self._is_attribute(key):
                # If the key is in the list of attributes, store it as an attribute
                element.set(key, str(value))
            else:
                ET.SubElement(element, key).text = str(value)
        
        # If a parent XML element is provided, attach this element to it
        if parent is not None:
            parent.append(element)
        return element


class ProgramNode(ASTNode):
    def __init__(self, classes):
        self.classes = classes 
        self.language = "SOL25"
        
class ClassNode(ASTNode):
    def __init__(self, name, parent, methods):
        self.name = name
        self.parent = parent
        self.methods = methods

class MethodNode(ASTNode):
    def __init__(self, selector, body):
        self.selector = selector
        self.body = body
        """
        one way how to check the arity of the method: 

        if self.selector.count(":") != len(self.body.parameter):
            raise ValueError("33")
        """
        
class ParenthesesNode(ASTNode):
    def __init__(self, expression):
        self.expression = expression

    def to_xml(self, parent=None)->ET.Element:
        # When converting to XML, simply output the inner expression.
        return self.expression.to_xml(parent)

class BlockNode(ASTNode):
    def __init__(self, parameter, statements):
        self.parameter = parameter
        self.statements = statements
        self.arity = len(parameter)

class ParameterNode(ASTNode):
    def __init__(self, name, order):
        self.name = name
        self.order = order

class AssignNode(ASTNode):
    def __init__(self, target, value):
        self.target = target
        self.value = value
        self.order = None

class SendNode(ASTNode):
    def __init__(self, target, selector, arguments):
        self.target = target
        self.selector = selector
        self.arguments = arguments

class VarNode(ASTNode):
    def __init__(self, name):
        self.name = name

class ArgNode(ASTNode):
    def __init__(self, expression, order:int):
        self.expression = expression
        self.order = order

class ExprNode(ASTNode):
    def __init__(self, expression):
        self.expression = expression
        
class LiteralNode(ASTNode):
    def __init__ (self, class_name, value):
        self.class_name = class_name
        self.value = value if value is not None else ""  