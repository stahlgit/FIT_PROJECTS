<?php

namespace IPP\Student\Objects\BuiltIns;

use IPP\Student\Objects\{AssignmentObject, BlockObject, ClassObject, MethodObject};
use IPP\Student\Objects\Expressions\{LiteralExpression, SendExpression, VariableExpression};

class IntegerClass extends ClassObject
{
    public int $value = 0;

    public function __construct(string $name, ?ClassObject $parent = null)
    {
        parent::__construct($name, $parent);
    }

    public static function registerMethods(ClassObject $integerClass): ClassObject
    {
        /** identicalTo  **/
        /** equalTo  **/
        $equalToBlock = new BlockObject(1);
        $equalToBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new SendExpression(
                    'equalTo:',
                    new VariableExpression('self'),
                    [new VariableExpression('arg0')]
                )
            )
        );

        /** asString  **/
        $asStringBlock = new BlockObject(0);
        $asStringBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new SendExpression(
                    'toString',
                    new VariableExpression('self')
                )
            )
        );

        /** isNumber  **/
        $isNumberBlock = new BlockObject(0);
        $isNumberBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new LiteralExpression('Boolean', 'true')
            )
        );

        /** greaterThan: **/
        $greaterThanBlock = new BlockObject(1);
        $greaterThanBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new SendExpression(
                    'greaterThan:',
                    new VariableExpression('self'),
                    [new VariableExpression('arg0')]
                )
            )
        );

        /** plus:  **/
        $plusBlock = new BlockObject(1);
        $plusBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new SendExpression(
                    'plus:',
                    new VariableExpression('self'),
                    [new VariableExpression('arg0')]
                )
            )
        );

        /** minus:  **/
        $minusBlock = new BlockObject(1);
        $minusBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new SendExpression(
                    'minus:',
                    new VariableExpression('self'),
                    [new VariableExpression('arg0')]
                )
            )
        );

        /** multiplyBy:  **/
        $multiplyByBlock = new BlockObject(1);
        $multiplyByBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new SendExpression(
                    'multiplyBy:',
                    new VariableExpression('self'),
                    [new VariableExpression('arg0')]
                )
            )
        );

        /** divBy:  **/
        $divByBlock = new BlockObject(1);
        $divByBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new SendExpression(
                    'divBy:',
                    new VariableExpression('self'),
                    [new VariableExpression('arg0')]
                )
            )
        );

        /** asInteger  **/
        $asIntegerBlock = new BlockObject(0);
        $asIntegerBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new VariableExpression('self')
            )
        );

        /** timesRepeat  **/
        $timesRepeatBlock = new BlockObject(1);
        $timesRepeatBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new SendExpression(
                    'timesRepeat:',
                    new VariableExpression('self'),
                    [ new VariableExpression('arg0') ]
                )
            )
        );

        $integerClass->addMethod(new MethodObject('equalTo:', $equalToBlock));
        $integerClass->addMethod(new  MethodObject('asString', $asStringBlock));
        $integerClass->addMethod(new MethodObject('isNumber', $isNumberBlock));
        $integerClass->addMethod(new MethodObject('greaterThan:', $greaterThanBlock));
        $integerClass->addMethod(new MethodObject('plus:', $plusBlock));
        $integerClass->addMethod(new MethodObject('minus:', $minusBlock));
        $integerClass->addMethod(new MethodObject('multiplyBy:', $multiplyByBlock));
        $integerClass->addMethod(new MethodObject('divBy:', $divByBlock));
        $integerClass->addMethod(new MethodObject('asInteger', $asIntegerBlock));
        $integerClass->addMethod(new MethodObject('timesRepeat:', $timesRepeatBlock));

        return $integerClass;
    }
}
