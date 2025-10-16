<?php

namespace IPP\Student\Runtime\BuildIns;

use IPP\Student\Exceptions\RuntimeException;
use IPP\Student\Exceptions\SemanticException;
use IPP\Student\Objects\{BlockObject, ClassObject, BuiltInFactory};
use IPP\Student\Runtime\{Scope, Instance};

class BlockInstance extends Instance
{
    public function __construct(ClassObject $class)
    {
        if ($class == null) {
            throw new RuntimeException("BLOCK INSTANCE: Class object cannot be null", 52);
        }
        parent::__construct($class);
    }

    /**
     * @param string $selector
     * @param Instance[]  $args
     * @return mixed
     */
    public function send(string $selector, array $args): mixed
    {
        // value handling --> executing block variable
        if (str_starts_with($selector, 'value')) {
            // Count colons to determine expected arity
            $arity = substr_count($selector, ':');
            if ($arity !== count($args)) {
                throw new SemanticException("Arity mismatch in block call.", 33);
            }

            $block = $this->attributes['block'] ?? null;
            $closureScope = $this->attributes['scope'] ?? null;

            if ($block instanceof BlockObject) {
                if ($closureScope instanceof Scope) {
                    return $block->execute(null, $args, $closureScope);
                } else {
                    throw new RuntimeException("Expected 'scope' attribute to be a Scope object or null.", 53);
                }
            } else {
                throw new RuntimeException("Expected 'block' attribute to be a BlockObject.", 53);
            }
        }
        if ($selector == 'whileTrue:') {
            return $this->executeWhileTrue($args[0]);
        } else {
            return parent::send($selector, $args);
        }
    }
    private function executeWhileTrue(mixed $bodyBlock): mixed
    {
        if (!$bodyBlock instanceof Instance) {
            throw new RuntimeException("Argument to whileTrue: must be a block instance.", 53);
        }

        while (true) {
            $conditionResult = $this->send('value', []);
            if (!$conditionResult instanceof Instance) {
                throw new RuntimeException("Condition block did not return an Instance.", 52);
            }
            if ($conditionResult->class->name !== 'True') {
                break;
            }
            $bodyBlock->send('value', []);
        }

        return BuiltInFactory::create('Nil', 'nil');
    }
}
