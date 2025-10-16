<?php

namespace IPP\Student\Traverse;

use DOMElement;
use IPP\Student\Objects\Expressions\VariableExpression;

class VariableTraverse
{
    public function traverse(DOMElement $node): VariableExpression
    {
        return new VariableExpression($node->getAttribute('name'));
    }
}
