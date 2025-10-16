<?php

namespace IPP\Student;

use IPP\Core\Interface\InputReader;

final class GetInput
{
    private static ?InputReader $stdin = null;

    public static function setStdIn(InputReader $reader): void
    {
        self::$stdin = $reader;
    }
    public static function getStdIn(): InputReader|null
    {
        return self::$stdin;
    }
}
