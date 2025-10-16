<?php

namespace IPP\Student\Traverse;

use DOMElement;
use IPP\Student\Objects\ParameterObject;

class ParameterTraverse
{
    public function traverse(DOMElement $node): ParameterObject
    {
        $parameterName = $node->getAttribute('name');
        $parameterOrder = $node->getAttribute('order');

        return new ParameterObject((int)$parameterOrder, $parameterName);
    }
}
