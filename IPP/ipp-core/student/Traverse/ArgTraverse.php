<?php

namespace IPP\Student\Traverse;

use DOMElement;
use IPP\Student\Exceptions\InterpretException;
use IPP\Student\Objects\Expressions\ExpressionInterface;

class ArgTraverse
{
    public function traverse(DOMElement $node): ExpressionInterface
    {
        foreach ($node->childNodes as $child) {
            if ($child->nodeType !== XML_ELEMENT_NODE) {
                continue;
            }
            if ($child->nodeName === 'expr') {
                /** @var DOMElement $child */
                return ((new ExpressionTraverse())->traverse($child));
            }
        }
        throw new InterpretException("Argument node does not contain an expression", 42);
    }
}
