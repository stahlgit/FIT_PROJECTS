<?php

namespace IPP\Student\Runtime\BuildIns;

use IPP\Student\Exceptions\RuntimeException;
use IPP\Student\Exceptions\SemanticException;
use IPP\Student\GetInput;
use IPP\Student\GetOutput;
use IPP\Student\Objects\BuiltInFactory;
use IPP\Student\Objects\ClassObject;
use IPP\Student\Runtime\Instance;

class StringInstance extends Instance
{
    public string $value;
    public function __construct(ClassObject $class, string $value = '')
    {
        if ($class == null) {
            throw new SemanticException("STRING INSTANCE: Class object cannot be null", 52);
        }
        parent::__construct($class);
        $this->value = $value;
    }

    /**
     * @param string $selector
     * @param StringInstance[] $args
     * @return mixed
     * @throws SemanticException
     */
    protected function handleBuiltIn(string $selector, array $args): mixed
    {
        switch ($selector) {
            case 'equalTo:':
                if (!isset($args[0])) {
                    throw new SemanticException("stringEqual: expects StringInstance as argument", 51);
                }
                return ($this->value === $args[0]->value)
                    ? BuiltInFactory::create('Boolean', 'true')
                    : BuiltInFactory::create('Boolean', 'false');
            case 'asString':
            case 'new': // since this is created in advance
                return $this;
            case 'asInteger':
                if (is_numeric($this->value) && ctype_digit(ltrim($this->value, '-'))) {
                    return new IntegerInstance(BuiltInFactory::$integerClass, intval($this->value));
                } else {
                    return BuiltInFactory::create('Nil', 'nil');
                }
            case 'read':
                $inputRead = GetInput::getStdIn();
                if ($inputRead === null) {
                    $inputFromFgets = fgets(STDIN);
                    $input = ($inputFromFgets !== false) ? trim($inputFromFgets) : '';
                } else {
                    $input = $inputRead->readString();
                }
                if ($input === null) {
                    $input = '';
                }
                return new self($this->class, $input);
            case 'print':
                $output = GetOutput::getStdout();
                if ($output === null) {
                    // if input reader is not set use echo
                    echo $this->value;
                } else {
                    $output->writeString($this->value);
                }
                return $this;
            case 'concatenateWith:':
                if (!isset($args[0])) {
                    return BuiltInFactory::create('Nil', 'nil');
                }
                return new self($this->class, $this->value . $args[0]->value);
            case 'startsWithendsBefore:':
                return $this->startsWithEndsBefore($args);
            default:
                return parent::send($selector, $args);
        }
    }

    /**
     * @param string $selector
     * @param StringInstance[] $args
     * @return mixed
     * @throws SemanticException
     */
    public function send(string $selector, array $args): mixed
    {
        if ($this->class->name == "String") {
            $method = null;
        } else {
            // check for method in current scope (also if some built in was overwritten)
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
     * @param Instance[] $args
     * @return mixed
     */
    private function startsWithEndsBefore(array $args): mixed
    {
        if (count($args) !== 2 || !($args[0] instanceof IntegerInstance) || !($args[1] instanceof IntegerInstance)) {
            return BuiltInFactory::create('Nil', 'nil');
        }
        $startIndex = $args[0]->value;
        $endIndex = $args[1]->value;
        if ($startIndex < 1 || $endIndex < 1) {
            return BuiltInFactory::create('Nil', 'nil');
        }

        if ($endIndex <= $startIndex) {
            return new self($this->class, '');
        }

        $substring = substr($this->value, $startIndex - 1, $endIndex - $startIndex);
        return new self($this->class, $substring);
    }
}
