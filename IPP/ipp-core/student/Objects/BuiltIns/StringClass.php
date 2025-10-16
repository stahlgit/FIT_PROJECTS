<?php

namespace IPP\Student\Objects\BuiltIns;

use IPP\Student\Objects\{AssignmentObject, BlockObject, ClassObject, MethodObject};
use IPP\Student\Objects\Expressions\{LiteralExpression, SendExpression, VariableExpression};

class StringClass extends ClassObject
{
    public string $value = '';

    public function __construct(string $name, ?ClassObject $parent = null)
    {
        parent::__construct($name, $parent);
    }

    public static function registerMethods(ClassObject $stringClass): ClassObject
    {
        /** equalTo:  **/
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

        /** isString  **/
        $isStringBlock = new BlockObject(0);
        $isStringBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new LiteralExpression('Boolean', 'true')
            )
        );

        /** asString  **/
        $asStringBlock = new BlockObject(0);
        $asStringBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new SendExpression(
                    'asString',
                    new VariableExpression('self')
                )
            )
        );

        /** asInteger */
        $asIntegerBlock = new BlockObject(0);
        $asIntegerBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new VariableExpression('self')
            )
        );
        /** read */
        $readBlock = new BlockObject(0);
        $readBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new SendExpression(
                    'read',
                    new VariableExpression('self')
                )
            )
        );
        /** print */
        $printBlock = new BlockObject(0);
        $printBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new SendExpression(
                    'print',
                    new VariableExpression('self')
                )
            )
        );


        /** concatenateWith:  */
        $concantenateWithBlock = new BlockObject(1);
        $concantenateWithBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new SendExpression(
                    'concatenateWith:',
                    new VariableExpression('self'),
                    [ new VariableExpression('arg0')]
                )
            )
        );

        /**startsWith:endsBefore:*/
        $startWithendsBeforeBlock = new BlockObject(2);
        $startWithendsBeforeBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new SendExpression(
                    'startsWith:endsBefore:',
                    new VariableExpression('self'),
                    [ new VariableExpression('arg0'), new VariableExpression('arg1')]
                )
            )
        );

        /** Add to class**/
        $stringClass->addMethod(new MethodObject('equalTo:', $equalToBlock));
        $stringClass->addMethod(new MethodObject('isString', $isStringBlock));
        $stringClass->addMethod(new MethodObject('asString', $asStringBlock));
        $stringClass->addMethod(new MethodObject('asInteger', $asIntegerBlock));
        $stringClass->addMethod(new MethodObject('read', $readBlock));
        $stringClass->addMethod(new MethodObject('print', $printBlock));
        $stringClass->addMethod(new MethodObject('concatenateWith:', $concantenateWithBlock));
        $stringClass->addMethod(new MethodObject('startsWith:endsBefore:', $startWithendsBeforeBlock));

        return $stringClass;
    }
}
