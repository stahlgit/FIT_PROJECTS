<?php

namespace IPP\Student\Traverse;

use DOMElement;
use IPP\Student\Exceptions\InterpretException;
use IPP\Student\Objects\Expressions\SendExpression;

class SendTraverse
{
    public function traverse(DOMElement $node): SendExpression
    {
        $selectorName = $node->getAttribute('selector');
        if (empty($selectorName)) {
            throw new InterpretException("No selector name", 42);
        }

        $receiver = null;
        $argNodes = []; // Store args with their order

        // First pass: Collect all <arg> nodes and their order
        foreach ($node->childNodes as $child) {
            if ($child->nodeType !== XML_ELEMENT_NODE) {
                continue;
            }

            if ($child->nodeName === 'arg') {
                /** @var DOMElement $child */
                $order = (int)$child->getAttribute('order');
                $argNodes[$order] = $child; // Use order as key
            }
        }

        // Sort arguments by order (ascending)
        ksort($argNodes);

        // Process arguments in sorted order
        $arguments = [];
        $argTraverse = new ArgTraverse();
        foreach ($argNodes as $order => $argNode) {
            $arguments[] = $argTraverse->traverse($argNode);
        }

        // Process receiver
        foreach ($node->childNodes as $child) {
            if ($child->nodeName === 'expr') {
                /** @var DOMElement $child */
                $receiver = (new ExpressionTraverse())->traverse($child);
                // $receiver = $this->handleExpressionNode($child);
                break; // Stop after finding the first receiver
            }
        }

        if ($receiver === null) {
            throw new InterpretException("Send Expression missing receiver", 42);
        }

        return new SendExpression($selectorName, $receiver, $arguments);
    }
}
