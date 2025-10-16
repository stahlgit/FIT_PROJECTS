<?php

namespace IPP\Student\Objects;

use IPP\Student\Exceptions\{SemanticException, RuntimeException};
use IPP\Student\Runtime\BuildIns\{FalseInstance, IntegerInstance, StringInstance, TrueInstance, NilInstance};
use IPP\Student\Runtime\Instance;

class BuiltInFactory
{
    public static ClassObject $trueClass;
    public static ClassObject $falseClass;
    public static ClassObject $stringClass;
    public static ClassObject $integerClass;
    public static ClassObject $nilClass;
    public static ClassObject $blockClass;

    /**
     * @param string $type
     * @param string $value
     * @return Instance
     * @throws RuntimeException
     * @throws SemanticException
     */
    public static function create(string $type, string $value): Instance
    {
        switch ($type) {
            case 'Boolean':
                if ($value == 'true') {
                    return TrueInstance::getInstance(self::$trueClass);
                } else {
                    return FalseInstance::getInstance(self::$falseClass);
                }
            case "True":
                if (self::$trueClass == null) {
                    throw new RunTimeException("True Class object cannot be null", 52);
                }
                return TrueInstance::getInstance(self::$trueClass);
            case "False":
                if (self::$falseClass == null) {
                    throw new RunTimeException("False Class object cannot be null", 52);
                }
                return FalseInstance::getInstance(self::$falseClass);
            case 'String':
                if (self::$stringClass == null) {
                    throw new RunTimeException("String Class object cannot be null", 52);
                }
                return new StringInstance(self::$stringClass, $value);
            case 'Integer':
                if (self::$integerClass == null) {
                    throw new RunTimeException("Integer Class object cannot be null", 52);
                }
                return new IntegerInstance(self::$integerClass, (int)$value);
            case 'Nil':
                if (self::$nilClass == null) {
                    throw new RunTimeException("Nil Class object cannot be null", 52);
                }
                return NilInstance::getInstance(self::$nilClass);
            default:
                echo "Unknown literal type: $type\n";
                throw new RunTimeException("Unknown literal type: $type", 51);
        }
    }
}
