<?php

namespace IPP\Student\Objects;

use IPP\Student\Exceptions\RuntimeException;
use IPP\Student\Runtime\BuildIns\{BlockInstance, FalseInstance, IntegerInstance, NilInstance, StringInstance,
    TrueInstance};
use IPP\Student\Runtime\Instance;

class ClassObject
{
    public string $name;
    public ?ClassObject $parent;
    /** @var array <string, MethodObject> */
    public array $methods = [];

    public function __construct(string $name, ?ClassObject $parent = null)
    {
        $this->name = $name;
        $this->parent = $parent;
    }

    public function addMethod(MethodObject $method): void
    {
        $this->methods[$method->selector] = $method;
    }

    public function findMethod(string $selector): ?MethodObject
    {
        if (isset($this->methods[$selector])) {
            return $this->methods[$selector];
        }
        // if the method is not found in the current class, check the parent class
        return $this->parent?->findMethod($selector);
    }

    public function findCurrentMethod(string $selector): ?MethodObject
    {
        if (isset($this->methods[$selector])) {
            return $this->methods[$selector];
        }
        return null;
    }

    /**
     * @param string $selector
     * @param array<Instance> $args
     * @return ?Instance
     * @throws RuntimeException
     */
    public function send(string $selector, array $args): ?Instance
    {
        if ($selector == 'from:') {
            if (!isset($args[0])) {
                throw new RuntimeException("Class object cannot be null", 53);
            }
            if ($args[0] instanceof StringInstance || $args[0] instanceof IntegerInstance) {
                return $this->createInstance($args[0]->value);
            }
        }
        if ($selector == 'new') {
            return $this->createInstance();
        }
        $method = $this->findMethod($selector);
        if (!$method) {
            throw new RuntimeException("Method '$selector' not found on class '{$this->name}'", 53);
        }
        return null;
    }

    public function createInstance(int|string|null $initialValue = null): Instance
    {
        // Climb up the parent chain to find the built-in root type
        $current = $this;
        while ($current !== null) {
            switch ($current->name) {
                case 'Block':
                    return new BlockInstance($this);
                case 'Integer':
                    return new IntegerInstance($this, (int)$initialValue);
                case 'String':
                    echo $initialValue;
                    return new StringInstance($this, (string)$initialValue);
                case 'True':
                    return TrueInstance::getInstance($this);
                case 'False':
                    return FalseInstance::getInstance($this);
                case 'Nil':
                    return NilInstance::getInstance();
                case 'Object':
                    return new Instance($this);
            }
            $current = $current->parent;
        }
        // Default fallback if no special base class was found
        return new Instance($this);
    }
}
