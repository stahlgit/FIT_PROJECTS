<?php

namespace IPP\Student\Objects\BuiltIns;

use IPP\Student\Objects\{AssignmentObject, BlockObject, ClassObject, MethodObject};
use IPP\Student\Objects\Expressions\{LiteralExpression, VariableExpression};

class BlockClass extends ClassObject
{
    public function __construct(string $name, ?ClassObject $parent = null)
    {
        parent::__construct($name, $parent);
    }

    public static function registerMethods(ClassObject $blockClass): ClassObject
    {
        /** isBlock  **/
        $isBlockBlock = new BlockObject(0);
        $isBlockBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new LiteralExpression('Boolean', 'true')
            )
        );

        /** whileTrue: */
        $whileTrueBlock = new BlockObject(1);
        $whileTrueBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new VariableExpression('self')
            )
        );

        $blockClass->addMethod(new MethodObject('isBlock', $isBlockBlock));
        $blockClass->addMethod(new MethodObject('whileTrue:', $whileTrueBlock));
        return $blockClass;
    }
}
