### Built in classes

```mermaid
classDiagram
    class Object {
        name : string
        parent: ClassObject
        identicalTo:()
        equalTo:()
        asString()
        isNumber()
        isBlock()
        isNil()
    }

    class Block {
        isBlock() <!-- Mentioned only when it's overwritten -->
        whileTrue:()
    }

    class False {
        not()
        and()
        or()
        ifTrue:ifFalse:()
    }

    class True {
        not()
        and()
        or()
        ifTrue:ifFalse:()
    }

    class Integer{
        equalTo:()
        asString()
        isNumber()
        greaterThan:()
        plus:()
        minus:()
        multiplyBy:()
        divBy:()
        asInteger()
        timesRepeat:()
    }

    class Nil{
        isNil()
        isString()
    }

    class String{
        equalTo:()
        isString()
        asString()
        asInteger()
        read()
        print()
        concatenateWith:()
        startWith:endsBefore:()

    }

    Object <|-- Block 
    Object <|-- False
    Object <|-- True
    Object <|-- Integer
    Object <|-- Nil
    Object <|-- String

```


### AST architecture 

```mermaid
classDiagram

class ClassObject{
    name : string
    parent : ?ClassObject
    methods: [ MethodObject ]

    addMethod(MethodObject method)
    findMehtod(string selector)
}

class MethodObject{
    selector : string
    block : BlockObject
}

class BlockObject{
    arity : int
    parameters: [ ParameterObject ]
    assignments : [ AssignmentObject ]
    addParameter(ParameterObject paramemter)
    addAssignment(AssignmentObject assignment)
    execute(?Instance $receiver, array args, ?Scope parentScope)


}

class ParameterObject{
    order : int
    name : string
}


class AssignmentObject{
    variable : VariableExpression 
    expression : ExpressionInterface
    assign(Scope scope)
}

class ExpressionInterface{
    evaluate(Scope scope)
}


class LiteralExpression {
    type : string
    value : string
    evaluate(Scope scope)
}

class SendExpression {
    selector : string
    receiver : ExpressionInterface
    arguments

}

class VariableExpression {
    name : string
    evaluate(Scope scope)
}

class BlockExpression {
    block : BlockObject
    evaluate(Scope scope)
}

ClassObject <-- MethodObject
MethodObject <-- BlockObject
BlockObject <-- ParameterObject
BlockObject <-- AssignmentObject
AssignmentObject <-- ExpressionInterface
AssignmentObject <-- VariableExpression

ExpressionInterface <|-- LiteralExpression
ExpressionInterface <|-- SendExpression
ExpressionInterface <|-- VariableExpression
ExpressionInterface <|-- BlockExpression 
BlockObject --> BlockExpression 
```


#### Runtime Architecture
Get Instance and instance is used for singletons (I think) --> aka when is just one and same instance of that 
```mermaid
classDiagram
    class Instance {
        ClassObject $class
        array $attributes
        send(string $selector, array $args)
    }

    class BlockInstance {
        send()
        handleBuiltIns()
    }

    class TrueInstance {
        private instance 
        send()
        getInstance()
    }

    class FalseInstance {
        private instance 
        send()
        getInstance()
    }

    class IntegerInstance {
        int $value
        checkInstance()
        checkBlock()
        handleBuiltIn
        send()
    }

    class StringInstance {
        string $value
        handleBuiltIn()
        send()
        startWithEndsBefore()s
    }

    class NilInstance {
        instance
        getInstance()
    }

    Instance <|-- BlockInstance
    Instance <|-- TrueInstance
    Instance <|-- FalseInstance
    Instance <|-- IntegerInstance
    Instance <|-- StringInstance
    Instance <|-- NilInstance
```
Instance Creation

1. Built-in Instances (True/False/Nil/Block):
 - Singletons created via getInstance() (e.g., T-rueInstance::getInstance()).
 - Initialized once during interpreter setup.

2. Primitive Instances (Integer/String):
 - Created via BuiltInFactory::create(type, value).
 - Example: BuiltInFactory::create('Integer', '5').

3. User-Defined Instances:
- Created via new or from: methods:
```console
$instance = $classObject->send('new', []); // Default initialization
$instance = $classObject->send('from:', [$argInstance]); // With value
```

The runtime system handles object instantiation, method dispatch, block execution, and scope management through several key components working together. The execution flow starts with the Main class' run method and propagates through nested message sends.


```mermaid
sequenceDiagram
    participant Sender
    participant Instance
    participant ClassObject
    participant MethodObject
    participant BlockObject

    Sender->>Instance: send("selector", args)
    alt Method exists in class
        Instance->>ClassObject: findMethod("selector")
        ClassObject->>MethodObject: get block
        MethodObject->>BlockObject: execute(receiver, args)
    else Attribute or Setter
        alt Attribute exists
            Instance-->>Sender: return attribute
        else Setter (ends with ":")
            Instance->>Instance: set attribute
        end
    end
```
##### Block Execution
 - Parameters:
   - Sorted by order attribute during XML parsing.
   - Mapped to arguments during block execution.
 - Scope:
   - Each block has a Scope object for local variables.
   - Inherits from parent scope (closure).

```mermaid
flowchart TB
    BlockStart([Block starts])
    --> AssignParams[Assign parameters by order]
    --> ExecuteAssignments[Execute assignments in order]
    --> ReturnResult[Return 'result' variable]

    style BlockStart fill:#4CAF50
    style ReturnResult fill:#FF5722
```

K objektovému návrhu
a implementaci je samozřejmě povinné sepsat také stručnou a terminologicky správnou dokumentaci
uvádějící, jak jste rámec ipp-core pro splnění zadání použili/rozšířili.

