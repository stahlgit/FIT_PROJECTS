<?php

namespace IPP\Student\Objects\BuiltIns;

use IPP\Student\Objects\{AssignmentObject, ClassObject, MethodObject, BlockObject};
use IPP\Student\Objects\Expressions\{SendExpression, VariableExpression, LiteralExpression};

/**
 * Base class for all objects in the language
 */
class ObjectClass extends ClassObject
{
    public function __construct(string $name, ?ClassObject $parent = null)
    {
        parent::__construct($name, $parent);
    }

    public static function registerMethods(ClassObject $objectClass): ClassObject
    {
        /** identicalTo:  **/
        $identicalToBlock = new BlockObject(1);
        $identicalToBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new SendExpression(
                    'identicalTo:',
                    new VariableExpression('self'),
                    [ new VariableExpression('arg0') ]
                )
            )
        );

        /** equalTo  **/
        $equalToBlock = new BlockObject(1);
        $equalToBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new SendExpression(
                    'identicalTo:',
                    new VariableExpression('self'),
                    [ new VariableExpression('arg0') ]
                )
            )
        );

        /** asString  **/
        $asStringBlock = new BlockObject(0);
        $asStringBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new LiteralExpression('String', '')
            )
        );

        /** isNumber  **/
        $isNumberBlock = new BlockObject(0);
        $isNumberBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new LiteralExpression('Boolean', 'false')
            )
        );

        /** isString  **/
        $isStringBlock = new BlockObject(0);
        $isStringBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new LiteralExpression('Boolean', 'false')
            )
        );

        /** isBlock  **/
        $isBlockBlock = new BlockObject(0);
        $isBlockBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new LiteralExpression('Boolean', 'false')
            )
        );

        /** isNil  **/
        $isNilBlock = new BlockObject(0);
        $isNilBlock->addAssignment(
            new AssignmentObject(
                new VariableExpression('result'),
                new LiteralExpression('Boolean', 'false')
            )
        );

        /** Add to class**/
        $objectClass->addMethod(new MethodObject('identicalTo', $identicalToBlock));
        $objectClass->addMethod(new MethodObject('equalTo', $equalToBlock));
        $objectClass->addMethod(new MethodObject('asString', $asStringBlock));
        $objectClass->addMethod(new MethodObject('isNumber', $isNumberBlock));
        $objectClass->addMethod(new MethodObject('isString', $isStringBlock));
        $objectClass->addMethod(new MethodObject('isBlock', $isBlockBlock));
        $objectClass->addMethod(new MethodObject('isNil', $isNilBlock));
        return $objectClass;
    }
}
