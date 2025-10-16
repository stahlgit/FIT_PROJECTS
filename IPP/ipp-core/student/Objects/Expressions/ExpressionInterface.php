<?php

namespace IPP\Student\Objects\Expressions;

use IPP\Student\Runtime\Scope;

interface ExpressionInterface
{
    // Contract for all expression objects
    public function evaluate(Scope $scope): mixed;
}
