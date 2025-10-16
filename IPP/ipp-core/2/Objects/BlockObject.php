<?php

namespace IPP\Student\Objects;

use IPP\Student\Exceptions\SemanticException;
use IPP\Student\Objects\Expressions\ExpressionInterface;
use IPP\Student\Runtime\BuildIns\NilInstance;
use IPP\Student\Runtime\{Instance, Scope};

class BlockObject
{
    public int $arity; // number of parameters
    /** @var ParameterObject[] */
    public array $parameters = []; // list of parameters
    /** @var AssignmentObject[] */
    public array $assignments = []; // list of assignments

    public function __construct(int $arity)
    {
        $this->arity = $arity;
    }

    public function addParameter(ParameterObject $parameter): void
    {
        $this->parameters[] = $parameter;
        usort($this->parameters, function ($a, $b) {
            return $a->order <=> $b->order;
        });
    }

    public function addAssignment(AssignmentObject $assignment): void
    {
        $this->assignments[] = $assignment;
    }

    /**
     * Execute the block with receiver (self) and arguments
     * @param Instance|null $receiver
     * @param Instance[]|ExpressionInterface[] $args
     * @param Scope|null $parentScope
     * @return mixed
     */
    public function execute(?Instance $receiver, array $args, ?Scope $parentScope = null): mixed
    {
        // Validate arity
        $result = null; // define variable (if there are no assignments->error)
        if (count($args) !== $this->arity) {
            throw new SemanticException("Arity mismatch", 33);
        }
        // create a new scope with parent (lexical scoping)
        $scope = new Scope($parentScope);

        // bind parameters as read-only variables
        foreach ($this->parameters as $index => $param) {
            $scope->setVariable($param->name, $args[$index], true); // Mark as read-only
        }
        // set 'self' in the scope (immutable)
        if ($receiver !== null) {
            $scope->setVariable('self', $receiver, true);
        }

        //Execute assignment in order
        foreach ($this->assignments as $assignment) {
            $value = $assignment->expression->evaluate($scope);
            $scope->setVariable($assignment->variable->name, $value, false);
            $result = $value;
        }
        if ($result === null) {
            return NilInstance::getInstance();
        }
        return $result;
    }
}
