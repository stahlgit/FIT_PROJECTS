<?php

namespace IPP\Student\Objects;

class MethodObject
{
    public string $selector; // name of the method
    public BlockObject $block; // block of the method

    public function __construct(string $selector, BlockObject $block)
    {
        $this->selector = $selector;
        $this->block = $block;
    }
}
