from src.lib.ast_nodes import ASTNode, ClassNode, MethodNode, VarNode


class SemanticAnalyzer:
    def __init__(self, ast):
        self.ast = ast # inputed Abstract Syntax Tree to analyze
        self.errors = [] # List of errors found during analysis
        self.symbols = {"global": self._get_global_symbols()}   # Stores defined variables per method/class scop
        self.defined_methods = set(self._get_builtin_methods()) # Track defined methods
        self.defined_classes = set(self._get_builtin_classes()) # Track defined classes
    
    def _get_global_symbols(self):
        """Define global symbols that are always available."""
        return {
            "String": True,
            "Integer": True,
            "nil": True,
            "self": True,  
            
        }
    def _get_builtin_methods(self):
        return {
            "new", "from:", "attr:", "attr", "next:",
            "identicalTo:","equalTo:",
            "greaterThen:", "plus:", "minus:", "multiplyBy:",
            "divBy:", 
            "asString", "asInteger"
            "isBlock", "isInteger", "isString", "isNil",
            "timesRepeat:",
            "vysl:",  "ifTrue:", "ifFalse:", "value:", 
            "read", "print",
            "concatenateWith:", 
            "startsWith:endsBefore:", 
            "whileTrue:"
            "not", "and:", "or:"
            "ifTrue:ifFalse:", 
            "attrib", "attrib:"
        }
    
    def _get_builtin_classes(self):
        return {
            "Object", "Nil", "True", "False", "Integer", "String", "Block"
        }

    def analyze(self):
        """Main entry point for semantic analysis."""
        self._check_main_class()
        self.__colect_definitions(self.ast)
        self._visit(self.ast)
        if self.errors:
            for error in self.errors:
                raise Exception(error)

    def _check_main_class(self):
        """Ensure Main class exists with a run method."""
        main_class = None  
        for cls in self.ast.classes:
            if cls.name == "Main":
                main_class = cls
                break
        
        if not main_class:
            self.errors.append(31)
            return
        # Check for run method in Main class
        has_run = any(method.selector == "run" for method in main_class.methods)
        if not has_run:
            self.errors.append(31)
            
    def __colect_definitions(self, node):
        if isinstance(node, ClassNode):
            self.defined_classes.add(node.name)
        
        if isinstance(node, MethodNode):
            self.defined_methods.add(node.selector) 

        # Recursively visit child nodes
        for attr, value in node.__dict__.items():
            if isinstance(value, list):
                for item in value:
                    if isinstance(item, ASTNode):
                        self.__colect_definitions(item)
            elif isinstance(value, ASTNode):
                self.__colect_definitions(value)

    def _visit(self, node):
        """Recursively visit AST nodes."""
        method = getattr(self, f"_visit_{type(node).__name__}", self._generic_visit)
        return method(node)

    def _generic_visit(self, node):
        """Default visit method that traverses child nodes."""
        for attr, value in node.__dict__.items():
            if isinstance(value, list):
                for item in value:
                    if isinstance(item, ASTNode):
                        self._visit(item)
            elif isinstance(value, ASTNode):
                self._visit(value)

    
    def _visit_ClassNode(self, node):
        """Process class definitions."""
        if node.parent not in self.defined_classes:
            self.errors.append(35)
        self.symbols[node.name] = {}  
        self._generic_visit(node)

    def _visit_MethodNode(self, node):
        """Process method definitions."""
        self.defined_methods.add(node.selector)
        
        method_scope = self.symbols.setdefault(node.selector, {})
        param_names = {param.name for param in node.body.parameter}
        
        # Detect variable name collisions
        if len(param_names) < len(node.body.parameter):
            self.errors.append(34)

        # Check arity
        expected_arity = node.selector.count(":")
        
        if expected_arity != len(node.body.parameter):
            self.errors.append(33)

        # Add parameters to symbol table
        for param in node.body.parameter:
            method_scope[param.name] = True
        
        self.visit(node.body)

    def _visit_BlockNode(self, node):
        """Process block scope and track declared parameters."""
        block_scope = self.symbols.setdefault("block", {})

        for param in node.parameter:
            block_scope[param.name] = True  # Mark parameters as initialized

        for statement in node.statements:
            self.visit(statement)


    def _visit_AssignNode(self, node):
        """Process variable assignments."""
        method_scope = self.symbols.setdefault("global", {})
        if isinstance(node.target, VarNode):
            method_scope[node.target.name] = True  # Declare variable

        self.visit(node.value)

    def _visit_VarNode(self, node):
        """Check for uninitialized variable usage."""
        if node.name not in self.symbols.get("global", {}) and node.name not in self.symbols.get("block", {}):
            # It's possible to check this just in defined classes, because it was method was already checked
            if node.name not in self.defined_classes: 
                self.errors.append(32)
            
    def _visit_SendNode(self, node):
        """Check method calls for correct arity."""
        selector = node.selector
        selector_parts = selector.split(":")
        expected_arity = len(selector_parts) - 1
        
        actual_arity = len(node.arguments)

        if expected_arity != actual_arity:
            self.errors.append(33)
        

        if node.selector == "value:":
            target = node.target.expression

            if isinstance(target, VarNode):
                if target.name == "self":
                    pass
                if target.name not in self.symbols.get("block", {}) and target.name not in self.symbols.get("global", {}):
                    self.errors.append(35)  # Undefined block variable
                    
        # sending value (to block for example)
        elif node.selector == "value":
            target = node.target.expression
            # check if target exists
            if target.name not in self.symbols.get("block", {}) and target.name not in self.symbols.get("global", {}):
                self.errors.append(35)

                
        elif node.selector not in self.defined_methods:
            if expected_arity > 1:
                # chained method call
                for i in range(1, expected_arity):
                    if f"{selector_parts[i]}:" not in self.defined_methods:
                        self.errors.append(35)
            else:
                self.errors.append(35)

        elif node.selector == "new":
            if node.target.expression.name not in self.defined_classes:
                return self.errors.append(32)
            
        self._visit(node.target)
        for arg in node.arguments:
            self._visit(arg.expression)
