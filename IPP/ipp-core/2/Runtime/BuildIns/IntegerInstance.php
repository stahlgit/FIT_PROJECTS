<?php

namespace IPP\Student\Runtime\BuildIns;

use Exception;
use IPP\Student\Exceptions\RuntimeException;
use IPP\Student\Exceptions\SemanticException;
use IPP\Student\Objects\BuiltInFactory;
use IPP\Student\Objects\ClassObject;
use IPP\Student\Runtime\Instance;

class IntegerInstance extends Instance
{
    public int $value;

    public function __construct(ClassObject $class, int $value = 0)
    {
        parent::__construct($class);
        $this->value = $value;
    }

    /**
     * @param Instance[] $args
     * @param string $message
     * @return void
     * @throws Exception
     */
    private function checkInstance(array $args, string $message): void
    {
        if (!isset($args[0]) || !$args[0] instanceof IntegerInstance) {
            echo "$message expects IntegerInstance as argument\n";
            throw new RuntimeException($message . " expects IntegerInstance as argument\n", 51);
        }
    }
    /**
     * @param Instance[] $args
     * @param string $message
     * @return void
     * @throws Exception
     */
    private function checkBlock(array $args, string $message): void
    {
        if (!isset($args[0]) || !$args[0] instanceof BlockInstance) {
            echo "$message expects IntegerInstance as argument\n";
            throw new RuntimeException($message . " expects BlockInstance as argument\n", 51);
        }
    }

    /**
     * @param string $selector
     * @param IntegerInstance[] $args
     * @return mixed
     * @throws RuntimeException
     */
    protected function handleBuiltIn(string $selector, array $args): mixed
    {
        switch ($selector) {
            case 'equalTo:':
                $this->checkInstance($args, 'equalTo:');
                return ($this->value === $args[0]->value)
                    ? BuiltInFactory::create('Boolean', 'true')
                    : BuiltInFactory::create('Boolean', 'false');
            case 'greaterThan:':
                $this->checkInstance($args, 'greaterThan:');
                return ($this->value > $args[0]->value)
                    ? BuiltInFactory::create('Boolean', 'true')
                    : BuiltInFactory::create('Boolean', 'false');
            case 'plus:':
                $this->checkInstance($args, 'plus:');
                return new self($this->class, $this->value + $args[0]->value);
            case 'minus:':
                $this->checkInstance($args, 'numericMinus');
                return new self($this->class, $this->value - $args[0]->value);
            case 'multiplyBy:':
                $this->checkInstance($args, 'multiplyBy:');
                return new self($this->class, $this->value * $args[0]->value);
            case 'divBy:':
                $this->checkInstance($args, 'divBy:');
                if ($args[0]->value === 0) {
                    throw new RuntimeException("Division by zero", 53);
                }
                return new self($this->class, intdiv($this->value, $args[0]->value));
            case 'toString':
                return BuiltInFactory::create('String', (string)$this->value);
            case 'asInteger':
                return $this;
            case 'timesRepeat:':
                $this->checkBlock($args, 'timesRepeat:');
                return $this->executeTimesRepeat($args[0]);
            case 'value':
                // wrap the raw integer back into an IntegerInstance
                return new self($this->class, $this->value);
            case 'value:':
                // set the integer (if you have mutable ints; otherwise throw)
                $this->value = (int)$args[0]->value;
                return $args[0]; // <-- not `$this`, return the new value (as per Smalltalk)
            default:
                return parent::send($selector, $args);
        }
    }

    /**
     * @param string $selector
     * @param IntegerInstance[] $args
     * @return mixed
     * @throws SemanticException
     */
    public function send(string $selector, array $args): mixed
    {
        if ($this->class->name == "Integer") {
            $method = null;
        } else {
            $method = $this->class->findCurrentMethod($selector);
        }

        if ($method !== null) {
            if (count($args) !== $method->block->arity) {
                throw new SemanticException("Arity mismatch for '$selector'", 33);
            }
            return $method->block->execute($this, $args);
        }
        return $this->handleBuiltin($selector, $args);
    }

    /**
     * Implements timesRepeat: block execution.
     * @param mixed $blockInstance
     * @return mixed
     */
    private function executeTimesRepeat(mixed $blockInstance): mixed
    {
        if (!($blockInstance instanceof Instance)) {
            throw new RuntimeException("Argument for timesRepeat: must be an Instance (Block expected).", 53);
        }
        if ($this->value <= 0) {
            return BuiltInFactory::create('Nil', 'nil');
        }
        for ($i = 1; $i <= $this->value; $i++) {
            // Pass iteration index as IntegerInstance
            $iterationArg = new self($this->class, $i);
            $blockInstance->send('value:', [$iterationArg]);
        }
        return BuiltInFactory::create('Nil', 'nil');
    }
}
