<?php

namespace IPP\Student\Runtime;

use IPP\Student\Exceptions\SemanticException;

class Scope
{
    /** @var array<string, mixed> */
    private array $variables = [];
    /** @var array<string, bool> */
    private array $readOnly = []; // Tracks read-only variables (parameters/self)
    private ?Scope $parent;

    public function __construct(?Scope $parent = null)
    {
        $this->parent = $parent;
    }

    public function getVariable(string $name): mixed
    {
        return $this->variables[$name] ?? $this->parent?->getVariable($name);
    }

    public function setVariable(string $name, mixed $value, bool $readOnly = false): void
    {
        if (isset($this->readOnly[$name])) {
            throw new SemanticException("Cannot set read only", 53);
        }
        $this->variables[$name] = $value;
        if ($readOnly) {
            $this->readOnly[$name] = true;
        }
    }
}
