<?php

namespace IPP\Student\Traverse;

use DOMElement;
use IPP\Student\Exceptions\InterpretException;
use IPP\Student\Objects\AssignmentObject;

class AssignTraverse
{
    public function traverse(DOMElement $node): AssignmentObject
    {
        $varTraverse = new VariableTraverse();
        $exprTraverse = new ExpressionTraverse();
        foreach ($node->childNodes as $child) {
            if ($child->nodeType !== XML_ELEMENT_NODE) {
                continue;
            }
            switch ($child->nodeName) {
                case 'var':
                    /** @var DOMElement $child */
                    $varNode = $varTraverse->traverse($child);
                    break;
                case 'expr':
                    /** @var DOMElement $child */
                    $exprNode = $exprTraverse->traverse($child);
                    break;
            }
        }

        if (!isset($varNode) || !isset($exprNode)) {
            throw new InterpretException('Assign node is missing variable or expression!', 42);
        }
        return new AssignmentObject($varNode, $exprNode);
    }
}
