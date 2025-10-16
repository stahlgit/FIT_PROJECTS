<?php

namespace IPP\Student\Objects;

use IPP\Student\Exceptions\SemanticException;
use IPP\Student\Objects\Expressions\{ExpressionInterface, VariableExpression};
use IPP\Student\Runtime\Scope;

class AssignmentObject
{
    // represents a variable assignment
    public VariableExpression $variable;
    public ExpressionInterface $expression;

    public function __construct(VariableExpression $variable, ExpressionInterface $expression)
    {
        $this->variable = $variable;
        $this->expression = $expression;
    }

    public function assign(Scope $scope): void
    {
        $value = $this->expression->evaluate($scope);
        try {
            $scope->setVariable($this->variable->name, $value);
        } catch (SemanticException $e) {
            exit($e->getCode());
        }
    }
}
