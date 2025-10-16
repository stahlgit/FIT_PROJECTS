<?php

namespace IPP\Student\Runtime;

use IPP\Student\Objects\ClassObject;

class RuntimeRegistry
{
    /** @var array<ClassObject> */
    private static array $classes = [];

    /**
     * @param array<ClassObject> $classes
     * @return void
     */
    public static function setClasses(array $classes): void
    {
        self::$classes = $classes;
    }

    public static function findClass(string $name): ?ClassObject
    {
        return self::$classes[$name] ?? null;
    }
}
