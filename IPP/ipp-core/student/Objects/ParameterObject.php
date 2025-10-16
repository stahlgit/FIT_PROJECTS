<?php

namespace IPP\Student\Objects;

class ParameterObject
{
    public int $order;
    public string $name;

    public function __construct(int $order, string $name)
    {
        $this->order = $order;
        $this->name = $name;
    }
}
