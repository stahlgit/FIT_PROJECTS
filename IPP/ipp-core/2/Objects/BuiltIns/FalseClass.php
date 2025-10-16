<?php

namespace IPP\Student\Objects\BuiltIns;

use IPP\Student\Objects\{AssignmentObject, BlockObject, ClassObject, MethodObject};
use IPP\Student\Objects\Expressions\{LiteralExpression, SendExpression, VariableExpression};

class FalseClass extends ClassObject
{
    public function __construct(string $name, ?ClassObject $parent = null)
    {
        parent::__construct($name, $parent);
    }

    public static function registerMethods(ClassObject $falseClass): ClassObject
    {
        /** not  **/
        $notBlock = new BlockObject(0);
        $notBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new LiteralExpression('Boolean', 'true')
            )
        );

        /** and **/
        $andBlock = new BlockObject(1);
        $andBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new VariableExpression('arg0')
            )
        );

        /** or **/
        $orBlock = new BlockObject(1);
        $orBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new VariableExpression('self')
            )
        );

        /** ifTrue:ifFalse:  **/
        $ifTrueFalseBlock = new BlockObject(2);
        $ifTrueFalseBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new SendExpression(
                    'value',
                    new VariableExpression('arg1'),
                    []
                )
            )
        );

        /** Add to class**/
        $falseClass->addMethod(new MethodObject('not', $notBlock));
        $falseClass->addMethod(new MethodObject('and', $andBlock));
        $falseClass->addMethod(new MethodObject('or', $orBlock));
        $falseClass->addMethod(new MethodObject('ifTrue:ifFalse:', $ifTrueFalseBlock));

        return $falseClass;
    }
}
