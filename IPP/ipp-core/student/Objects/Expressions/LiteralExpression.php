<?php

namespace IPP\Student\Objects\Expressions;

use IPP\Student\Exceptions\SemanticException;
use IPP\Student\Objects\BuiltInFactory;
use IPP\Student\Objects\ClassObject;
use IPP\Student\Runtime\{Instance, RuntimeRegistry, Scope};

class LiteralExpression implements ExpressionInterface
{
    public string $type;
    public string $value;

    public function __construct(string $type, string $value)
    {
        $this->type = $type;
        $this->value = $value;
    }

    public function evaluate(Scope $scope): Instance|ClassObject
    {
        if ($this->type === 'class') {
            $classObject = RuntimeRegistry::findClass($this->value);
            if ($classObject === null) {
                throw new SemanticException("Class '{$this->value}' not found.", 32);
            }

            if ($this->value === 'String') {
                // premature creation for read function
                return BuiltInFactory::create($this->value, "");
            }
            return $classObject;
        }
        return BuiltInFactory::create($this->type, $this->value);
    }
}
