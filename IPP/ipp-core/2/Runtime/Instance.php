<?php

namespace IPP\Student\Runtime;

use IPP\Student\Exceptions\RuntimeException;
use IPP\Student\Exceptions\SemanticException;
use IPP\Student\Objects\BlockObject;
use IPP\Student\Objects\BuiltInFactory;
use IPP\Student\Objects\ClassObject;

class Instance
{
    public ClassObject $class;
    /** @var array<string, Instance|BlockObject|Scope> */
    public array $attributes = [];

    public function __construct(ClassObject $class)
    {
        if ($class == null) {
            throw new SemanticException("Class object cannot be null", 52);
        }
        $this->class = $class;
    }

    /**
     * @param string $selector
     * @param Instance[] $args
     * @return mixed
     * @throws SemanticException
     */
    public function send(string $selector, array $args): mixed
    {
        if ($this->class == null) {
            throw new SemanticException("Class object cannot be null", 52);
        }
        if ($selector === 'identicalTo:') {
            return ($this === $args[0])
                ? BuiltInFactory::create('Boolean', 'true')
                : BuiltInFactory::create('Boolean', 'false');
        }

        $method = $this->class->findMethod($selector);
        if ($method === null) {
            if (isset($this->attributes[$selector])) {
                return $this->attributes[$selector];
            }
            // Setter fallback: ending with ':' and single argument
            if (str_ends_with($selector, ':') && count($args) === 1) {
                // strip ':' from selector to get variable name
                $attrName = substr($selector, 0, -1);
                $this->attributes[$attrName] = $args[0];

                return $args[0];
            }
            echo "Method $selector not found in class {$this->class->name}\n";
            throw new RuntimeException("Unknown selector: $selector", 51);
        }

        $block = $method->block;
        if (count($args) !== $block->arity) {
            throw new SemanticException("Arity mismatch ", 33);
        }
        return $block->execute($this, $args);
    }
}
