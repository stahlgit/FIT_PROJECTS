<?php

namespace IPP\Student\Traverse;

use DOMElement;
use IPP\Student\Exceptions\InterpretException;
use IPP\Student\Objects\ClassObject;

class ClassTraverse
{
    /**
     * @param array<ClassObject> $existingClasses
     */
    public function __construct(private array $existingClasses)
    {
    }

    public function traverse(DOMElement $node): ClassObject
    {
        $className = $node->getAttribute('name');
        $parentName = $node->getAttribute('parent');

        if (empty($className)) {
            throw new InterpretException("No class name", 42);
        }

        $parent = $this->existingClasses[$parentName] ?? null;
        if ($parent === null) {
            throw new InterpretException("Unknown parent class: {$parentName}", 32);
        }

        $class = new ClassObject($className, $parent);
        $methodTraverser = new MethodTraverse();

        foreach ($node->childNodes as $child) {
            if ($child->nodeType !== XML_ELEMENT_NODE) {
                continue;
            }
            if ($child->nodeName === 'method') {
                /** @var DOMElement $child */
                $class->addMethod($methodTraverser->traverse($child));
            }
        }
        return $class;
    }
}
