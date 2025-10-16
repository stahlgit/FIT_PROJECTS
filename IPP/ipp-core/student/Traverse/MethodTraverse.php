<?php

namespace IPP\Student\Traverse;

use DOMElement;
use IPP\Student\Exceptions\InterpretException;
use IPP\Student\Objects\MethodObject;

class MethodTraverse
{
    public function traverse(DOMElement $node): MethodObject
    {
        $methodName = $node->getAttribute('selector');
        if (empty($methodName)) {
            throw new InterpretException("Method without selector", 42);
        }
        foreach ($node->childNodes as $child) {
            if ($child->nodeType !== XML_ELEMENT_NODE) {
                continue;
            }
            if ($child->nodeName === 'block') {
                /** @var DOMElement $child */
                $block = (new BlockTraverse())->traverse($child);
                return new MethodObject($methodName, $block);
            }
        }
        throw new InterpretException("Method without body", 42);
    }
}
