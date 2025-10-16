<?php

namespace IPP\Student\Objects\Expressions;

use IPP\Student\Objects\{BlockObject, BuiltInFactory};
use IPP\Student\Runtime\BuildIns\BlockInstance;
use IPP\Student\Runtime\Scope;

class BlockExpression implements ExpressionInterface
{
    public BlockObject $block;

    public function __construct(BlockObject $block)
    {
        $this->block = $block;
    }

    public function evaluate(Scope $scope): BlockInstance
    {
        $blockInstance = new BlockInstance(BuiltInFactory::$blockClass);
        $blockInstance->attributes['block'] = $this->block;
        $blockInstance->attributes['scope'] = $scope;
        return $blockInstance;
    }
}
