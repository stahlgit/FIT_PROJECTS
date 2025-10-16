<?php

namespace IPP\Student;

use DOMDocument;
use DOMElement;
use IPP\Core\AbstractInterpreter;
use IPP\Student\Exceptions\{InterpretException, SemanticException};
use IPP\Student\Objects\{BuiltInInitializer, ClassObject};
use IPP\Student\Runtime\{Instance, RuntimeRegistry};
use IPP\Student\Traverse\ClassTraverse;

class Interpreter extends AbstractInterpreter
{
    /** @var array<string, ClassObject> */
    private array $classes = [];

    public function execute(): int
    {
        GetInput::setStdIn($this->input);
        GetOutput::setStdout($this->stdout);

        // Initialize BuiltIn Classes
        $this->classes = BuiltInInitializer::initialize();
        RuntimeRegistry::setClasses($this->classes);

        // Parse the source XML into runtime model
        $dom = $this->source->getDOMDocument();
        $this->traverseProgram($dom);

        // check for Main Class
        $mainClass = $this->classes['Main'] ?? null;
        if ($mainClass === null) {
            throw new SemanticException("Class 'Main' not found.", 31); // correct condition
        }

        // Check for run method
        $runMethod = $mainClass->findMethod('run');
        if ($runMethod === null) {
            throw new SemanticException("Method 'run' not found in class 'Main'", 31);
        }

        $mainInstance = new Instance($mainClass);
        // start execution
        $runMethod->block->execute($mainInstance, []);

        return 0;
    }

    private function traverseProgram(DOMDocument $dom): void
    {
        $root = $dom->documentElement;
        if ($root === null) {
            throw new InterpretException("No root element", 41);
        }
        foreach ($root->childNodes as $node) {
            if ($node->nodeType !== XML_ELEMENT_NODE) {
                continue;
            }
            if ($node->nodeName === 'class') {
                $parser = new ClassTraverse($this->classes);
                /** @var DOMElement $node */
                $class = $parser->traverse($node);
                $this->classes[$class->name] = $class;
                RuntimeRegistry::setClasses($this->classes);
            }
        }
    }
}
