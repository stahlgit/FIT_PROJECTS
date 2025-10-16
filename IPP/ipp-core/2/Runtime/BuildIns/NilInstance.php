<?php

namespace IPP\Student\Runtime\BuildIns;

use IPP\Student\Exceptions\RuntimeException;
use IPP\Student\Exceptions\SemanticException;
use IPP\Student\Objects\BuiltInFactory;
use IPP\Student\Objects\ClassObject;
use IPP\Student\Runtime\Instance;

class NilInstance extends Instance
{
    public static string $value = 'nil';
    private static ?self $instance = null;

    private function __construct(ClassObject $class)
    {
        if ($class == null) {
            throw new SemanticException("NIL INSTANCE: Class object cannot be null", 52);
        }
        parent::__construct($class);
    }

    public static function getInstance(?ClassObject $nilClass = null): self
    {
        if (self::$instance !== null) {
            return self::$instance;
        }

        if ($nilClass === null) {
            throw new RuntimeException("NilInstance has not been initialized yet", 52);
        }

        return self::$instance = new self($nilClass);
    }


    public function send(string $selector, array $args): mixed
    {
        // from NilInstance to StringInstance
        if ($selector === 'asString') {
            return new StringInstance(BuiltInFactory::$stringClass, 'nil');
        }

        return parent::send($selector, $args);
    }
}
