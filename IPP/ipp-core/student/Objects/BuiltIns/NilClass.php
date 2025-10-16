<?php

namespace IPP\Student\Objects\BuiltIns;

use IPP\Student\Objects\{ClassObject, BlockObject, AssignmentObject, MethodObject};
use IPP\Student\Objects\Expressions\{LiteralExpression, VariableExpression};

class NilClass extends ClassObject
{
    public function __construct(string $name, ?ClassObject $parent = null)
    {
        parent::__construct($name, $parent);
    }

    public static function registerMethods(ClassObject $nilClass): ClassObject
    {
        /** asString  **/
        $asStringBlock = new BlockObject(0);
        $asStringBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new LiteralExpression('String', 'nil')
            )
        );

        /** isNil  **/
        $isNilBlock = new BlockObject(0);
        $isNilBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new LiteralExpression('Boolean', 'true')
            )
        );

        /** Add to class**/
        $nilClass->addMethod(new MethodObject('isNil', $isNilBlock));
        $nilClass->addMethod(new MethodObject('isString', $asStringBlock));
        return $nilClass;
    }
}
