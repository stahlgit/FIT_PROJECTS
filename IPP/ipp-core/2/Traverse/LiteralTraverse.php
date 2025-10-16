<?php

namespace IPP\Student\Traverse;

use DOMElement;
use IPP\Student\Exceptions\InterpretException;
use IPP\Student\Objects\Expressions\LiteralExpression;

class LiteralTraverse
{
    public function traverse(DOMElement $node): LiteralExpression
    {
        $class = $node->getAttribute('class');
        $value = $node->getAttribute('value');
        if (empty($class)) {
            throw new InterpretException("Literal node missing 'class' attribute", 42);
        }
        if ($class === 'String') {
            $value = stripcslashes($value);
        }

        if ($value == null) {
            throw new InterpretException("Literal node missing 'value' attribute", 42);
        }

        return new LiteralExpression($class, $value);
    }
}
