<?php

namespace IPP\Student\Objects\Expressions;

use IPP\Student\Exceptions\SemanticException;
use IPP\Student\Objects\ClassObject;
use IPP\Student\Runtime\{Instance, Scope};

class SendExpression implements ExpressionInterface
{
    public string $selector;
    public ExpressionInterface $receiver;
    /** @var ExpressionInterface[] */
    public array $arguments = [];

    /**
     * @param string $selector
     * @param ExpressionInterface $receiver
     * @param ExpressionInterface[] $arguments
     */
    public function __construct(string $selector, ExpressionInterface $receiver, array $arguments = [])
    {
        $this->selector = $selector;
        $this->receiver = $receiver;
        $this->arguments = $arguments;
    }

    public function evaluate(Scope $scope): mixed
    {
        $receiverObject = $this->receiver->evaluate($scope);
        /** @var array<Instance> $args */
        $args = array_map(fn($arg) => $arg->evaluate($scope), $this->arguments);

        if ($receiverObject instanceof ClassObject) {
            return $receiverObject->send($this->selector, $args);
        }
        if (!($receiverObject instanceof Instance)) {
            throw new SemanticException("Receiver must be instance of " . Instance::class, 52);
        }
        return $receiverObject->send($this->selector, $args);
    }
}
