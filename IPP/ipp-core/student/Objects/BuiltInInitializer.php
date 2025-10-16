<?php

namespace IPP\Student\Objects;

use IPP\Student\Objects\BuiltIns\{BlockClass, FalseClass, IntegerClass, NilClass, ObjectClass, StringClass, TrueClass};

class BuiltInInitializer
{
    /** @return array<string, ClassObject> */
    public static function initialize(): array
    {
        $objectClass = new ObjectClass('Object');
        $trueClass = new TrueClass('True', $objectClass);
        $falseClass = new FalseClass('False', $objectClass);
        $nilClass = new NilClass('Nil', $objectClass);
        $integerClass = new IntegerClass('Integer', $objectClass);
        $stringClass = new StringClass('String', $objectClass);
        $blockClass = new BlockClass('Block', $objectClass);

        BuiltInFactory::$trueClass = TrueClass::registerMethods($trueClass);
        BuiltInFactory::$falseClass = FalseClass::registerMethods($falseClass);
        BuiltInFactory::$nilClass = NilClass::registerMethods($nilClass);
        BuiltInFactory::$integerClass = IntegerClass::registerMethods($integerClass);
        BuiltInFactory::$stringClass = StringClass::registerMethods($stringClass);
        BuiltInFactory::$blockClass = BlockClass::registerMethods($blockClass);

        $objectClass = ObjectClass::registerMethods($objectClass);
        BuiltInFactory::create('Nil', 'nil');

        return [
            $objectClass->name => $objectClass,
            $trueClass->name => BuiltInFactory::$trueClass,
            $falseClass->name => BuiltInFactory::$falseClass,
            $nilClass->name => BuiltInFactory::$nilClass,
            $integerClass->name => BuiltInFactory::$integerClass,
            $stringClass->name => BuiltInFactory::$stringClass,
            $blockClass->name => BuiltInFactory::$blockClass,
        ];
    }
}
