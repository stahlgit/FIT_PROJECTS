<?php

namespace IPP\Student\Objects\Expressions;

use IPP\Student\Runtime\Scope;

class VariableExpression implements ExpressionInterface
{
    public string $name;

    public function __construct(string $name)
    {
        $this->name = $name;
    }

    public function evaluate(Scope $scope): mixed
    {
        return $scope->getVariable($this->name);
    }
}
