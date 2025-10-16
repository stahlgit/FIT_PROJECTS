<?php

namespace IPP\Student\Traverse;

use DOMElement;
use IPP\Student\Exceptions\InterpretException;
use IPP\Student\Objects\Expressions\ExpressionInterface;

class ExpressionTraverse
{
    public function traverse(DOMElement $node): ExpressionInterface
    {
        foreach ($node->childNodes as $child) {
            if ($child->nodeType !== XML_ELEMENT_NODE) {
                continue;
            }
            switch ($child->nodeName) {
                case 'send':
                    /** @var DOMElement $child */
                    return (new SendTraverse())->traverse($child);
                case 'var':
                    /** @var DOMElement $child */
                    return (new VariableTraverse())->traverse($child);
                case 'literal':
                    /** @var DOMElement $child */
                    return (new LiteralTraverse())->traverse($child);
                case 'block':
                    /** @var DOMElement $child */
                    return (new BlockExpressionTraverse())->traverse($child);
                default:
                    throw new InterpretException("Unknown child node in <expr>: {$child->nodeName}", 42);
            }
        }
        throw new InterpretException("Expression node does not contain a valid child expression!", 42);
    }
}
