<?php

namespace IPP\Student\Runtime\BuildIns;

use IPP\Student\Exceptions\RuntimeException;
use IPP\Student\Objects\BuiltInFactory;
use IPP\Student\Objects\ClassObject;
use IPP\Student\Runtime\Instance;

class FalseInstance extends Instance
{
    private static ?self $instance = null;

    private function __construct(ClassObject $class)
    {
        if ($class == null) {
            throw new RuntimeException("FALSE INSTANCE: Class object cannot be null", 52);
        }
        parent::__construct($class);
    }

    /**
     * @param string $selector
     * @param Instance[] $args
     * @return mixed
     * @throws RuntimeException
     */
    public function send(string $selector, array $args): mixed
    {
        switch ($selector) {
            case 'not':
                return BuiltInFactory::create('Boolean', 'true');
            case 'and:':
                return $this; // false and anything = false
            case 'or:':
                return $args[0] ?? BuiltInFactory::create('Boolean', 'false');
            case 'ifTrue:ifFalse:':
                if (!isset($args[1])) {
                    throw new RuntimeException("ifTrue:ifFalse: expects at least 2 arguments, first missing.", 51);
                }
                return $args[1]->send('value', []); //false path
            default:
                return parent::send($selector, $args);
        }
    }

    public static function getInstance(ClassObject $falseClass): self
    {
        return self::$instance ??= new self($falseClass);
    }
    public static function resetInstance(): void
    {
        self::$instance = null;
    }
}
