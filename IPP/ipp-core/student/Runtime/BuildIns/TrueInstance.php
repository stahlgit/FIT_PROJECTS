<?php

namespace IPP\Student\Runtime\BuildIns;

use IPP\Student\Exceptions\RuntimeException;
use IPP\Student\Objects\BuiltInFactory;
use IPP\Student\Objects\ClassObject;
use IPP\Student\Runtime\Instance;

class TrueInstance extends Instance
{
    private static ?self $instance = null;

    private function __construct(ClassObject $trueClass)
    {
        if ($trueClass == null) {
            throw new RuntimeException("NIL INSTANCE: Class object cannot be null", 52);
        }
        parent::__construct($trueClass);
    }
    /**
     * @param string $selector
     * @param Instance[] $args
     * @return mixed
     * @throws \Exception
     */
    public function send(string $selector, array $args): mixed
    {
        switch ($selector) {
            case 'not':
                return BuiltInFactory::create('Boolean', 'false'); // create ? not get ?
            case 'and:':
                return $args[0] ?? BuiltInFactory::create('Boolean', 'false');  // default fallback safety
            case 'or:':
                return $this;  // true or anything = true
            case 'ifTrue:ifFalse:':
                if (!isset($args[0])) {
                    throw new RuntimeException("ifTrue:ifFalse: expects at least 2 arguments, first missing.", 51);
                }
                return $args[0]->send('value', []);
            default:
                return parent::send($selector, $args);// fallback to class-defined method
        }
    }

    public static function getInstance(ClassObject $trueClass): self
    {
        return self::$instance ??= new self($trueClass);
    }
    public static function resetInstance(): void
    {
        self::$instance = null;
    }
}
