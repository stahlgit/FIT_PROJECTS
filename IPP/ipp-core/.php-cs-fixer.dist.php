<?php


use PhpCsFixer\Finder;
use PhpCsFixer\Config;

$finder = (new Finder())
    ->in(__DIR__) // Scans the current directory
    ->exclude('vendor'); // Excludes the vendor directory

return (new Config())
    ->setRules([
        '@PSR12' => true,
        '@PSR1' => true,
        '@PSR4' => true,
        'array_syntax' => ['syntax' => 'short'], // Example of an additional rule
        // Add or customize other rules as needed
    ])
    ->setFinder($finder)
    ->setRiskyAllowed(true); // Allow risky rules (handle with caution)