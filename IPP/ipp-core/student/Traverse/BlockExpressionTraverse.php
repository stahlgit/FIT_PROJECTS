<?php

namespace IPP\Student\Traverse;

use DOMElement;
use IPP\Student\Objects\BlockObject;
use IPP\Student\Objects\Expressions\BlockExpression;

class BlockExpressionTraverse
{
    public function traverse(DOMElement $node): BlockExpression
    {
        $block = new BlockObject((int)$node->getAttribute('arity'));

        $parameterTraverse = new ParameterTraverse();
        $assignTraverse = new AssignTraverse();

        $assignNodes = [];

        foreach ($node->childNodes as $child) {
            if ($child->nodeType !== XML_ELEMENT_NODE) {
                continue;
            }
            switch ($child->nodeName) {
                case 'parameter':
                    /** @var DOMElement $child */
                    $block->addParameter($parameterTraverse->traverse($child));
                    break;
                case 'assign':
                    /** @var DOMElement $child */
                    $order = (int)$child->getAttribute('order');
                    $assignNodes[$order] = $child;
                    break;
            }
        }
        ksort($assignNodes);
        foreach ($assignNodes as $assignNode) {
            $block->addAssignment($assignTraverse->traverse($assignNode));
        }
        return new BlockExpression($block);
    }
}
